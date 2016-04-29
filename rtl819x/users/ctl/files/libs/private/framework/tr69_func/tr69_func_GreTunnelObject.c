/*./auto/tr69_func_GreTunnelObject.c*/

#include "tsl_common.h"
#include "tr69_func.h"
#include "tr69_func_common.h"
#include "ctl.h"
#include "tsl_strconv.h"

/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_get_GreTunnelObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.GRETunnel
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_GreTunnelObject_t *p_cur_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_get_GreTunnelObject_value(tsl_char_t *p_oid_name, _GreTunnelObject *p_cur_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_UNCHANGED;

	ctllog_debug("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	return rv;
}


/**************************************************************************
 *	[FUNCTION NAME]:
 *	        tf69_func_set_GreTunnelObject_value
 *
 *	[DESCRIPTION]:
 *	        Device.GRETunnel
 *
 *	[PARAMETER]:
 *	        tsl_char_t *p_oid_name
 *	        st_GreTunnelObject_t *p_cur_data
 *	        st_GreTunnelObject_t *p_new_data
 *
 *	[RETURN]
 *              >= 0          SUCCESS
 *              <  0          ERROR
 **************************************************************************/

tsl_int_t tr69_func_set_GreTunnelObject_value(tsl_char_t *p_oid_name, _GreTunnelObject *p_cur_data, _GreTunnelObject *p_new_data)
{
 	tsl_int_t rv = TR69_RT_SUCCESS_VALUE_CHANGED;

	ctllog_error("FUNCTION LINE %s %s %d \n", p_oid_name, __FUNCTION__, __LINE__);

	if (p_new_data != NULL && p_cur_data != NULL){
		if(p_new_data->enable && !p_cur_data->enable){
			//ip link add gre_tun type gretap local XXX.XXX.XXX.XXX remote XXX.XXX.XXX.XXX
			//ip link set dev gre_tun up
			if( DO_SYSTEM("ip link add gre_tun type gretap local %s remote %s" , p_new_data->X_ACTIONTEC_COM_GRETunnelLocalIPv4Address,
				 p_new_data->X_ACTIONTEC_COM_GRETunnelRemoteIPv4Address ) < 0){
				ctllog_debug("[GRE_TUNNEL] ip link add gre tunnel failed.\n");
			}

			if( DO_SYSTEM("ip link set dev gre_tun up") < 0){
				ctllog_debug("[GRE_TUNNEL] ip link set gre tunnel up failed.\n");
			}  

		}

        	if(!p_new_data->enable && p_cur_data->enable){
            		//ip link set gre_tun down
         		//ip link del gre_tun
            		if( DO_SYSTEM( "ip link set gre_tun down" ) < 0){
                		ctllog_debug("[GRE_TUNNEL] ip link set gre tunnel down failed.\n");
            		}

            		if( DO_SYSTEM("ip link del gre_tun") < 0){
                		ctllog_debug("[GRE_TUNNEL] ip link del gre tunnel failed.\n");
            		}
        	}

		if (p_cur_data->enable && p_new_data->enable && 
			(tsl_strcmp(p_new_data->X_ACTIONTEC_COM_GRETunnelLocalIPv4Address, p_cur_data->X_ACTIONTEC_COM_GRETunnelLocalIPv4Address) ||
			tsl_strcmp(p_new_data->X_ACTIONTEC_COM_GRETunnelRemoteIPv4Address, p_cur_data->X_ACTIONTEC_COM_GRETunnelRemoteIPv4Address)) 
		)
		{
			//re-cofigure gre tunnel
			if( DO_SYSTEM("ip link set gre_tun down") < 0){
				ctllog_debug("[GRE_TUNNEL] ip link set gre tunnel down failed.\n");
			}

			if( DO_SYSTEM("ip link del gre_tun") < 0){
				ctllog_debug("[GRE_TUNNEL] ip link del gre tunnel failed.\n");
			} 

			if( DO_SYSTEM("ip link add gre_tun type gretap local %s remote %s", p_new_data->X_ACTIONTEC_COM_GRETunnelLocalIPv4Address,
                                 p_new_data->X_ACTIONTEC_COM_GRETunnelRemoteIPv4Address) < 0){
				ctllog_debug("[GRE_TUNNEL] ip link add gre tunnel failed.\n");
			}

			if( DO_SYSTEM("ip link set dev gre_tun up") < 0){
				ctllog_debug("[GRE_TUNNEL] ip link set gre tunnel up failed.\n");
			}            
		}


	}

	return rv;
}

