#include <villas/nodes/ft4222.hpp>

#include <stdint.h> //uniptr
#include <iostream>

#include <villas/sample.h>
#include <villas/exceptions.hpp>
#include <villas/super_node.hpp>

#include <villas/config.h>
#include <villas/node.h>
#include <villas/exceptions.hpp>
#include <villas/memory.h>
#include <villas/utils.hpp>

/* Forward declartions */
static struct vnode_type p;
void *ft4222_thread_func(void *);

using namespace villas;
using namespace villas::node;
using namespace villas::utils;

/**
 * @brief Helper function to convert the FT_STATUS (and the extended FT4222_STATUS) enum to a printable string.
 * 
 * @param status status code to convert
 * @return const char* status code as string
 */
const char *ft4222_stat_to_str(FT_STATUS status)
{
	FT4222_STATUS ft4222_status = (FT4222_STATUS)status;
	switch (ft4222_status)
	{
	case FT4222_OK:
		return "FT4222_OK";
		break;
	case FT4222_INVALID_HANDLE:
		return "FT4222_INVALID_HANDLE";
		break;
	case FT4222_DEVICE_NOT_FOUND:
		return "FT4222_DEVICE_NOT_FOUND";
		break;
	case FT4222_DEVICE_NOT_OPENED:
		return "FT4222_DEVICE_NOT_OPENED";
		break;
	case FT4222_IO_ERROR:
		return "FT4222_IO_ERROR";
		break;
	case FT4222_INSUFFICIENT_RESOURCES:
		return "FT4222_INSUFFICIENT_RESOURCES";
		break;
	case FT4222_INVALID_PARAMETER:
		return "FT4222_INVALID_PARAMETER";
		break;
	case FT4222_INVALID_BAUD_RATE:
		return "FT4222_INVALID_BAUD_RATE";
		break;
	case FT4222_DEVICE_NOT_OPENED_FOR_ERASE:
		return "FT4222_DEVICE_NOT_OPENED_FOR_ERASE";
		break;
	case FT4222_DEVICE_NOT_OPENED_FOR_WRITE:
		return "FT4222_DEVICE_NOT_OPENED_FOR_WRITE";
		break;
	case FT4222_FAILED_TO_WRITE_DEVICE:
		return "FT4222_FAILED_TO_WRITE_DEVICE";
		break;
	case FT4222_EEPROM_READ_FAILED:
		return "FT4222_EEPROM_READ_FAILED";
		break;
	case FT4222_EEPROM_WRITE_FAILED:
		return "FT4222_EEPROM_WRITE_FAILED";
		break;
	case FT4222_EEPROM_ERASE_FAILED:
		return "FT4222_EEPROM_ERASE_FAILED";
		break;
	case FT4222_EEPROM_NOT_PRESENT:
		return "FT4222_EEPROM_NOT_PRESENT";
		break;
	case FT4222_EEPROM_NOT_PROGRAMMED:
		return "FT4222_EEPROM_NOT_PROGRAMMED";
		break;
	case FT4222_INVALID_ARGS:
		return "FT4222_INVALID_ARGS";
		break;
	case FT4222_NOT_SUPPORTED:
		return "FT4222_NOT_SUPPORTED";
		break;
	case FT4222_OTHER_ERROR:
		return "FT4222_OTHER_ERROR";
		break;
	case FT4222_DEVICE_LIST_NOT_READY:
		return "FT4222_DEVICE_LIST_NOT_READY";
		break;
	case FT4222_DEVICE_NOT_SUPPORTED:
		return "FT4222_DEVICE_NOT_SUPPORTED";
		break;
	case FT4222_CLK_NOT_SUPPORTED:
		return "FT4222_CLK_NOT_SUPPORTED";
		break;
	case FT4222_VENDER_CMD_NOT_SUPPORTED:
		return "FT4222_VENDER_CMD_NOT_SUPPORTED";
		break;
	case FT4222_IS_NOT_SPI_MODE:
		return "FT4222_IS_NOT_SPI_MODE";
		break;
	case FT4222_IS_NOT_I2C_MODE:
		return "FT4222_IS_NOT_I2C_MODE";
		break;
	case FT4222_IS_NOT_SPI_SINGLE_MODE:
		return "FT4222_IS_NOT_SPI_SINGLE_MODE";
		break;
	case FT4222_IS_NOT_SPI_MULTI_MODE:
		return "FT4222_IS_NOT_SPI_MULTI_MODE";
		break;
	case FT4222_WRONG_I2C_ADDR:
		return "FT4222_WRONG_I2C_ADDR";
		break;
	case FT4222_INVAILD_FUNCTION:
		return "FT4222_INVAILD_FUNCTION";
		break;
	case FT4222_INVALID_POINTER:
		return "FT4222_INVALID_POINTER";
		break;
	case FT4222_EXCEEDED_MAX_TRANSFER_SIZE:
		return "FT4222_EXCEEDED_MAX_TRANSFER_SIZE";
		break;
	case FT4222_FAILED_TO_READ_DEVICE:
		return "FT4222_FAILED_TO_READ_DEVICE";
		break;
	case FT4222_I2C_NOT_SUPPORTED_IN_THIS_MODE:
		return "FT4222_I2C_NOT_SUPPORTED_IN_THIS_MODE";
		break;
	case FT4222_GPIO_NOT_SUPPORTED_IN_THIS_MODE:
		return "FT4222_GPIO_NOT_SUPPORTED_IN_THIS_MODE";
		break;
	case FT4222_GPIO_EXCEEDED_MAX_PORTNUM:
		return "FT4222_GPIO_EXCEEDED_MAX_PORTNUM";
		break;
	case FT4222_GPIO_WRITE_NOT_SUPPORTED:
		return "FT4222_GPIO_WRITE_NOT_SUPPORTED";
		break;
	case FT4222_GPIO_PULLUP_INVALID_IN_INPUTMODE:
		return "FT4222_GPIO_PULLUP_INVALID_IN_INPUTMODE";
		break;
	case FT4222_GPIO_PULLDOWN_INVALID_IN_INPUTMODE:
		return "FT4222_GPIO_PULLDOWN_INVALID_IN_INPUTMODE";
		break;
	case FT4222_GPIO_OPENDRAIN_INVALID_IN_OUTPUTMODE:
		return "FT4222_GPIO_OPENDRAIN_INVALID_IN_OUTPUTMODE";
		break;
	case FT4222_INTERRUPT_NOT_SUPPORTED:
		return "FT4222_INTERRUPT_NOT_SUPPORTED";
		break;
	case FT4222_GPIO_INPUT_NOT_SUPPORTED:
		return "FT4222_GPIO_INPUT_NOT_SUPPORTED";
		break;
	case FT4222_EVENT_NOT_SUPPORTED:
		return "FT4222_EVENT_NOT_SUPPORTED";
		break;
	case FT4222_FUN_NOT_SUPPORT:
		return "FT4222_FUN_NOT_SUPPORT";
		break;
	default:
		return "Unknown Error";
		break;
	}
}

