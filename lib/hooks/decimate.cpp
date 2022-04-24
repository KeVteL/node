/** Decimate hook.
 *
 * @author Steffen Vogel <svogel2@eonerc.rwth-aachen.de>
 * @copyright 2014-2022, Institute for Automation of Complex Power Systems, EONERC
 * @license Apache 2.0
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
	TimeUnit unit;

	if(unitStr == "nS")
		unit = nS;

	if(unitStr == "uS")
		unit = uS;
		
	if(unitStr == "mS")
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


	//opt 1: Ratio. Decreases the samplerate by a given ratio (Only every nth sample gets though)
	//Opt 2: Every. Decreases the samplerate such that a sample gets though every X Sec. Mutually exclusive to Ratio.
	//Opt 3: Allign. Can only be used with every. Selects the first sample that is let through.

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
	if(allignStr != nullptr){
		pMode = (Mode) (((int) pMode) + 2);
		auto allignStrParsed = parseTimeString(allignStr);
		
	}else{
		timespec_get(&lastSample,TIME_UTC);
	}

	if(pMode == RatioAllignMode)
		throw ConfigError(json,"node-config-hook-decimate","Ratio allign is not yet implemented.");
	if(pMode == EveryAllignMode)
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
