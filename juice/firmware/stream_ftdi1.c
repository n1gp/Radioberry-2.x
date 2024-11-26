/*
 * N1GP - 11/25/2024
 * This is an alternate stream module that uses libftdi1
 * https://www.intra2net.com/en/developer/libftdi
 *
 * I am able to stream 10 RXs using the radioberry_10rx.rbf
 * gateware.
 *
 */

#include "stream_ftdi1.h"

struct ftdi_context *ftdi;

int init_stream()
{
    char manufacturer[128], description[128];
    struct ftdi_device_list *devlist;
    int res;
    int num;
    unsigned int chunksize=4096;

    printf("Init device for iq stream handling...\r\n");

    if ((ftdi = ftdi_new()) == 0)
    {
        fprintf(stderr, "ftdi_new failed\n");
        goto out_init;
    }

    if (ftdi_set_interface(ftdi, INTERFACE_A) < 0)
    {
        fprintf(stderr, "ftdi_set_interface failed\n");
        goto free_out_init;
    }

    if ((num = ftdi_usb_find_all(ftdi, &devlist, 0, 0)) < 0)
    {
        fprintf(stderr, "No FTDI with default VID/PID found\n");
        goto free_out_init;
    }

    if (num == 1)
    {
        if ((res = ftdi_usb_get_strings(ftdi, devlist[0].dev, manufacturer, 128, description, 128, NULL, 0)) < 0)
        {
            fprintf(stderr, "ftdi_usb_get_strings failed: %d (%s)\n", res, ftdi_get_error_string(ftdi));
            goto free_dev_out_init;
        }

        if ((res = strncmp(description, "radioberry-juice", 16)) != 0)
        {
            fprintf(stderr, "Failed to find a radioberry-juice :%s:\n", description);
            goto free_dev_out_init;
        }

        res = ftdi_usb_open_dev(ftdi,  devlist[0].dev);
        if (res<0)
        {
            fprintf(stderr, "Unable to open device: (%s)",
                    ftdi_get_error_string(ftdi));
            goto free_dev_out_init;
        }
    }
    else if (num == 0)
    {
        fprintf(stderr, "No Devices found with default VID/PID\n");
        goto free_dev_out_init;
    }

    ftdi_list_free(&devlist);

    if (ftdi_set_bitmode(ftdi,  0xFF, BITMODE_SYNCFF) < 0)
    {
        fprintf(stderr,"Can't set bitmode, Error %s\n",ftdi_get_error_string(ftdi));
        goto free_out_init;
        ftdi_usb_close(ftdi);
        goto free_out_init;
    }

    if(ftdi_set_latency_timer(ftdi, 2))
    {
        fprintf(stderr,"Can't set latency, Error %s\n",ftdi_get_error_string(ftdi));
        ftdi_usb_close(ftdi);
        goto free_out_init;
    }

    if(ftdi_setflowctrl(ftdi, SIO_RTS_CTS_HS))
    {
        fprintf(stderr,"Can't set flow control, Error %s\n",ftdi_get_error_string(ftdi));
        ftdi_usb_close(ftdi);
        goto free_out_init;
    }

    if(ftdi_usb_purge_buffers(ftdi))
    {
        fprintf(stderr,"Can't purge buffers, Error %s\n",ftdi_get_error_string(ftdi));
        ftdi_usb_close(ftdi);
        goto free_out_init;
    }

    ftdi->usb_read_timeout = 1000;
    ftdi->usb_write_timeout = 1000;

    ftdi_read_data_set_chunksize(ftdi, chunksize);
    ftdi_read_data_get_chunksize(ftdi, &chunksize);
    printf("Init device succeeded for iq streaming using FT245 protocol. chunksize:%d\r\n", chunksize);

    return 0;

free_dev_out_init:
    ftdi_list_free(&devlist);
free_out_init:
    ftdi_free(ftdi);
out_init:
    return STREAM_FAILURE;
}

int deinit_stream()
{
    printf("Close device  for streaming.\r\n");
    ftdi_set_bitmode(ftdi,  0, 0);

    if(ftdi_usb_purge_buffers(ftdi))
    {
        fprintf(stderr,"Can't purge buffers, Error %s\n",ftdi_get_error_string(ftdi));
    }

    ftdi_usb_close(ftdi);
    ftdi_free(ftdi);

    return 0;
}

int write_stream(unsigned char stream[])
{
    unsigned int numBytesSent;

    numBytesSent = ftdi_write_data(ftdi, stream, 1032);

    if (1032 != numBytesSent)
    {
        fprintf(stderr, "ds stream time not complete; only %d bytes sent \n", numBytesSent);
        return -1;
    }

    return 0;
}

int read_stream(unsigned char stream[])
{
    unsigned int BytesReceived;

    BytesReceived = ftdi_read_data(ftdi, stream, 1032);

    if (1032 != BytesReceived)
    {
        fprintf(stderr, "us stream read not complete \n");
        return -1;
    }

    return 0;
}

// End of source.,
