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

#include <libft4222.h> //FT4222h vendor API
#include <ftd2xx.h> //D2XX Driver

#define FT4222_SIGLE_BUF_SIZE 2048
#define FT4222_FULL_BUF_SIZE (3*FT4222_SIGLE_BUF_SIZE)

//Macros for easier bit manipulation when reading out the sample array
#define GET_NEXT_24_BIT(ARRAY,INDEX) ((ARRAY[(INDEX)]) | ((ARRAY[(INDEX)+1]) << 8) | ((ARRAY[(INDEX)+2]) << 16)) //This macro reads out 3 bytes of data in ARRAY and puts it in the 24 lower byts of a 32 bit int in this order |EMPTY|+2|+1|+0
#define UPPER_SMP(DATA) (((DATA) & 0xFFF000) >> 12); //This macro gets the upper 12 bit of the data produced by the macro above
#define LOWER_SMP(DATA) ((DATA) & 0xFFF); //This macro gets the lower 12 bits of the data produced by GET_NETXT_24_bit

static FT_DEVICE_LIST_INFO_NODE ft4222_devices[4]; //There can be at most 4 FT devices at a time


struct ft4222
{
    /* Device */
    FT_HANDLE dev_handle;

    struct
    {
        double sample_rate;
        size_t channel_count;
        long long unsigned int sequece;
    };
    
    


    
};


