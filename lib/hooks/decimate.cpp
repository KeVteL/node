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



DecimateHook::time_unit_tuple DecimateHook::parseTimeString(std::string timeStr) const
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
	
	double time = std::stod(numStr);
	std::transform(unitStr.begin(), unitStr.end(), unitStr.begin(),[](unsigned char c){ return std::tolower(c); });
	TimeUnit unit;


	if(unitStr == "ns")
		unit = nS;

	if(unitStr == "us")
		unit = uS;
		
	if(unitStr == "ms")
		unit = mS;
		
	if(unitStr == "s")
		unit = S;
		
	if(unitStr == "m")
		unit = m;
		
	if(unitStr == "h")
		unit = h;
	
	if(unitStr == "d")
		unit = d;

	if(unitStr == ""){
		throw new RuntimeError("No Unit supplied");
	}

	return std::make_tuple(time,unit);

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

	//Ensure, only ratio or every or every + allign is supplied
	if(ratio > 0 && everyStr != nullptr){
		throw ConfigError(json,"node-config-hook-decimate","Cant use both ratio and every at the same time. Please select only one option.");
	}

	//Select right mode
	if(ratio > 0){
		pMode = RatioMode;
	}
	if(everyStr != nullptr){
		pMode = EveryMode;
		auto everyStrParsed = parseTimeString(everyStr);
		double everyD = std::get<0>(everyStrParsed);
		switch (std::get<1>(everyStrParsed))
		{
		case nS:
			everyD *= 1;
			break;
		case uS:
			everyD *= 1E3;
			break;
		case mS:
			everyD *= 1E6;
			break;
		case S:
			everyD *= 1E9;
			break;
		case m:
			everyD *= 6E10;
			break;
		case h:
			everyD *= 3.6E12;
			break;
		case d:
			everyD *= 8.64E13;
			break;
		default:
			throw ConfigError(json,"node-config-hook-decimate","Failed to parse 'Every' String.");
			break;
		}
		if(everyD != (long) everyD){
			throw ConfigError(json,"node-config-hook-decimate","Cant use time steps < 1nS. Please increase");
		}
		every = (long) everyD;
	}

	//Select allign
	timespec_get(&lastSample,TIME_UTC);
	if(allignStr != nullptr){
		pMode = (Mode) (((int) pMode) + 2);
		auto allignStrParsed = parseTimeString(allignStr);

		if(std::get<0>(allignStrParsed) != (int) std::get<0>(allignStrParsed)){
			throw ConfigError(json,"node-config-hook-decimate","Cant allign to fractions. Please use a smaller timescale.");
		}

		switch (std::get<1>(allignStrParsed))
		{
		case nS: //Alling to begin of next uS
			lastSample.tv_nsec -= (lastSample.tv_nsec % 1000);
			lastSample.tv_nsec += (int)std::get<0>(allignStrParsed);
			break;
		case uS: //Allign to begin of next mS
			lastSample.tv_nsec -= (lastSample.tv_nsec % 1000000);
			lastSample.tv_nsec += ((int)std::get<0>(allignStrParsed)*1000);
			break;
		case mS://Allign to begin of next S
			lastSample.tv_nsec -= 0;
			lastSample.tv_nsec += (int)std::get<0>(allignStrParsed);
			break;
		case S://Allign to beginn of next min
			lastSample.tv_nsec = 0;
			lastSample.tv_sec -= (lastSample.tv_sec % 60);
			lastSample.tv_sec += (int)std::get<0>(allignStrParsed);
			break;
		case m://Allign to begin of next h
			lastSample.tv_nsec = 0;
			lastSample.tv_sec -= (lastSample.tv_sec % (60*60));
			lastSample.tv_sec += (int)std::get<0>(allignStrParsed);
			break;
		case h://Allign to begin of next d
			lastSample.tv_nsec = 0;
			lastSample.tv_sec -= (lastSample.tv_sec % (60*60*24));
			lastSample.tv_sec += (int)std::get<0>(allignStrParsed);
			break;
		case d:
			throw ConfigError(json,"node-config-hook-decimate","Cant allign to days. Please choose a smaller time unit.");
			break;
		default:
			throw ConfigError(json,"node-config-hook-decimate","Failed to parse 'Every' String.");
			break;
		}		
	}

	if(pMode == RatioAllignMode)
		throw ConfigError(json,"node-config-hook-decimate","Ratio allign is not yet implemented.");
	state = State::PARSED;
}


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
	if(pMode == EveryMode || pMode == EveryAllignMode){
		logger->debug("Every");
		if(((smp->ts.origin.tv_sec * 1E9 +  smp->ts.origin.tv_nsec) - ( lastSample.tv_sec*1E9 + lastSample.tv_nsec)) < every){
			return Reason::SKIP_SAMPLE;
		}
	}

	//Mode: Every Allign
	//if(pMode == EveryAllignMode){
	//	throw RuntimeError("Not Implemented yet!");
	//}
	

	lastSample = smp->ts.origin;
	return Reason::OK;
}



/* Register hook */
static char n[] = "decimate";
static char d[] = "Downsamping by integer factor with optional syncronisation";
static HookPlugin<DecimateHook, n, d, (int) Hook::Flags::NODE_READ | (int) Hook::Flags::NODE_WRITE | (int) Hook::Flags::PATH> p;

} /* namespace node */
} /* namespace villas */
