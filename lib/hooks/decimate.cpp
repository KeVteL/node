/** Decimate hook.
 *
 * @author Steffen Vogel <svogel2@eonerc.rwth-aachen.de>
 * @copyright 2014-2022, Institute for Automation of Complex Power Systems, EONERC
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

#include <villas/hooks/decimate.hpp>
#include <villas/sample.hpp>

namespace villas {
namespace node {

void DecimateHook::start()
{
	assert(state == State::PREPARED);

	counter = 0;

	state = State::STARTED;
}


void DecimateHook::parseTimeString(std::string timeStr);

	// split data into time and unit
	int numMode = true;
	std::string numStr, unitStr;

	//remove leading spaces
	while (timeStr[0] == ' ')
	{
		timeStr = timeStr.substr(1,timeStr.length() -1);
	};

	for (char c : timeStr)
	{	
		if(numMode){
			//Write numbers and digits to numStri	
			if(std::isdigit(c) || c == '.'){	
				numStr += c;
			}else{
				numMode = false;
			}
		}else{
			unitStr += c;
		}
	}

}

void DecimateHook::parse(json_t *json)
{
	int ret;
	json_error_t err;

	assert(state != State::STARTED);

	Hook::parse(json);

	std::string everySI;
	std::string allignSI;

	ret = json_unpack_ex(json, &err, 0, "{ s: i, s?: b, s?:s, s?:s }",
		"ratio", &ratio,
		"renumber", &renumber,
		"every", &everySI,
		"allign",&allignSI
	);

	if (ret)
		throw ConfigError(json, err, "node-config-hook-decimate");


	//Todo: Parse everySI string.


	//Todo: Parse allign string

	//ToDo: make sure, either ratio or every string are supplied. Not both.



	state = State::PARSED;
}


/*
ToDo: Currently, the first sample that is let through by this hook is the 
Sync String: 5m
*/
Hook::Reason DecimateHook::process(struct Sample *smp)
{
	assert(state == State::STARTED);

	if (renumber)
		smp->sequence /= ratio;

	if (ratio && counter++ % ratio != 0)
		return Hook::Reason::SKIP_SAMPLE;

	return Reason::OK;
}

/* Register hook */
static char n[] = "decimate";
static char d[] = "Downsamping by integer factor";
static HookPlugin<DecimateHook, n, d, (int) Hook::Flags::NODE_READ | (int) Hook::Flags::NODE_WRITE | (int) Hook::Flags::PATH> p;

} /* namespace node */
} /* namespace villas */
