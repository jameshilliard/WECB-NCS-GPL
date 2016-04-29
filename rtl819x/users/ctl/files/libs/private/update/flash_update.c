#include "flash_update.h"
#include "flash_define.h"

#define ACTIONTEC

#ifdef ACTIONTEC

#define DEV 			"ubi0:img"
#define MOUNT_POINT 	"/mnt/ubifs/"
#define CMDLINE_FILE	"/proc/cmdline"
#define FILE_SYS_TYPE 	"ubifs"
#define CMDLINE_MAXLEN	4096
#define FLASH_READ 0
#define FLASH_WRITE 1

typedef long addr_t;

/*
 * magic - signature that means that data/header is present in the section.
 * head_size - size of header
 * counter - the section with the largest counter is the active (current) one.
 * kernel_size - size of the kernel+ramfs in this section
 * kernel_chksum - check sum of the data in the section(kernel+ramfs)  include this struct
 *          (not including this crc field)
 * rootfs_size - size of the cramfs.img in this section
 * rootfs_chksum - check sum of the data in the section(cramfs.img)  not include this struct
 * tag - userspace's header
 */
struct section_header{
	u32 magic;
	u32 head_size; // == sizeof(section_header) + sizeof(tag)
	u32 counter;
	u32 kernel_size;
	u32 kernel_chksum;
	u32 rootfs_size;
	u32 rootfs_chksum;
	u8 tag[TAG_LEN];
};

static inline unsigned int swap32(unsigned int x)
{
    return (((x & 0xff000000) >> 24) | ((x & 0x00ff0000) >>  8) |
	((x & 0x0000ff00) <<  8) | ((x & 0x000000ff) << 24));
}

#define flash_swap_32(x)	swap32(x)

static int flash_update(const char *src, const char *kernel, const char *rootfs, AEI_IMG_HEADER *tag);
static void header_init(struct section_header *header,int active, AEI_IMG_HEADER *tag);
static void header_finish_chksum(struct section_header *header, u32 kernel_chksum, u32 rootfs_chksum);
static int header_read_write(int fd, struct section_header *header, int write_flag);
static void flash_swap_header(struct section_header *header);
static int flash_write_file(int fd_dest, int fd_src, size_t size, u32 *chksum);
static int read_check(const char *kernel, const char *rootfs);
static int get_filename(char *kernel_name, char * rootfs_name, int flag);
static int in_active(const char *filename);
static int get_active_file(void);

static char *kernelfile[] = {
	[0] = "rt_sys1.img",
	[1] = "rt_sys2.img",
	};
static char *rootfsfile[] = {
	[0] = "rt_sys1.img.cramfs",
	[1] = "rt_sys2.img.cramfs",
	};


static u32 big_counter = 0;
static int update_state;

enum{
	UPDATE_START = 0,
	MOUNT_FINISH = 20,
	GET_FILENAME_FINISH = 20,
	WRITE_START = 40,
	WRITE_FINISH = 50,
	CHECK_FINISH = 10,
};

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        flash_read_header
 *
 *	[DESCRIPTION]:
 *		read ACTIVE/INACTIVE system's header
 *
 *	[PARAMETER]:
 *	        tag[in] 	: buf for tag
 *	        flag[in] 	: INACTIVE/ACTIVE
 *
 *	[RETURN]
 *              >= 0	 	:         SUCCESS
 *              other		:         FAIL
 *
 *	[History]
 *			@2011.9.15  jerome.zhao: create this function
 **************************************************************************/
