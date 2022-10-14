/** Node-type for uldaq connections.
 *
 * @file
 * @author Manuel Pitz <manuel.pitz@eonerc.rwth-aachen.de>
 * @author Steffen Vogel <svogel2@eonerc.rwth-aachen.de>
 * @copyright 2014-2022, Institute for Automation of Complex Power Systems, EONERC
 * @license Apache 2.0
 *********************************************************************************/

#pragma once

#include <pthread.h>

#include <villas/queue_signalled.h>
#include <villas/pool.hpp>

#include <uldaq.h>

#define ULDAQ_MAX_DEV_COUNT 100
#define ULDAQ_MAX_RANGE_COUNT 8

namespace villas {
namespace node {

struct sample_bucket{
	double *sample_data;
	timespec sample_time;
};

struct uldaq {
	const char *device_id;

	DaqDeviceHandle device_handle;
	DaqDeviceDescriptor *device_descriptor;
	DaqDeviceInterface device_interface_type;

	uint64_t sequence;
	struct {
		bool active = false;
		int channel;
		double variance;
		double level;
		double frequency;
		double deadtime;
	} external_trigger;

	
	struct {
		double sample_rate;
		unsigned trig_smp_count; // number of samples to read when triggered

		size_t buffer_len;
		size_t buffer_pos;
		size_t channel_count;

		ScanOption scan_options;
		AInScanFlag flags;
		AiQueueElement *queues;
		ScanStatus status;
		TransferStatus transfer_status; 

		timespec sample_second;

		pthread_mutex_t mutex;
		
		CQueueSignalled empty_buckets;
		CQueueSignalled full_buckets;

		sample_bucket * current_read_bucket = nullptr;
		sample_bucket * current_write_bucket = nullptr;
	} in;

	struct {
		// TODO
	} out;
};

int uldaq_type_start(SuperNode *sn);

int uldaq_init(NodeCompat *n);

int uldaq_destroy(NodeCompat *n);

int uldaq_parse(NodeCompat *n, json_t *json);

char * uldaq_print(NodeCompat *n);

int uldaq_check(NodeCompat *n);

int uldaq_start(NodeCompat *n);

int uldaq_stop(NodeCompat *n);

int uldaq_read(NodeCompat *n, struct Sample * const smps[], unsigned cnt);


} /* namespace node */
} /* namespace villas */
