/** Node type: IEC 61850-9-2 (Sampled Values)
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2017, Institute for Automation of Complex Power Systems, EONERC
 * @license GNU General Public License (version 3)
 *
 * VILLASnode
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************************/

/**
 * @addtogroup iec61850_sv IEC 61850-9-2 (Sampled Values) node type
 * @ingroup node
 * @{
 */

#pragma once

#include <stdint.h>

#include <libiec61850/sv_publisher.h>
#include <libiec61850/sv_subscriber.h>

#include <villas/queue_signalled.h>
#include <villas/pool.h>
#include <villas/node.h>
#include <villas/list.h>
#include <villas/nodes/iec61850.h>

#ifdef __cplusplus
extern "C"{
#endif

struct iec61850_sv {
	char *interface;
	int app_id;
	struct ether_addr dst_address;

	struct {
		bool enabled;

		SVSubscriber subscriber;
		SVReceiver receiver;

		struct queue_signalled queue;
		struct pool pool;

		struct list mapping;		/**< Mappings of type struct iec61850_type_descriptor */
		int total_size;
	} subscriber;

	struct {
		bool enabled;

		SVPublisher publisher;
		SVPublisher_ASDU asdu;

		char *svid;

		int vlan_priority;
		int vlan_id;
		int smpmod;
		int smprate;
		int confrev;

		struct list mapping;		/**< Mappings of type struct iec61850_type_descriptor */
		int total_size;
	} publisher;
};

int iec61850_sv_init(struct super_node *sn);

int iec61850_sv_deinit();

int iec61850_sv_parse(struct node *n, json_t *json);

char * iec61850_sv_print(struct node *n);

int iec61850_sv_start(struct node *n);

int iec61850_sv_stop(struct node *n);

int iec61850_sv_destroy(struct node *n);

int iec61850_sv_read(struct node *n, struct sample *smps[], unsigned cnt);

int iec61850_sv_write(struct node *n, struct sample *smps[], unsigned cnt);

int iec61850_sv_fd(struct node *n);

#ifdef __cplusplus
}
#endif

/** @} */
