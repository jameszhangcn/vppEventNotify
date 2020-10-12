/*
 * testgovpp.c - skeleton vpp engine plug-in
 *
 * Copyright (c) <current-year> <your-organization>
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <vnet/vnet.h>
#include <vnet/plugin/plugin.h>
#include <testgovpp/testgovpp.h>

#include <vlibapi/api.h>
#include <vlibmemory/api.h>
#include <vpp/app/version.h>
#include <stdbool.h>

#include <testgovpp/testgovpp.api_enum.h>
#include <testgovpp/testgovpp.api_types.h>

#define REPLY_MSG_ID_BASE testgovpp_main.msg_id_base
#include <vlibapi/api_helper_macros.h>

testgovpp_main_t testgovpp_main;

/* Action function shared between message handler and debug CLI */

int testgovpp_enable_disable (testgovpp_main_t * tmp, u32 sw_if_index,
                                   int enable_disable)
{
  vnet_sw_interface_t * sw;
  int rv = 0;

  /* Utterly wrong? */
  if (pool_is_free_index (tmp->vnet_main->interface_main.sw_interfaces,
                          sw_if_index))
    return VNET_API_ERROR_INVALID_SW_IF_INDEX;

  /* Not a physical port? */
  sw = vnet_get_sw_interface (tmp->vnet_main, sw_if_index);
  if (sw->type != VNET_SW_INTERFACE_TYPE_HARDWARE)
    return VNET_API_ERROR_INVALID_SW_IF_INDEX;

  testgovpp_create_periodic_process (tmp);

  vnet_feature_enable_disable ("device-input", "testgovpp",
                               sw_if_index, enable_disable, 0, 0);

  /* Send an event to enable/disable the periodic scanner process */
  vlib_process_signal_event (tmp->vlib_main,
                             tmp->periodic_node_index,
                             TESTGOVPP_EVENT_PERIODIC_ENABLE_DISABLE,
                            (uword)enable_disable);
  return rv;
}

int gtp_error_ind_enable_disable (testgovpp_main_t * tmp, u32 sw_if_index,
                                   int enable_disable)
{

  int rv = 0;

  testgovpp_create_cu_up_event_process(tmp);

  vnet_feature_enable_disable ("device-input", "testgovpp",
                               sw_if_index, enable_disable, 0, 0);

  /* Send an event to enable/disable the periodic scanner process */
  vlib_process_signal_event (tmp->vlib_main,
                             tmp->cu_up_event_node_index,
                             TESTGOVPP_CU_UP_EVENT_ENABLE_DISABLE,
                            (uword)enable_disable);
  return rv;
}

static clib_error_t *
testgovpp_enable_disable_command_fn (vlib_main_t * vm,
                                   unformat_input_t * input,
                                   vlib_cli_command_t * cmd)
{
  testgovpp_main_t * tmp = &testgovpp_main;
  u32 sw_if_index = ~0;
  int enable_disable = 1;

  int rv;

  while (unformat_check_input (input) != UNFORMAT_END_OF_INPUT)
    {
      if (unformat (input, "disable"))
        enable_disable = 0;
      else if (unformat (input, "%U", unformat_vnet_sw_interface,
                         tmp->vnet_main, &sw_if_index))
        ;
      else
        break;
  }

  if (sw_if_index == ~0)
    return clib_error_return (0, "Please specify an interface...");

  rv = testgovpp_enable_disable (tmp, sw_if_index, enable_disable);

  switch(rv)
    {
  case 0:
    break;

  case VNET_API_ERROR_INVALID_SW_IF_INDEX:
    return clib_error_return
      (0, "Invalid interface, only works on physical ports");
    break;

  case VNET_API_ERROR_UNIMPLEMENTED:
    return clib_error_return (0, "Device driver doesn't support redirection");
    break;

  default:
    return clib_error_return (0, "testgovpp_enable_disable returned %d",
                              rv);
    }
  return 0;
}

/* *INDENT-OFF* */
VLIB_CLI_COMMAND (testgovpp_enable_disable_command, static) =
{
  .path = "testgovpp enable-disable",
  .short_help =
  "testgovpp enable-disable <interface-name> [disable]",
  .function = testgovpp_enable_disable_command_fn,
};

