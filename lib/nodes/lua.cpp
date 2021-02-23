/** Lua node-type
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2014-2021, Institute for Automation of Complex Power Systems, EONERC
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

#include <villas/nodes/example.hpp>
#include <villas/utils.hpp>
#include <villas/sample.h>
#include <villas/plugin.h>
#include <villas/super_node.hpp>

/* Forward declartions */
static struct plugin p;

using namespace villas::node;
using namespace villas::utils;

int lua_type_start(villas::node::SuperNode *sn)
{
	/* TODO: Add implementation here */

	return 0;
}

int lua_type_stop()
{
	/* TODO: Add implementation here */

	return 0;
}

int lua_init(struct vnode *n)
{
	struct lua *l = (struct lua *) n->_vd;

	/* TODO: Add implementation here. The following is just an example */

	s->setting1 = 0;
	s->setting2 = nullptr;

	return 0;
}

int lua_destroy(struct vnode *n)
{
	struct lua *l = (struct lua *) n->_vd;

	/* TODO: Add implementation here. The following is just an example */

	if (s->setting2)
		free(s->setting2);

	return 0;
}

int lua_parse(struct vnode *n, json_t *cfg)
{
	int ret;
	struct lua *l = (struct lua *) n->_vd;

	json_error_t err;

	/* TODO: Add implementation here. The following is just an example */

	ret = json_unpack_ex(cfg, &err, 0, "{ s?: i, s?: s }",
		"setting1", &s->setting1,
		"setting2", &s->setting2
	);
	if (ret)
		jerror(&err, "Failed to parse configuration of node %s", node_name(n));

	return 0;
}

char * lua_print(struct vnode *n)
{
	struct lua *l = (struct lua *) n->_vd;

	/* TODO: Add implementation here. The following is just an example */

	return strf("setting1=%d, setting2=%s", s->setting1, s->setting2);
}

int lua_check(struct vnode *n)
{
	struct lua *l = (struct lua *) n->_vd;

	/* TODO: Add implementation here. The following is just an example */

	if (s->setting1 > 100 || s->setting1 < 0)
		return -1;

	if (!s->setting2 || strlen(s->setting2) > 10)
		return -1;

	return 0;
}

int lua_prepare(struct vnode *n)
{
	struct lua *l = (struct lua *) n->_vd;

	/* TODO: Add implementation here. The following is just an example */

	s->state1 = s->setting1;

	if (strcmp(s->setting2, "double") == 0)
		s->state1 *= 2;

	return 0;
}

int lua_start(struct vnode *n)
{
	struct lua *l = (struct lua *) n->_vd;

	/* TODO: Add implementation here. The following is just an example */

	s->start_time = time_now();

	return 0;
}

int lua_stop(struct vnode *n)
{
	//struct lua *l = (struct lua *) n->_vd;

	/* TODO: Add implementation here. */

	return 0;
}

int lua_pause(struct vnode *n)
{
	//struct lua *l = (struct lua *) n->_vd;

	/* TODO: Add implementation here. */

	return 0;
}

int lua_resume(struct vnode *n)
{
	//struct lua *l = (struct lua *) n->_vd;

	/* TODO: Add implementation here. */

	return 0;
}

int lua_read(struct vnode *n, struct sample *smps[], unsigned cnt, unsigned *release)
{
	int read;
	struct lua *l = (struct lua *) n->_vd;
	struct timespec now;

	/* TODO: Add implementation here. The following is just an example */

	assert(cnt >= 1 && smps[0]->capacity >= 1);

	now = time_now();

	smps[0]->data[0].f = time_delta(&now, &s->start_time);

	/* Dont forget to set other flags in struct sample::flags
	 * E.g. for sequence no, timestamps... */
	smps[0]->flags = (int) SampleFlags::HAS_DATA;
	smps[0]->signals = &n->in.signals;

	read = 1; /* The number of samples read */

	return read;
}

int lua_write(struct vnode *n, struct sample *smps[], unsigned cnt, unsigned *release)
{
	int written;
	//struct lua *l = (struct lua *) n->_vd;

	/* TODO: Add implementation here. */

	written = 0; /* The number of samples written */

	return written;
}

int lua_reverse(struct vnode *n)
{
	//struct lua *l = (struct lua *) n->_vd;

	/* TODO: Add implementation here. */

	return 0;
}

int lua_poll_fds(struct vnode *n, int fds[])
{
	//struct lua *l = (struct lua *) n->_vd;

	/* TODO: Add implementation here. */

	return 0; /* The number of file descriptors which have been set in fds */
}

int lua_netem_fds(struct vnode *n, int fds[])
{
	//struct lua *l = (struct lua *) n->_vd;

	/* TODO: Add implementation here. */

	return 0; /* The number of file descriptors which have been set in fds */
}

__attribute__((constructor(110)))
static void register_plugin() {
	p.name			= "lua";
	p.description		= "Produce and consume samples by arbitrary Lua code";
	p.type			= PluginType::NODE;
	p.node.instances.state	= State::DESTROYED;
	p.node.vectorize	= 0;
	p.node.size		= sizeof(struct lua);
	p.node.type.start	= lua_type_start;
	p.node.type.stop	= lua_type_stop;
	p.node.init		= lua_init;
	p.node.destroy		= lua_destroy;
	p.node.prepare		= lua_prepare;
	p.node.parse		= lua_parse;
	p.node.print		= lua_print;
	p.node.check		= lua_check;
	p.node.start		= lua_start;
	p.node.stop		= lua_stop;
	p.node.pause		= lua_pause;
	p.node.resume		= lua_resume;
	p.node.read		= lua_read;
	p.node.write		= lua_write;
	p.node.reverse		= lua_reverse;
	p.node.poll_fds		= lua_poll_fds;
	p.node.netem_fds	= lua_netem_fds;

	int ret = vlist_init(&p.node.instances);
	if (!ret)
		vlist_init_and_push(&plugins, &p);
}

__attribute__((destructor(110)))
static void deregister_plugin() {
	vlist_remove_all(&plugins, &p);
}
