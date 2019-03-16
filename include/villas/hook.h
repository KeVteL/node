/** Hook functions
 *
 * Every node or path can register hook functions which is called for every
 * processed sample. This can be used to debug the data flow, get statistics
 * or alter the sample contents.
 *
 * This file includes some examples.
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2014-2019, Institute for Automation of Complex Power Systems, EONERC
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
 */

/**
 * @addtogroup hooks User-defined hook functions
 * @ingroup path
 * @{
 *********************************************************************************/

#pragma once

#include <villas/hook_type.h>
#include <villas/list.h>
#include <villas/common.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct path;
struct sample;

enum hook_reason {
	HOOK_OK,
	HOOK_ERROR,
	HOOK_SKIP_SAMPLE,
	HOOK_STOP_PROCESSING
};

/** Descriptor for user defined hooks. See hooks[]. */
struct hook {
	enum state state;

	int enabled;		/**< Is this hook active? */
	int priority;		/**< A priority to change the order of execution within one type of hook. */

	struct path *path;
	struct node *node;

	struct vlist signals;

	struct hook_type *_vt;	/**< C++ like Vtable pointer. */
	void *_vd;		/**< Private data for this hook. This pointer can be used to pass data between consecutive calls of the callback. */

	json_t *cfg;		/**< A JSON object containing the configuration of the hook. */
};

int hook_init(struct hook *h, struct hook_type *vt, struct path *p, struct node *n);

int hook_init_signals(struct hook *h, struct vlist *signals);

int hook_parse(struct hook *h, json_t *cfg);

int hook_prepare(struct hook *h, struct vlist *signals);

int hook_destroy(struct hook *h);

int hook_start(struct hook *h);

int hook_stop(struct hook *h);

int hook_periodic(struct hook *h);

int hook_restart(struct hook *h);

int hook_process(struct hook *h, struct sample *smp);

/** Compare two hook functions with their priority. Used by vlist_sort() */
int hook_cmp_priority(const void *a, const void *b);

static inline
struct hook_type * hook_type(struct hook *h)
{
	return h->_vt;
}

#ifdef __cplusplus
}
#endif