/**
 * @brief This function scans for all connected ft4222 devices and puts them in the ft4222_devices array.
 */
int ft4222_gather_devices()
{
	FT_STATUS status = FT_OK;
	Logger log = logging.get("node:ft4222");

	//get number of connected devices from d2xx driver
	DWORD num_devices = 0;
	status = FT_CreateDeviceInfoList(&num_devices);
	size_t num_ft4222 = 0;

	for (DWORD i = 0; i < num_devices; i++)
	{
		//Get info for i-th node
		FT_DEVICE_LIST_INFO_NODE dev_info;
		memset(&dev_info, 0, sizeof(dev_info)); //empty DEV_INFO_NODE

		status = FT_GetDeviceInfoDetail(i, &dev_info.Flags, &dev_info.Type, &dev_info.ID, &dev_info.LocId, dev_info.SerialNumber, dev_info.Description, &dev_info.ftHandle);
		if (status != FT_OK)
		{
			log->debug("Could not get device info. Error: {}\n", ft4222_stat_to_str(status));
			return -1;
		}

		//Check description of the node if a FT4222h has been found
		std::string dev_description = dev_info.Description;
		if (dev_description == "FT4222" || dev_description == "FT4222 A" || dev_description == "FT4222 B" || dev_description == "FT4222 C" || dev_description == "FT4222 D")
		{
			ft4222_devices[num_ft4222] = dev_info; //The device is a ft4222 and is saved
			log->debug("Found FT4222h device with description {}", dev_description);
			num_ft4222++;
		}
	}
	return 0;
}

