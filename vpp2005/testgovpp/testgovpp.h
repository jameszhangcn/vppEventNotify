
/*
 * testgovpp.h - skeleton vpp engine plug-in header file
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
#ifndef __included_testgovpp_h__
#define __included_testgovpp_h__

#include <vnet/vnet.h>
#include <vnet/ip/ip.h>
#include <vnet/ethernet/ethernet.h>

#include <vppinfra/hash.h>
#include <vppinfra/error.h>

#define MAX_GTP_ERROR_IND_DATA_LEN (20)

typedef struct {
    /* API message ID base */
    u16 msg_id_base;

    /* on/off switch for the periodic function */
    u8 periodic_timer_enabled;
    u8 cu_up_event_timer_enabled;
    /* Node index, non-zero if the periodic process has been created */
    u32 periodic_node_index;
    u32 cu_up_event_node_index;

    /* convenience */
    vlib_main_t * vlib_main;
    vnet_main_t * vnet_main;
    ethernet_main_t * ethernet_main;
} testgovpp_main_t;

extern testgovpp_main_t testgovpp_main;

extern vlib_node_registration_t testgovpp_node;
extern vlib_node_registration_t testgovpp_periodic_node;

/* Periodic function events */
#define TESTGOVPP_EVENT1 1
#define TESTGOVPP_EVENT2 2
#define TESTGOVPP_EVENT_PERIODIC_ENABLE_DISABLE 3
#define TESTGOVPP_CU_UP_EVENT_ENABLE_DISABLE 4

void testgovpp_create_periodic_process (testgovpp_main_t *);
void testgovpp_create_timer_process (testgovpp_main_t *);
void testgovpp_create_cu_up_event_process(testgovpp_main_t *);
#endif /* __included_testgovpp_h__ */

/*
 * fd.io coding-style-patch-verification: ON
 *
 * Local Variables:
 * eval: (c-set-style "gnu")
 * End:
 */

