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
 * @brief This function starts an instance of aFT4222h node. The chip is configured as SPI slave. Clock and drive strength are set.
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
	if(s->use_dumper){
		s->raw_dumper = new villas::node::Dumper("/tmp/ft4222_dump");
		n->logger->debug("Created dumper at /tmp/ft4222_dump");	
	}

	s->sequece = 0;
	return 0;
}

/**
 * @brief This function destroys an instance of a Ft4222h node. 
 */
int ft4222_destroy(struct vnode *n)
{
	//Todo: Kill thread and stuff
	struct ft4222 *s = (struct ft4222 *)n->_vd;
	FT4222_UnInitialize(&s->dev_handle);
	FT_Close(&s->dev_handle);
	if(&s->use_dumper){
		delete &s->raw_dumper;
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
						 "sample_rate", &(s->sample_rate),
						 "vectorize", &vec_int,
						 "signals", &sig_json_str);

	if (ret < 0)
	{
		n->logger->error("Fehler: {} \t\t At: line:{}\tcol:{}\tsrc:{}\tpos:{}", err.text, err.line, err.column, err.source, err.position);
		throw new RuntimeError("Pasing failed");
	}

	s->channel_count = vlist_length(&n->in.signals);
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
 * @brief This function fetches read data from the from the raw buffers in the background
 * 
 * @param n 
 * @param smps 
 * @param cnt 
 * @return int 
 */
 bool hmmErr = false;
 int hack, hack1 = 0;
int ft4222_read(struct vnode *n, struct sample *const smps[], unsigned cnt)
{
	struct ft4222 *u = (struct ft4222 *)n->_vd;
		Logger log = logging.get("node:ft4222");

	//The libFT4222 internaly maintains its own buffer on the host computer that is supplied with the data from the FT4222h
	//This function only needs to read out this buffer.

	//0. Confirm that enough space is ready for the data.
	//const int needed_space = n->in.vectorize * u->channel_count * (3.0 / 2.0);

	//assert(cnt == n->in.vectorize);													   //Enough samples
	//assert(smps[0]->capacity >= u->channel_count);									   //Large enough samples
	//assert((float)needed_space == (n->in.vectorize * u->channel_count * (3.0 / 2.0))); //This confirms that there will be no sample that is split between this read and the next one

	//1. Confirm that there is enough data in the libft4222 buffer to get 100 smp packages
	uint16_t data_in_buffer;
	
	/*do
	{
		FT4222_SPISlave_GetRxStatus(u->dev_handle, &available_space);
		//n->logger->debug("Data in buffer: {}",available_space);
	} while (available_space <= needed_space);
*/

	FT4222_SPISlave_GetRxStatus(u->dev_handle, &data_in_buffer);
	
	
	if(++hack%1 == 0){
			log->debug("start Data in buffer: {}\n", data_in_buffer);
	}
	
	//if(data_in_buffer <= needed_space){
	//	return 0; //Trywait
	//}

	//2. Read out data and sort into sample packages
	uint16 read_data;
	uint8 buffer[data_in_buffer];
	if(data_in_buffer < u->channel_count * (3.0 / 2.0))
		return 0;

	/*if(data_in_buffer == 65535){
		throw RuntimeError("whatever1");
	}*/

	uint16 toRead = (data_in_buffer>=1200)?1200:data_in_buffer;
	FT4222_SPISlave_Read(u->dev_handle, buffer, toRead, &read_data);
	cnt = (int)(toRead/u->channel_count/1.5);
	//assert(read_data == toRead);


	FT_STATUS tmpStat = FT4222_SPISlave_GetRxStatus(u->dev_handle, &data_in_buffer);

	if (tmpStat != FT_OK ){
		log->debug("FTERRRR\n\n");
		throw RuntimeError("whatever");
	}


	if(++hack%1 == 0){
			log->debug("end Data in buffer: {}\n", data_in_buffer);
	}
	for (size_t i = 0; i < cnt; i++) //Loop over all samples we want
	{
		struct sample *smp = smps[i];

		int row_start = i * u->channel_count * (3.0 / 2.0);

		for (size_t chan_index = 0; chan_index < u->channel_count; chan_index++)
		{
			//Channel index is multiplied by 3.0/2.0 = 1.5 to move 12 bit forward with every channel
			//Channel index is then cast to an int to allign back with 8 bit array.
			int chan_start = ((int)(chan_index * 1.5)) + row_start;
			int chan_allign = chan_start % 3;

			if (chan_allign == 0) //Bit 11 alligns with start of buffer value
			{
				short a = (buffer[chan_start] << 4);
				short b = (((buffer[chan_start + 1]) & 0xF0) >> 4);
				smp->data[chan_index].f = (float) (a | b);
			}
			else if (chan_allign == 1) //Bit 11 is in the middle of a buffer value
			{
				smp->data[chan_index].f = (((buffer[chan_start]) & 0x0F) << 8) |
										  (buffer[chan_start + 1]);
			}
			else //Allignment error
			{
				throw new RuntimeError("Allignment faliure");
			}
			u->raw_dumper->writeDataBinary(1, &(smp->data[chan_index].f));
		}
		if (hack > 100 && smp->data[2].f < 1 && !hmmErr) {
			hmmErr = true;
			log->debug("Something just went wrong!!\n\n\n");
		}else if(hmmErr && hack1 < 100)
			hack1++;
		else if(hmmErr){
			log->debug("Booom!!\n\n\n");
			throw new RuntimeError("Big error");
		}
		smp->length = u->channel_count;
		smp->signals = &n->in.signals;
		smp->sequence = u->sequece++;
		smp->flags = (int)SampleFlags::HAS_SEQUENCE | (int)SampleFlags::HAS_DATA;
	}
	return cnt;
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