/**
 * @brief This function parses the .conf configuration file.
 */
int ft4222_parse(struct vnode *n, json_t *json)
{
	//0. Setup values we are going to need
	int ret;
	struct ft4222 *s = (struct ft4222 *)n->_vd;
	json_error_t err;
	int sys_clock_int;
	int vec_int;
	const char *test;
	const char *sig_json_str = nullptr;
	bool use_dumper;

	//1. Node json
	const char *a = json_dumps(json, 0);
	n->logger->debug(a);

	ret = json_unpack_ex(json, &err, 0, "{s?: s,s?: i, s: b, s:{s?: i,s?: i,s: o}}",
						 "type", &test,
						 "system_clock", &sys_clock_int,
						 "dumper", &use_dumper,
						 "in",
						 "sample_rate", &(s->chan_config.sample_rate),
						 "vectorize", &vec_int,
						 "signals", &sig_json_str);

	if (ret < 0)
	{
		n->logger->error("Fehler: {} \t\t At: line:{}\tcol:{}\tsrc:{}\tpos:{}", err.text, err.line, err.column, err.source, err.position);
		throw new RuntimeError("Pasing failed");
	}

	s->chan_config.channel_count = vlist_length(&n->in.signals);
	s->use_dumper = use_dumper;

	return 0;
}

/**
 * @brief TODO
 * 
 * @param n 
 * @return char* 
 */
char *ft_print(struct vnode *n)
{
	//struct ft4222 *s = (struct ft4222 *)n->_vd;
	return strf("Version with dumper");
}

/**
 * @brief This function initializes an instance of a FT4222h node. The device handle is created and the device is opened.
 */
int ft4222_init(struct vnode *n)
{
	FT_STATUS status = FT_OK;
	struct ft4222 *s = (struct ft4222 *)n->_vd;

	//0. Get all FT4222 devices
	if (ft4222_gather_devices() < 0)
	{
		return -1;
	}

	//1. Open the first available device.
	n->logger->debug("Opening Device {}", ft4222_devices[0].Description);
	status = FT_OpenEx((PVOID)(uintptr_t)ft4222_devices[0].LocId, FT_OPEN_BY_LOCATION, &s->dev_handle);

	if (status != FT_OK)
	{
		n->logger->error("FT4222 FT_OpenEx failed with with {}\n", ft4222_stat_to_str(status));
		return -1;
	}

	if (&s->dev_handle == NULL)
	{
		n->logger->error("Did not open a device\n");
		return -1;
	}

	return 0;
}

/**
 * @brief This function starts an instance of a FT4222h node. The chip is configured as SPI slave. Clock and drive strength are set and the thread is started.
 */
