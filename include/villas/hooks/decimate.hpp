/** Decimate hook.
 *
 * @author Steffen Vogel <svogel2@eonerc.rwth-aachen.de>
 * @copyright 2014-2022, Institute for Automation of Complex Power Systems, EONERC
 * @license Apache 2.0
 *********************************************************************************/

#pragma once

#include <villas/hook.hpp>
#include <tuple>

namespace villas {
namespace node {

class DecimateHook : public LimitHook {

protected:
	int ratio;
	bool renumber;
	unsigned counter;
	long every;
	timespec lastSample;

	// enum TimeScale{
	// 	nS = 0,
	// 	uS = 3,
	// 	mS = 6,
	// 	S = 9,
	// 	m = 12,
	// 	h = 12,
	// 	d = 13
	// };

	enum Mode{
		RatioMode,
		RatioAllignMode,
		EveryMode,
		EveryAllignMode
	};


public:
	using LimitHook::LimitHook;

	virtual
	void setRate(double rate, double maxRate = -1)
	{
		assert(maxRate > 0);

		int ratio = maxRate / rate;
		if (ratio == 0)
			ratio = 1;

		setRatio(ratio);
	}

	void setRatio(int r)
	{
		ratio = r;
	}

	virtual
	void start();

	virtual
	void parse(json_t *json);

	virtual
	Hook::Reason process(struct Sample *smp);

private:
	DecimateHook::Mode pMode;

	long parseTimeString(std::string) const;
};

} /* namespace node */
} /* namespace villas */