int flash_read_header(void *tag, int flag)
{
	int ret;
	char kernel_name[MAX_FILENAME];
	char rootfs_name[MAX_FILENAME];

	struct section_header buf;
	int fd;

	mkdir(MOUNT_POINT, 0);

	ret = mount(DEV, MOUNT_POINT, FILE_SYS_TYPE, MS_SYNCHRONOUS, 0);
	if (ret < 0) {
		perror("Mount error");
		return ret;
	}

	ret = get_filename(kernel_name, rootfs_name, flag);
	if (ret < 0)
	{
		printf("Get_filename error\n");
		goto L1;
	}

	fd = open(kernel_name, O_RDONLY);
	if (fd < 0)
	{
		perror("Flash read open error");
		ret = fd;
		goto L1;
	}

	ret = header_read_write(fd, &buf, FLASH_READ);
	if (ret < 0)
	{
		printf("Header_read_write error\n");
		goto L2;
	}

	memcpy(tag, &buf, sizeof(buf.tag));

L2:
	close(fd);
L1:
	umount(MOUNT_POINT);

	return ret;
}

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        flash_update_from_file
 *
 *	[DESCRIPTION]:
 *		update from RAM file, and active it
 *
 *	[PARAMETER]:
 *	        filename[in] 	: RAM filename
 *	        tag[in]		: tag write to flash
 *
 *	[RETURN]
 *              >= 0	 	:         SUCCESS
 *              other		:         FAIL
 *
 *	[History]
 *			@2011.9.15  jerome.zhao: create this function
 **************************************************************************/
int flash_update_from_file(const char *filename, AEI_IMG_HEADER *tag)
{
	int ret;
	char kernel_name[MAX_FILENAME], rootfs_name[MAX_FILENAME];

	update_state = UPDATE_START;

	mkdir(MOUNT_POINT, 0);

	// ret = mount(argv[1], argv[2], argv[3], 0, 0);
	ret = mount(DEV, MOUNT_POINT, FILE_SYS_TYPE, MS_SYNCHRONOUS | MS_REMOUNT, 0);
	if (ret < 0) {
		perror("Mount error");
		return ret;
	}

	update_state += MOUNT_FINISH;

	ret = get_filename(kernel_name, rootfs_name, INACTIVE);
	if (ret < 0)
	{
		printf("Get_filename error\n");
		goto L1;
	}

	update_state += GET_FILENAME_FINISH;

	ret = flash_update(filename, kernel_name, rootfs_name, tag);
	if (ret < 0) {
		printf("Update fault\n");
		in_active(kernel_name);
	} else {
		ret = 0;
	}

L1:
	mount(DEV, MOUNT_POINT, FILE_SYS_TYPE, MS_SYNCHRONOUS | MS_RDONLY | MS_REMOUNT, 0);

	return ret;
}