int ft4222_start(struct vnode *n)
{
	FT_STATUS status = FT_OK;
	struct ft4222 *s = (struct ft4222 *)n->_vd;

	//0. Init as slave device to start reading in data on the chip
	status = FT4222_SPISlave_InitEx(s->dev_handle, SPI_SLAVE_NO_PROTOCOL);
	if (status != FT_OK)
	{
		n->logger->error("Init device as SPI Slave failed with error: {}\n", ft4222_stat_to_str(status));
		FT_Close(&s->dev_handle);
		return -1;
	}

	//1. Set internal clock to 80 Mhz
	status = FT4222_SetClock(s->dev_handle, FT4222_ClockRate::SYS_CLK_80);
	if (status != FT_OK)
	{
		n->logger->error("Setting clock failed with error: {}\n", ft4222_stat_to_str(status));
		FT_Close(&s->dev_handle);
		return -1;
	}

	//2. Set SPI Driving strenght
	status = FT4222_SPI_SetDrivingStrength(s->dev_handle, DS_12MA, DS_16MA, DS_16MA);
	if (status != FT_OK)
	{
		n->logger->error("Setting device driving strength failed with error: {}\n", ft4222_stat_to_str(status));
		FT_Close(&s->dev_handle);
		return -1;
	}

	//3. Create socket to dump values to for debugging reasons, if set by the
	if (s->use_dumper)
	{
		n->logger->debug("Created dumper at /tmp/ft4222_dump");
		s->raw_dumper = new villas::node::Dumper("/tmp/ft4222_dump");
	}

	//4. Start background read thread
	s->thread_args.buffer_parsed = new uint16_t[FT4222_BUFFER_SIZE];
	s->thread_args.working_buffer = new uint8[FT4222_D2XX_BUFFER_SIZE];
	s->thread_args.write_head = 0;
	s->thread_args.read_head = 0;
	s->thread_args.sem_parsed = (sem_t *)malloc(sizeof(sem_t));
	s->thread_args.sem_protect = (sem_t *)malloc(sizeof(sem_t));
	s->read_thread = (pthread_t *)malloc(sizeof(pthread_t));

	sem_init(s->thread_args.sem_parsed, false, 0);
	sem_init(s->thread_args.sem_protect, false, FT4222_BUFFER_SIZE);
	s->chan_config.sequece = 0;
	s->thread_args.is_running = true;

	pthread_create(s->read_thread, NULL, ft4222_thread_func, s);
	return 0;
}

/**
 * @brief This function destroys an instance of a Ft4222h node. 
 */
int ft4222_destroy(struct vnode *n)
{

	struct ft4222 *s = (struct ft4222 *)n->_vd;

	n->logger->debug("Deleting FT4222\n");

	//Stop thread
	s->thread_args.is_running = false;
	pthread_join(*(s->read_thread), NULL);

	//Free memory
	delete[] s->thread_args.buffer_parsed;
	delete[] s->thread_args.working_buffer;
	free(s->read_thread);
	free(s->thread_args.sem_parsed);

	//De-Init FT4222
	FT4222_UnInitialize(&s->dev_handle);
	FT_Close(&s->dev_handle);
	if (&s->use_dumper)
	{
		delete &s->raw_dumper;
	}
	return 0;
}

/**
 * @brief This function runs as a paralell thread in the background and is the main interaction point with the FT4222h
 * It fills the XXX buffer with parsed 12 bit samples and monitores for overflows
 * 
 */
void *ft4222_thread_func(void *thread_args)
{
	ft4222 *s = (ft4222 *)thread_args;
	FT4222_STATUS status = FT4222_STATUS::FT4222_OK;
	Logger log = logging.get("node:ft4222:read_thread");
	log->debug("Enter FT4222 thread_func");
	uint16 rxSize = 0;
	uint16 read_data = 0;
	uint8 chan_pos = 0;
	while (s->thread_args.is_running && FT4222_SPISlave_GetRxStatus(s->dev_handle, &rxSize) == FT4222_OK)
	{
		if (rxSize == FT4222_D2XX_BUFFER_SIZE)
		{ //FT4222 buffer overflow
			log->error("d2xx buffer overflow. Disregarding {} values", rxSize / 1.5);
			status = FT4222_SPI_ResetTransaction(s->dev_handle, 0x00);
			//throw new RuntimeError("Resetted spi\n");
		}

		if (rxSize > 0)
		{
			log->debug("rxSize={}\n", rxSize);		//Data in FT4222 buffer
			uint16 to_read = rxSize - (rxSize % 3); //Only read from FT4222 Buffer alligned to 3 bytes to prevent a sample from getting chopped up
			if(to_read == 0)
				continue;
			status = FT4222_SPISlave_Read(s->dev_handle, s->thread_args.working_buffer, to_read, &read_data);
			assert(read_data == to_read);
			if (status != FT4222_OK)
			{
				log->error("Read failed");
				return nullptr;
			}
			//Parse samples into buffer
			for (int i = 0; i < to_read / 3; i++)
			{ //Read out 3 bytes and create two samples
				short a = s->thread_args.working_buffer[3 * i];
				short b = s->thread_args.working_buffer[3 * i + 1];
				short c = s->thread_args.working_buffer[3 * i + 2];

				uint16 smp1 = (a << 4) | ((b & 0xF0) >> 4);
				uint16 smp2 = ((b & 0x0F) << 8) | (c);

				if (chan_pos == 2 && smp1 < 1000)
				{
					log->error("SPI allignment error. WH={}, i={}", s->thread_args.write_head, i);
					//throw new RuntimeError("asd");
				}
				s->thread_args.buffer_parsed[s->thread_args.write_head] = smp1;
				s->thread_args.write_head++; //Buffer size is even, so we dont need to check for an overflow here
				chan_pos++;

				s->thread_args.buffer_parsed[s->thread_args.write_head] = smp2;
				s->thread_args.write_head++;
				chan_pos++;

				if (s->thread_args.write_head >= FT4222_BUFFER_SIZE)
				{
					s->thread_args.write_head = 0;
				}

				if (chan_pos >= s->chan_config.channel_count)
				{
					chan_pos = 0;
					sem_post(s->thread_args.sem_parsed); //Signal the arrival of a new entire sample
														 //Check for overflow
					int val = 0;
					sem_getvalue(s->thread_args.sem_parsed, &val);
					if (val > FT4222_BUFFER_SIZE / 8)
					{
						log->error("Internal buffer overflow!\n");
						throw new RuntimeError("Internal buffer overflow.\n");
						s->thread_args.is_running = false;
					}
				}
			}
		}
	}
	log->debug("Leaving thread_func");
	return nullptr;
}

