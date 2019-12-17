/*
 * ftdi.c
 *
 *  Created on: 01.10.2014.
 *      Author: SrkoS
 */

#include "ftdi.h"
#include <string.h>
#include <stdlib.h>

FT_STATUS ftdi_open(uint32_t BaudRate, uint32_t ReadTimeout_ms, FT_HANDLE *ftHandle)
{
	FT_STATUS ftStatus;
	int ftdi_index = 0;

	if (ftHandle == (FT_HANDLE *) NULL)
		return FT_INVALID_HANDLE;

	if (*ftHandle)
	{
		FT_Close(*ftHandle);
		*ftHandle = 0;
	}

	do
	{
		ftStatus = FT_Open(ftdi_index, ftHandle);

		if (ftStatus == 2)
			return ftStatus; // no other FTDI device

        ftdi_index++;
	} while (ftStatus);

	// OK / open !

	ftStatus = ftdi_configure_hnd(*ftHandle, BaudRate, ReadTimeout_ms);
	if (ftStatus)
	{
		FT_Close(*ftHandle);
		*ftHandle = 0;
	}

	return ftStatus;
}

/**
 * configure opened handle
 * @return
 */
FT_STATUS ftdi_configure_hnd(FT_HANDLE ftHandle, uint32_t speed, uint32_t timeout_ms)
{
	FT_STATUS ftStatus;
	uint8_t lat_timer = 2; // DEFAULT_LATENCYTIMER;

#if DEBUG_STD

	uint32_t dwDriverVer;
	ftStatus = FT_GetDriverVersion(ftHandle, (DWORD *) &dwDriverVer);
	if (ftStatus == FT_OK)
	printf("FTDI Driver version = 0x%x", dwDriverVer);

#endif

	ftStatus = FT_OK;

//Turn off bit bang mode
	ftStatus |= FT_SetBitMode(ftHandle, 0x00, 0);
// Reset the device
	ftStatus = FT_ResetDevice(ftHandle);
// Purge transmit and receive buffers
	ftStatus |= FT_Purge(ftHandle, FT_PURGE_RX | FT_PURGE_TX);
// Set the baud rate
	ftStatus |= FT_SetBaudRate(ftHandle, speed);
// 1 s timeouts on read / write
	ftStatus |= FT_SetTimeouts(ftHandle, timeout_ms, 1000);
// Set to communicate at 8N1
	ftStatus |= FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_1,
	FT_PARITY_NONE); // 8N1
// Disable hardware / software flow control
	ftStatus |= FT_SetFlowControl(ftHandle, FT_FLOW_NONE, 0, 0);
// Set the latency of the receive buffer way down (2 ms) to facilitate speedy transmission
	ftStatus |= FT_SetLatencyTimer(ftHandle, lat_timer);

//	if (ftStatus != FT_OK)
//	{
//		ReaderCloseHnd();
//
//	}

// debug
//	ftStatus = FT_GetLatencyTimer(ftHandle, &lat_timer);
//	dbg_str(DBG_USB, "LatencyTimer(%d)=> %d\n", DEFAULT_LATENCYTIMER,
//			(int) lat_timer);

//	update_ftdi_info();

	return ftStatus;
}

//---------------------------------------------------------------------------
uint32_t ftdi_write(FT_HANDLE ftHandle, void *buffer, uint32_t size)
{
	uint32_t bytes_written;
	FT_STATUS ft_status;

	ft_status = FT_Write(ftHandle, buffer, size, (LPDWORD) &bytes_written);
	if (ft_status)
		return ft_status;

	if (bytes_written != size)
		return -abs(bytes_written - size);

	return 0;
}
//------------------------------------------------------------------------------
uint32_t ftdi_read(FT_HANDLE ftHandle, void *buffer, uint32_t size)
{
	uint32_t bytes_returned;
	FT_STATUS ft_status;

	memset(buffer, 0, size);

	ft_status = FT_Read(ftHandle, buffer, size, (LPDWORD) &bytes_returned);
	if (ft_status)
		return ft_status;

	if (bytes_returned != size)
		return -abs(bytes_returned - size);

	return 0;
}
//------------------------------------------------------------------------------
