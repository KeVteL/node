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
#include <semaphore.h>

#include <villas/dumper.hpp>

#include <libft4222.h> //FT4222h vendor API
#include <ftd2xx.h>    //D2XX Driver

static FT_DEVICE_LIST_INFO_NODE ft4222_devices[4]; //There can be at most 4 FT devices at a time

#define FT4222_D2XX_BUFFER_SIZE 65535
#define FT4222_BUFFER_SIZE 40000 //5k sample packages

struct ft4222
{
    /* Device */
    FT_HANDLE dev_handle;
    villas::node::Dumper *raw_dumper;
    bool use_dumper;
    pthread_t *read_thread;

    struct
    {
        double sample_rate;
        size_t channel_count;
        long long unsigned int sequece;
    } chan_config;

    struct
    {
        uint16_t *buffer_parsed; //buffer queue for final values.
        uint8 *working_buffer;    //working buffer to parse samples in
        sem_t *sem_parsed; //number of clean values in buffer_parsed
        sem_t *sem_protect; //number of clean values in buffer_parsed
        bool is_running;
        size_t write_head,read_head;
    } thread_args;
};
