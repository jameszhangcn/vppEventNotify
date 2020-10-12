/*
 * testgovpp_periodic.c - skeleton plug-in periodic function
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

#include <vlib/vlib.h>
#include <vppinfra/error.h>
#include <testgovpp/testgovpp.h>
#include <vlibapi/api.h>
#include <vlibapi/api_types.h>
#include <vlibapi/api_helper_macros.h>
#include <vlibmemory/api.h>
#include <testgovpp/testgovpp.api_enum.h>
#include <testgovpp/testgovpp.api_types.h>

static void
handle_event1 (testgovpp_main_t *pm, f64 now, uword event_data)
{
  clib_warning ("received TESTGOVPP_EVENT1");
}

static void
handle_event2 (testgovpp_main_t *pm, f64 now, uword event_data)
{
  clib_warning ("received TESTGOVPP_EVENT2");
}

static void
handle_periodic_enable_disable (testgovpp_main_t *pm, f64 now, uword event_data)
{
   clib_warning ("Periodic timeouts now %s",
     event_data ? "enabled" : "disabled");
   pm->periodic_timer_enabled = event_data;
}

static void
handle_cu_up_event_enable_disable (testgovpp_main_t *pm, f64 now, uword event_data)
{
   clib_warning ("cu up events timeouts now %s",
     event_data ? "enabled" : "disabled");
   pm->cu_up_event_timer_enabled = event_data;
}

//#if 0
static void
send_cu_up_gtp_error_ind_event(vpe_api_main_t *am,
vpe_client_registration_t * reg,
vl_api_registration_t * vl_reg,
vl_api_cu_up_event_type_t event_type,
vl_api_cu_up_gtp_error_ind_t *data,
int raw_msg_len, u8 * raw_msg_data)
{
  vl_api_cu_up_gtp_error_ind_event_t *mp;
  int msg_size;
  
  msg_size = sizeof(*mp);
  msg_size += raw_msg_len;

  mp=vl_msg_api_alloc(msg_size);
  clib_memset(mp,0,msg_size);
  mp->_vl_msg_id = ntohs(VL_API_CU_UP_GTP_ERROR_IND_EVENT + testgovpp_main.msg_id_base);

  mp->client_index = reg->client_index;
  mp->pid = reg->client_pid;
  mp->type = event_type;
  mp->data.ue_id = data->ue_id;
  mp->data.te_id = data->te_id;
  mp->data.rb_id = data->rb_id;
  mp->raw_msg_len = raw_msg_len;
  memcpy(mp->raw_msg_data,raw_msg_data, raw_msg_len);
  clib_warning ("send_cu_up_gtp_error_ind_event msg_id %d client index %d pid %d type %d raw_msg_len %d ", mp->_vl_msg_id, mp->client_index, mp->pid, mp->type, raw_msg_len);
  vl_api_send_msg(vl_reg, (u8*)mp);
}
//#endif

static void
handle_timeout (testgovpp_main_t *pm, f64 now)
{
  clib_warning ("timeout at %.2f", now);
}

static void
handle_cu_up_event_timeout (testgovpp_main_t *pm, f64 now)
{
  clib_warning ("cu up event timeout enter at %.2f", now);
  vpe_api_main_t *vam = &vpe_api_main;
  vpe_client_registration_t *reg;
  vl_api_registration_t *vl_reg;

  u8 msg_data[10] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA};

  vl_api_cu_up_gtp_error_ind_t gtp_error_ind;
      gtp_error_ind.ue_id = 0x1234;
      gtp_error_ind.te_id = 0x3456;
      gtp_error_ind.rb_id = 0xAA;
      clib_warning ("handle_cu_up_event_timeout ue-id %x ",gtp_error_ind.ue_id);

      pool_foreach(reg, vam->cu_up_gtp_error_ind_events_registrations,
      ({
        clib_warning ("handle_cu_up_event_timeout client_index %x ",reg->client_index);
        vl_reg = vl_api_client_index_to_registration(reg->client_index);
        if (vl_reg){
            clib_warning ("handle_cu_up_event_timeout send gtp error ind ");
            send_cu_up_gtp_error_ind_event(vam, reg, vl_reg, CU_UP_EVENT_GTP_ERROR_IND, &gtp_error_ind, 10, msg_data);
        }
      }));

  clib_warning ("cu up event timeout at %.2f", now);
}

static uword
testgovpp_periodic_process (vlib_main_t * vm,
	                  vlib_node_runtime_t * rt, vlib_frame_t * f)
{
  testgovpp_main_t *pm = &testgovpp_main;
  f64 now;
  f64 timeout = 10.0;
  uword *event_data = 0;
  uword event_type;
  int i;
  //vpe_api_main_t *vam = &vpe_api_main;
  //vpe_client_registration_t *reg;
  //vl_api_registration_t *vl_reg;

  clib_warning ("testgovpp_periodic_process enter... ");

  while (1)
    {
      if (pm->periodic_timer_enabled)
        vlib_process_wait_for_event_or_clock (vm, timeout);
      else
        vlib_process_wait_for_event (vm);

      now = vlib_time_now (vm);

      event_type = vlib_process_get_events (vm, (uword **) & event_data);
      clib_warning ("testgovpp_periodic_process event_type %d ",event_type);

      switch (event_type)
	{
	  /* Handle TESTGOVPP_EVENT1 */
	case TESTGOVPP_EVENT1:
	  for (i = 0; i < vec_len (event_data); i++)
	    handle_event1 (pm, now, event_data[i]);
	  break;

	  /* Handle TESTGOVPP_EVENT2 */
	case TESTGOVPP_EVENT2:
	  for (i = 0; i < vec_len (event_data); i++)
	    handle_event2 (pm, now, event_data[i]);
	  break;
          /* Handle the periodic timer on/off event */
	case TESTGOVPP_EVENT_PERIODIC_ENABLE_DISABLE:
	  for (i = 0; i < vec_len (event_data); i++)
	    handle_periodic_enable_disable (pm, now, event_data[i]);
	  break;

          /* Handle periodic timeouts */
	case ~0:
	  
    handle_timeout (pm, now);
	  break;
	}
      vec_reset_length (event_data);
    }
  return 0;			/* or not */
}



