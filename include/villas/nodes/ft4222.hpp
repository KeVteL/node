/** Node type for communicating with a FT4222h device by FTDI
 * @file ft4222.hpp
 * @author Vincent Bareiss (vincent.bareiss@rwth-aachen.de)
 * @brief 
 * @version 0.1
 * @date 2021-07-07
 * 
 * @copyright Copyright (c) 2021
 * 
 */


#pragma once
#include <pthread.h> //Multithreading

#include <villas/dumper.hpp>

#include <libft4222.h> //FT4222h vendor API
#include <ftd2xx.h> //D2XX Driver

static FT_DEVICE_LIST_INFO_NODE ft4222_devices[4]; //There can be at most 4 FT devices at a time

struct ft4222
{
    /* Device */
    FT_HANDLE dev_handle;
    villas::node::Dumper *raw_dumper;

    struct
    {
        double sample_rate;
        size_t channel_count;
        long long unsigned int sequece;
    };
};


