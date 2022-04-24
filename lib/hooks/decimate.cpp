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



long DecimateHook::parseTimeString(std::string timeStr) const
{
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
			if(std::isdigit(c) || c == '.'){	
				numStr += c;
			}else{
				numMode = false;
				unitStr+=c;
			}
		}else{
			unitStr += c;
		}
	}
	
	long timeInNS = std::stod(numStr);

	if(unitStr == "nS")
		timeInNS = timeInNS * pow(10,0);

	if(unitStr == "uS")
		timeInNS = timeInNS * pow(10,3);
		
	if(unitStr == "mS")
		timeInNS = timeInNS * pow(10,6);
		
	if(unitStr == "s")
		timeInNS = timeInNS * pow(10,9);
		
	if(unitStr == "m")
		timeInNS = timeInNS * 6 * pow(10,10);
		
	if(unitStr == "h")
		timeInNS = timeInNS * 3.6 * pow(10,12);
	
	if(unitStr == "d")
		timeInNS = timeInNS * 8.64 * pow(10,14);

	if(unitStr == ""){
		throw new RuntimeError("No Unit supplied");
	}

	return timeInNS;

}

void DecimateHook::parse(json_t *json)
{
	int ret;
	json_error_t err;

	assert(state != State::STARTED);

	Hook::parse(json);

	const char *everyStr = nullptr;
	const char *allignStr = nullptr;
	ratio = -1;

	ret = json_unpack_ex(json, &err, 0, "{ s?: i, s?: b, s?:s, s?:s }",
		"ratio", &ratio,
		"renumber", &renumber,
		"every", &everyStr,
		"allign",&allignStr
	);

	if (ret)
		throw ConfigError(json, err, "node-config-hook-decimate");


	//opt 1: Ratio. Decreases the samplerate by a given ratio (Only every nth sample gets though)
	//Opt 2: Every. Decreases the samplerate such that a sample gets though every X Sec.
	//Opt 3: Allign. Can only be used with every. Selects the first sample that is let through.

	//Ensure, only ratio or every or every + allign is supplied
	if(ratio > 0 && everyStr != nullptr){
		throw ConfigError(json,"node-config-hook-decimate","Cant use both ratio and every at the same time. Please select only one option.");
	}

	//Select right mode
	if(ratio > 0){
		pMode = RatioMode;
	}
	if(ratio > 0 && allignStr != nullptr){
		pMode = RatioAllignMode;
		throw ConfigError(json,"node-config-hook-decimate","Ratio allign is not yet implemented.");
	}
	if(everyStr != nullptr){
		pMode = EveryMode;
		every = parseTimeString(everyStr);
	}

	timespec_get(&lastSample,TIME_UTC);

	state = State::PARSED;
}


/*
ToDo: Currently, the first sample that is let through by this hook is the first one that is 
Sync String: 5m
*/
Hook::Reason DecimateHook::process(struct Sample *smp)
{
	assert(state == State::STARTED);

	if (renumber)
		smp->sequence /= ratio;

	//Mode: Ratio
	if(pMode == RatioMode){
		logger->debug("Ratio");
		if (ratio && counter++ % ratio != 0)
			return Hook::Reason::SKIP_SAMPLE;
	}
	
	//Mode: Ratio Allign
	if(pMode == RatioAllignMode){
		throw RuntimeError("Not Implemented yet!");
	}

	//Mode: Every
	if(pMode == EveryMode){
		logger->debug("Every");
		if(((smp->ts.origin.tv_sec * 1E9 +  smp->ts.origin.tv_nsec) - ( lastSample.tv_sec*1E9 + lastSample.tv_nsec)) < every){
			return Reason::SKIP_SAMPLE;
		}
	}

	//Mode: Every Allign
	if(pMode == EveryAllignMode){
		throw RuntimeError("Not Implemented yet!");
	}
	

	lastSample = smp->ts.origin;
	return Reason::OK;
}



/* Register hook */
static char n[] = "decimate";
static char d[] = "Downsamping by integer factor with optional syncronisation";
static HookPlugin<DecimateHook, n, d, (int) Hook::Flags::NODE_READ | (int) Hook::Flags::NODE_WRITE | (int) Hook::Flags::PATH> p;

} /* namespace node */
} /* namespace villas */