static uword
testgovpp_cu_up_event_process (vlib_main_t * vm,
	                  vlib_node_runtime_t * rt, vlib_frame_t * f)
{
  testgovpp_main_t *pm = &testgovpp_main;
  f64 now;
  f64 timeout = 10.0;
  uword *event_data = 0;
  uword event_type;
  int i;

  clib_warning ("testgovpp_cu_up_event_process enter... ");

  while (1)
    {
      if (pm->cu_up_event_timer_enabled)
        vlib_process_wait_for_event_or_clock (vm, timeout);
      else
        vlib_process_wait_for_event (vm);

      now = vlib_time_now (vm);

      event_type = vlib_process_get_events (vm, (uword **) & event_data);
      clib_warning ("testgovpp_cu_up_event_process event_type %d ",event_type);

      switch (event_type)
	{
	  /* Handle TESTGOVPP_EVENT1 */
	case TESTGOVPP_EVENT1:
	  for (i = 0; i < vec_len (event_data); i++)
	    handle_event1 (pm, now, event_data[i]);
	  break;

	  /* Handle TESTGOVPP_EVENT2 */
	case TESTGOVPP_EVENT2:
	  for (i = 0; i < vec_len (event_data); i++)
	    handle_event2 (pm, now, event_data[i]);
	  break;
          /* Handle the periodic timer on/off event */
	case TESTGOVPP_CU_UP_EVENT_ENABLE_DISABLE:
	  for (i = 0; i < vec_len (event_data); i++)
	    handle_cu_up_event_enable_disable (pm, now, event_data[i]);
	  break;

          /* Handle periodic timeouts */
	case ~0:
    handle_cu_up_event_timeout (pm, now);
	  break;
	}
      vec_reset_length (event_data);
    }
  return 0;			/* or not */
}

void testgovpp_create_periodic_process (testgovpp_main_t *tmp)
{
  /* Already created the process node? */
  if (tmp->periodic_node_index > 0)
    return;

  /* No, create it now and make a note of the node index */
  tmp->periodic_node_index = vlib_process_create (tmp->vlib_main,
    "testgovpp-periodic-process",
    testgovpp_periodic_process, 16 /* log2_n_stack_bytes */);
}

void testgovpp_create_cu_up_event_process (testgovpp_main_t *tmp)
{
  /* Already created the process node? */
  if (tmp->cu_up_event_node_index > 0)
    return;

  /* No, create it now and make a note of the node index */
  tmp->cu_up_event_node_index = vlib_process_create (tmp->vlib_main,
    "testgovpp_create_cu_up_event_process",
    testgovpp_cu_up_event_process, 16 /* log2_n_stack_bytes */);
}

/*
 * fd.io coding-style-patch-verification: ON
 *
 * Local Variables:
 * eval: (c-set-style "gnu")
 * End:
 */