/**
 * @brief This function fetches read data from the from the raw buffers in the background
 * 
 * @param n 
 * @param smps 
 * @param cnt 
 * @return int 
 */
int hack = 0;
int hack2 = 20;
int seq = 0;
int ft4222_read(struct vnode *n, struct sample *const smps[], unsigned cnt)
{
	struct ft4222 *s = (struct ft4222 *)n->_vd;
	Logger log = logging.get("node:ft4222");
	//0. Check if we have enough room for cnt requested samples
	assert(smps[0]->capacity >= s->chan_config.channel_count);

	//Read samples
	int smp_available = 0;
	int to_read = 0;
	sem_getvalue(s->thread_args.sem_parsed, &smp_available);
	if (smp_available > (int)cnt)
	{
		log->warn("More samples available in buffer {} then requested to read {}\n", smp_available, cnt);
		to_read = cnt;
	}
	else
	{
		to_read = smp_available;
	}

	//log->debug("Reading {} samples\n",to_read);

	for (int i = 0; i < to_read; i++)
	{
		sem_trywait(s->thread_args.sem_parsed);
		for (size_t j = 0; j < s->chan_config.channel_count; j++)
		{
			smps[i]->data[j].f = s->thread_args.buffer_parsed[s->thread_args.read_head];

			//sem_post(s->thread_args.sem_protect); //Signal space for new buffer
			//Wrap around
			if (++(s->thread_args.read_head) >= FT4222_BUFFER_SIZE)
			{
				s->thread_args.read_head = 0;
			}
		}

		if (hack % 50 == 0 && smps[i]->data[2].f < 2)
		{
			log->error("This should not happen");
			log->error("WH={}\nRH={}\n", s->thread_args.write_head, s->thread_args.read_head);
			hack2--;
		}
		if (hack2 == 0)
		{
			throw new RuntimeError("Errorororo");
		}
		smps[i]->sequence = s->chan_config.sequece++;
		smps[i]->length = s->chan_config.channel_count;
		smps[i]->signals = &n->in.signals;
		smps[i]->flags = (int)SampleFlags::HAS_SEQUENCE | (int)SampleFlags::HAS_DATA;
	};

	return to_read;
}

__attribute__((constructor(110))) static void register_plugin()
{
	p.name = "ft4222";
	p.description = "Node Type for a FT4222h USB to SPI/I2C bridge IC by FTDI";
	p.vectorize = 100;
	p.size = sizeof(struct ft4222);

	p.init = ft4222_init;
	p.destroy = ft4222_destroy;

	p.start = ft4222_start;
	//p.stop = ft4222_stop;
	p.read = ft4222_read;

	p.parse = ft4222_parse;
	p.print = ft_print;

	if (!node_types)
		node_types = new NodeTypeList();

	node_types->push_back(&p);
}
