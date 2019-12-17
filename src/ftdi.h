/*
 * ftdi.h
 *
 *  Created on: 01.10.2014.
 *      Author: SrkoS
 *
 *
 * * sample:
 * easy open FTDI port

FT_STATUS ftStatus;
FT_HANDLE ftHandle = 0;

bool init_serial_port(void)
{
	uint32_t BaudRate = 1000000;
	uint32_t ReadTimeout = 2000; // Milliseconds

	ftStatus = ftdi_open(BaudRate, ReadTimeout, &ftHandle);

	return ftStatus == FT_OK;
}

 *
 */
#ifndef FTDI_H_
#define FTDI_H_

#include "ftd2xx.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Open first FTDI device that could and configure.
 * Information about opened FTDI device stored in ftHandle.
 *
 * If port already open, this function will close port first.
 *
 * Default timeout for write bytes to FTDI device is 1000 ms
 *
 * @param BaudRate			desired communication speed
 * @param ReadTimeout_ms	timeout for reading from FTDI device in milliseconds
 * @param ftHandle			variable for storing handle for opened device
 * @return		Device status ( FT_STATUS enumeration )
 */
FT_STATUS ftdi_open(uint32_t BaudRate, uint32_t ReadTimeout_ms, FT_HANDLE *ftHandle);

/**
 * This function configure communication speed and timeouts for ALREADY OPENED FTDI device
 *
 * Default timeout for write bytes to FTDI device is 1000 ms
 *
 * @param ftHandle			FTDI device handle ( already opened )
 * @param BaudRate			desired communication speed
 * @param ReadTimeout_ms	timeout for reading from FTDI device in milliseconds
 * @return		Device status ( FT_STATUS enumeration )
 */
FT_STATUS ftdi_configure_hnd(FT_HANDLE ftHandle, uint32_t BaudRate, uint32_t ReadTimeout_ms);

/**
 * The data in the output buffer is sent to the FTDI device.
 *
 * @param ftHandle			FTDI Device handle ( already opened )
 * @param buffer				Output buffer with data for sending to device
 * @param size				Size of the output buffer in bytes
 * @return			0 for OK;
 * 					if return > 0 	that is FT_STATUS enum;
 * 					if return < 0 	that is the difference between the expected and obtained bytes
 */
uint32_t ftdi_write(FT_HANDLE ftHandle, void *buffer, uint32_t size);

/**
 * Device send  data in the output buffer is sent to the device.
 *
 * @param ftHandle			FTDI Device handle ( already opened )
 * @param buffer				Input buffer ( already reserved space in memory )
 * @param size				Expected byte count to read from FTDI device
 * @return			0 for OK;
 * 					if return > 0 	that is FT_STATUS enum;
 * 					if return < 0 	that is the difference between the expected and obtained bytes
 */
uint32_t ftdi_read(FT_HANDLE ftHandle, void *buffer, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* FTDI_H_ */