VLIB_CLI_COMMAND (want_cu_up_gtp_error_ind_command, static) =
{
  .path = "testgovpp gtp err enable-disable",
  .short_help =
  "testgovpp gtp err enable-disable [disable]",
  .function = testgovpp_enable_disable_command_fn,
};

/* *INDENT-ON* */

/* API message handler */
static void vl_api_testgovpp_enable_disable_t_handler
(vl_api_testgovpp_enable_disable_t * mp)
{
  vl_api_testgovpp_enable_disable_reply_t * rmp;
  testgovpp_main_t * tmp = &testgovpp_main;
  int rv;

  rv = testgovpp_enable_disable (tmp, ntohl(mp->sw_if_index),
                                      (int) (mp->enable_disable));

  REPLY_MACRO(VL_API_TESTGOVPP_ENABLE_DISABLE_REPLY);
}

//pub_sub_handler(cu_up_gtp_error_ind_events,CU_UP_GTP_ERROR_IND_EVENTS)
static void vl_api_want_cu_up_gtp_error_ind_events_t_handler (                             
    vl_api_want_cu_up_gtp_error_ind_events_t *mp)                                          
{                                                                       
    vpe_api_main_t *vam = &vpe_api_main;                                
    vpe_client_registration_t *rp;                                      
    vl_api_want_cu_up_gtp_error_ind_events_reply_t *rmp;
    testgovpp_main_t * tmp = &testgovpp_main;                               
    uword *p;                                                           
    i32 rv = 0;                                                         
                                                                        
    p = hash_get (vam->cu_up_gtp_error_ind_events_registration_hash, mp->client_index);      
    if (p) {                                                            
        if (mp->enable_disable) {                                       
	    clib_warning ("pid %d: already enabled...", ntohl(mp->pid)); 
            rv = VNET_API_ERROR_INVALID_REGISTRATION;                   
            goto reply;                                                 
        } else {                                                        
            rp = pool_elt_at_index (vam->cu_up_gtp_error_ind_events_registrations, p[0]);    
            pool_put (vam->cu_up_gtp_error_ind_events_registrations, rp);                    
            hash_unset (vam->cu_up_gtp_error_ind_events_registration_hash,                  
                mp->client_index);                                      
            goto reply;                                                 
        }                                                               
    }                                                                   
    if (mp->enable_disable == 0) {                                      
        clib_warning ("pid %d: already disabled...", mp->pid);          
        rv = VNET_API_ERROR_INVALID_REGISTRATION;                       
        goto reply;                                                     
    }                                                                   
    pool_get (vam->cu_up_gtp_error_ind_events_registrations, rp);                            
    rp->client_index = mp->client_index;                                
    rp->client_pid = mp->pid;                                           
    hash_set (vam->cu_up_gtp_error_ind_events_registration_hash, rp->client_index,
              rp - vam->cu_up_gtp_error_ind_events_registrations);       
              
    rv = gtp_error_ind_enable_disable (tmp, 0, (int) (mp->enable_disable));
reply:                                                                 
    REPLY_MACRO (VL_API_WANT_CU_UP_GTP_ERROR_IND_EVENTS_REPLY);           
} 

/* API definitions */
#include <testgovpp/testgovpp.api.c>

static clib_error_t * testgovpp_init (vlib_main_t * vm)
{
  testgovpp_main_t * tmp = &testgovpp_main;
  clib_error_t * error = 0;

  tmp->vlib_main = vm;
  tmp->vnet_main = vnet_get_main();

  /* Add our API messages to the global name_crc hash table */
  tmp->msg_id_base = setup_message_id_table ();

  return error;
}

VLIB_INIT_FUNCTION (testgovpp_init);

/* *INDENT-OFF* */
VNET_FEATURE_INIT (testgovpp, static) =
{
  .arc_name = "device-input",
  .node_name = "testgovpp",
  .runs_before = VNET_FEATURES ("ethernet-input"),
};
/* *INDENT-ON */

/* *INDENT-OFF* */
VLIB_PLUGIN_REGISTER () =
{
  .version = VPP_BUILD_VER,
  .description = "testgovpp plugin description goes here",
};
/* *INDENT-ON* */

/*
 * fd.io coding-style-patch-verification: ON
 *
 * Local Variables:
 * eval: (c-set-style "gnu")
 * End:
 */
