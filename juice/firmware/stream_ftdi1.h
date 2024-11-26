#ifndef __RADIOBERRY_IQ_STREAM_H__
#define  __RADIOBERRY_IQ_STREAM_H__


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ftdi.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define STREAM_FAILURE -1

int init_stream(void);

int read_stream(unsigned char stream[]);

int write_stream(unsigned char stream[]);

int deinit_stream(void);

#endif
