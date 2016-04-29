#ifndef AEI_RUT_PASSWORD_H
#define AEI_RUT_PASSWORD_H
extern void AEI_rut_getRootPassword(tsl_char_t *rootPassword,tsl_int_t root_len,tsl_char_t *RandomRootPassword,tsl_char_t * serialnum,tsl_int_t ser_len);
void AEI_getRootPassword(tsl_char_t *rootPassword_seed,tsl_char_t * rootPassword,tsl_int_t root_len);
extern void AEI_rut_getAdminPassword(tsl_char_t *adminPassword,tsl_char_t *serialnum);
#endif 