int flash_update_active_img(int img_num)
{
	int ret = 0;
	int size;
	struct section_header header;
	int fd;
	char kernel_name[MAX_FILENAME];
	u8 buf[BUF_SIZE];
	u32 kernel_chksum = 0;
	u32 rootfs_chksum = 0;

	strncpy(kernel_name, MOUNT_POINT, MAX_FILENAME);
	strncat(kernel_name, kernelfile[img_num], MAX_FILENAME);

	mkdir(MOUNT_POINT, 0);

	// ret = mount(argv[1], argv[2], argv[3], 0, 0);
	ret = mount(DEV, MOUNT_POINT, FILE_SYS_TYPE, MS_SYNCHRONOUS | MS_REMOUNT, 0);
	if (ret < 0) {
		perror("Mount error");
		return ret;
	}

	fd = open(kernel_name, O_RDWR);
	if (fd < 0)
	{
		perror("In_active open error");
		ret = fd;
		goto L0;
	}

	header_read_write(fd, &header, FLASH_READ);
	flash_swap_header(&header);
	rootfs_chksum = header.rootfs_chksum;
	header.kernel_chksum = 0;
	header.rootfs_chksum = 0;

	lseek(fd, header.head_size, SEEK_SET);

	while (1) {
		size = read(fd, buf, BUF_SIZE);
		if (size <= 0)
		{
			if (size == 0)
			{
				break;
			}

			perror("Read_check:read error");
			ret = size;

			break;
		}

		chksum_update(&kernel_chksum, buf, size);
	}

	header.counter += 2;

	header_finish_chksum(&header, kernel_chksum, rootfs_chksum);
	flash_swap_header(&header);
	header_read_write(fd, &header, FLASH_WRITE);

L1:
	fsync(fd);
	close(fd);
L0:
	mount(DEV, MOUNT_POINT, FILE_SYS_TYPE, MS_SYNCHRONOUS | MS_RDONLY | MS_REMOUNT, 0);

	return ret;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        flash_update_state
 *
 *	[DESCRIPTION]:
 *		return update state
 *
 *	[RETURN]
 *              >= 0	 	:         update state
 *              other		:         FAIL
 *
 *	[History]
 *			@2011.9.21  jerome.zhao: create this function
 **************************************************************************/
int flash_update_state(void)
{
	return update_state;
}


static u32 get_biggest_counter(void)
{
	get_active_file();

	return big_counter;
}

static int get_active_file(void)
{
	int fd;
	int size;
	char buf[CMDLINE_MAXLEN];
	char *p = NULL;
	int i;
	int ret;
	struct section_header header;
	char dest_name[MAX_FILENAME];
	static int active_file = -1;

	if (active_file != -1)
	{
		return active_file;
	}
	active_file = 0;

	fd = open(CMDLINE_FILE, O_RDONLY);
	if (fd < 0)
	{
		perror("Open cmdline error");
		return fd;
	}

	size = read(fd, buf, CMDLINE_MAXLEN);
	if (size < 0)
	{
		perror("Read cmdline error");
		close(fd);
		return size;
	}

	close(fd);

	p = strstr(buf, "bootimg=");
	if (p == NULL)	// find the max header.counter
	{
		fprintf(stderr, "No bootimg cmd args\n");
		active_file = 0;
		
		for (i = 0; i < sizeof(kernelfile) / sizeof(kernelfile[0]); i++)
		{
			strncpy(dest_name, MOUNT_POINT, MAX_FILENAME);
			strncat(dest_name, kernelfile[i], MAX_FILENAME);

			fd = open(dest_name, O_RDONLY);
			if (fd < 0)
			{
				continue;
			}

			ret = header_read_write(fd, &header, FLASH_READ);
			close(fd);
			if (ret < 0)
			{
				return -1;
			}

			flash_swap_header(&header);
			if (header.magic != FLASH_SECTION_MAGIC)
			{
				continue;
			}
			if (big_counter < header.counter)
			{
				big_counter = header.counter;	
			}
		}		
		return active_file;
	}
	else	// find the current header.counter
	{
		p += strlen("bootimg=");

		for (i = 0; i < sizeof(kernelfile) / sizeof(kernelfile[0]); i++)
		{
			if (strncmp(p, kernelfile[i], strlen(kernelfile[i])) == 0)
			{
				active_file = i;
				
				strncpy(dest_name, MOUNT_POINT, MAX_FILENAME);
				strncat(dest_name, kernelfile[active_file], MAX_FILENAME);

				fd = open(dest_name, O_RDONLY);
				if (fd < 0)
				{
					perror("Get_active_file error");
					return -1;
				}

				ret = header_read_write(fd, &header, FLASH_READ);
				close(fd);
				if (ret < 0)
				{
					return -1;
				}

				flash_swap_header(&header);

				big_counter = header.counter;

				return active_file;
			}
		}
	}

	return active_file;
}

static int get_filename(char *kernel_name, char * rootfs_name, int flag)
{
	int select = 0;
	int active_file;
	

	active_file = get_active_file();
	if (active_file < 0)
	{
		return active_file;
	}

	if (flag == INACTIVE)
	{
		select = sizeof(kernelfile)/sizeof(kernelfile[0]) - active_file - 1;
	}
	else
	{
		select = active_file;
	}

	strncpy(kernel_name, MOUNT_POINT, MAX_FILENAME);
	strncat(kernel_name, kernelfile[select], MAX_FILENAME);
	strncpy(rootfs_name, MOUNT_POINT, MAX_FILENAME);
	strncat(rootfs_name, rootfsfile[select], MAX_FILENAME);

	return 0;
}


// write flash
static int flash_write_file(int fd_dest, int fd_src, size_t size, u32 *chksum)
{
	u8 buf[BUF_SIZE];
	size_t count = 0;
	size_t size_buf;
	
	while (size)
	{
		size_buf = sizeof(buf);

		if (size - count < size_buf)
		{
			size_buf = size - count;
		}
		
		size_buf = read(fd_src, buf, size_buf);
		if (size_buf <= 0)
		{
			if (size_buf < 0)
			{
				perror("Read error");
				return size_buf;
			}

			break;
		}

		if (write(fd_dest, buf, size_buf) != size_buf)
		{
			perror("Write error");
			return -1;
		}

		chksum_update(chksum, buf, size_buf);

		count += size_buf;
	}

	fsync(fd_dest);

	return -(size - count);
}


static int flash_update(const char *src, const char *kernel, const char *rootfs, AEI_IMG_HEADER *tag)
{
	int fd_src, fd_kernel, fd_rootfs;
	int ret;
	u32 kernel_chksum = 0;
	u32 rootfs_chksum = 0;
	struct section_header header;

	printf("Upgrade kernel:%s rootfs:%s\n", kernel, rootfs);

	// init header
	header_init(&header, 1, tag);

	// open src file
	fd_src = open(src, O_RDONLY);
	if (fd_src < 0)
	{
		perror("Open src file error");
		ret = fd_src;
		goto L1;
	}

	// read tag
	lseek(fd_src, tag->tag_offset, SEEK_SET);
	ret = read(fd_src, header.tag, tag->tag_size);
	if (ret != tag->tag_size)
	{
		fprintf(stderr, "Read tag error\n");
		goto L1;
	}

	// write kernel img
	unlink(kernel);

	fd_kernel = open(kernel, O_WRONLY | O_TRUNC | O_CREAT |O_SYNC, 0666);
	if (fd_kernel < 0)
	{
		perror("Open kernel img error");
		ret = fd_kernel;
		goto L1;
	}

	lseek(fd_src, tag->kernel_offset, SEEK_SET);
	lseek(fd_kernel, sizeof(header), SEEK_SET);

	ret = flash_write_file(fd_kernel, fd_src, header.kernel_size, &kernel_chksum);
	if (ret < 0)
	{
		goto L2;
	}
	else
	{
		printf("Upgrade %s: count 0x%x bytes\n", kernel, header.kernel_size);
	}

	// write rootfs img
	unlink(rootfs);

	fd_rootfs = open(rootfs, O_WRONLY | O_TRUNC | O_CREAT |O_SYNC, 0666);
	if (fd_rootfs < 0)
	{
		perror("Open rootfs img error");
		ret = fd_rootfs;
		goto L2;
	}

	lseek(fd_src, tag->rootfs_offset, SEEK_SET);

	ret = flash_write_file(fd_rootfs, fd_src, header.rootfs_size, &rootfs_chksum);
	if (ret < 0)
	{
		goto L3;
	}
	else
	{
		printf("Upgrade %s: count 0x%x bytes\n", rootfs, header.rootfs_size);
	}

	// init header and write
	header_finish_chksum(&header, kernel_chksum, rootfs_chksum);
	flash_swap_header(&header);
	ret = header_read_write(fd_kernel, &header, FLASH_WRITE);
	if (ret < 0)
	{
		fprintf(stderr, "Header_read_write error\n");
		goto L3;
	}
	
	close(fd_rootfs);
	close(fd_kernel);
	close(fd_src);

	// read check
	ret = read_check(kernel, rootfs);
	if (ret < 0)
	{
		return ret;
		in_active(kernel);
	}

	printf("Read check success\n");

	return ret;

L3:
	close(fd_rootfs);	
L2:
	close(fd_kernel);
L1:
	close(fd_src);

	return ret;
}

static int in_active(const char *kernel)
{
	struct section_header header;
	int fd;

	fd = open(kernel, O_RDWR);
	if (fd < 0)
	{
		perror("In_active open error");
		return fd;
	}

	header_read_write(fd, &header, FLASH_READ);
	header.counter = 0;
	header_read_write(fd, &header, FLASH_WRITE);

	close(fd);

	return 0;
}

static int read_check(const char *kernel, const char *rootfs)
{
	u32 read_chksum = 0;
	u32 kernel_chksum, rootfs_chksum;
	int size;
	int fd;
	int ret = 0;
	struct section_header header;
	u8 buf[BUF_SIZE];

	printf("Begin read check...\n");

	// check kernel chksum
	fd = open(kernel, O_RDONLY);
	if (fd < 0)
	{
		perror("Read_check open kernel img error");
		return fd;
	}

	header_read_write(fd, &header, FLASH_READ);
	flash_swap_header(&header);

	kernel_chksum = header.kernel_chksum;
	header.kernel_chksum = 0;
	rootfs_chksum = header.rootfs_chksum;
	header.rootfs_chksum = 0;

	lseek(fd, header.head_size, SEEK_SET);

	while (1) {
		size = read(fd, buf, BUF_SIZE);
		if (size <= 0)
		{
			if (size == 0)
			{
				break;
			}

			perror("Read_check:read error");
			ret = size;
			
			goto L1;
		}

		chksum_update(&read_chksum, buf, size);
	}

	close(fd);

	chksum_update(&read_chksum, (u8 *)&header, header.head_size);

	if (read_chksum != kernel_chksum)
	{
		fprintf(stderr, "Read_check check kernel error\n");
		ret = -1;
		goto L0;
	}

	// check rootfs chksum
	read_chksum = 0;	
	fd = open(rootfs, O_RDONLY);
	if (fd < 0)
	{
		perror("Read_check open rootfs img error");
		return fd;
	}

	while (1) {
		size = read(fd, buf, BUF_SIZE);
		if (size <= 0)
		{
			if (size == 0)
			{
				break;
			}

			perror("Read_check:read error");
			ret = size;
			
			goto L1;
		}

		chksum_update(&read_chksum, buf, size);
	}

	close(fd);

	if (read_chksum != rootfs_chksum)
	{
		printf("Read_check check rootfs error\n");
		ret = -1;
		goto L0;
	}

L1: 
	close(fd);
L0:
	return ret;
}

static int header_read_write(int fd, struct section_header *header, int write_flag)
{
	int ret;

	ret = lseek(fd, 0, SEEK_SET);
	if (ret < 0)
	{
		perror("Lseek 0 SEEK_SET error");
		goto L0;
	}

	if (write_flag == FLASH_WRITE)
	{
		ret = write(fd, header, sizeof(*header));
		if (ret < 0)
		{
			perror("Write header error");
		}
		fsync(fd);
		goto L0;
	}

	ret = read(fd, header, sizeof(*header));
	if (ret < 0)
	{
		perror("Read header error");
	}
L0:
	return ret;
}

static void header_init(struct section_header *header,int active, AEI_IMG_HEADER *tag)
{
    memset(header, 0, sizeof(*header));
	header->magic = FLASH_SECTION_MAGIC;
	header->head_size = sizeof(*header);
	header->counter = active ? get_biggest_counter() + 1 : 0;
	header->kernel_size = tag->kernel_size;
	header->rootfs_size = tag->rootfs_size;

	return;
}

static void header_finish_chksum(struct section_header *header, u32 kernel_chksum, u32 rootfs_chksum)
{
	chksum_update(&kernel_chksum, (u8 *)header, sizeof(*header));
	header->kernel_chksum = kernel_chksum;
	header->rootfs_chksum = rootfs_chksum;

	return;
}

static void flash_swap_header(struct section_header *header)
{
#if 0
	header->magic = flash_swap_32(header->magic);
	header->head_size = flash_swap_32(header->head_size);
	header->counter = flash_swap_32(header->counter);
	header->kernel_size = flash_swap_32(header->kernel_size);
	header->kernel_chksum = flash_swap_32(header->kernel_chksum);
	header->rootfs_size = flash_swap_32(header->rootfs_size);
	header->rootfs_chksum = flash_swap_32(header->rootfs_chksum);
#endif
}


#endif
