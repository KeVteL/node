/** The API ressource for querying statistics.
 *
 * @author Steffen Vogel <post@steffenvogel.de>
 * @copyright 2014-2022, Institute for Automation of Complex Power Systems, EONERC
 * @license Apache 2.0
 *********************************************************************************/

#include <jansson.h>

#include <villas/node.hpp>
#include <villas/stats.hpp>
#include <villas/super_node.hpp>
#include <villas/utils.hpp>
#include <villas/api.hpp>
#include <villas/api/session.hpp>
#include <villas/api/requests/node.hpp>
#include <villas/api/response.hpp>

namespace villas {
namespace node {
namespace api {

class StatsRequest : public NodeRequest {

public:
	using NodeRequest::NodeRequest;

	virtual Response * execute()
	{
		if (method != Session::Method::GET)
			throw InvalidMethod(this);

		if (body != nullptr)
			throw BadRequest("Stats endpoint does not accept any body data");

		if (node->getStats() == nullptr)
			throw BadRequest("The statistics collection for this node is not enabled");

		return new JsonResponse(session, HTTP_STATUS_OK, node->getStats()->toJson());
	}
};

/* Register API requests */
static char n[] = "node/stats";
static char r[] = "/node/(" RE_NODE_NAME "|" RE_UUID ")/stats";
static char d[] = "get internal statistics counters";
static RequestPlugin<StatsRequest, n, r, d> p;

} /* namespace api */
} /* namespace node */
} /* namespace villas */
