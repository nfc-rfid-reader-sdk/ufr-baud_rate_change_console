#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "http.h"
#include "ftdi.h"

#if __WIN32__
#	include <windows.h>
#else
#	include <unistd.h>
#	define Sleep(ms) usleep((ms)*1000)
#endif

FT_STATUS ftStatus;
FT_HANDLE ftHandle = 0;
DWORD dwWritten;
DWORD dwRead;

uint8_t set_uart_speed_cmd[] = {0x55, 0x70, 0xAA, 0x05, 0x00, 0x00, 0x91};
uint8_t set_uart_speed_ack[] = {0xAC, 0x70, 0xCA, 0x05, 0x00, 0x00, 0x1A};
uint8_t set_uart_speed_reply[] = {0xDE, 0x70, 0xED, 0x00, 0x00, 0x00, 0x4A};
uint8_t default_speed_cmd[] = {0x55, 0xF1, 0xAA, 0x00, 0x01, 0x01, 0x15};
uint8_t default_speed_reply[] = {0xDE, 0xF1, 0xED, 0x00, 0x01, 0x01, 0xC9};
uint8_t boot_reply[] = {0x03, 0x55, 0x55, 0xBB};
uint8_t br_data[10];

BOOL init_serial_port(uint32_t BaudRate, int reader_type)
{
	uint32_t ReadTimeout = 1000; // Milliseconds

	ftStatus = ftdi_open(BaudRate, ReadTimeout, &ftHandle);
    if(ftStatus)
        return FALSE;

    if(reader_type == 1)
    {
        ftStatus = FT_SetRts(ftHandle);
    }
    else
    {
       ftStatus = FT_ClrRts(ftHandle);
    }

 	return ftStatus == FT_OK;
}

BOOL init_serial_port_2(int reader_type)
{
	uint32_t ReadTimeout = 1000; // Milliseconds
	uint32_t BaudRate;

    if(reader_type == 1)
    {
        BaudRate = 115200;
    }
    else
    {
        BaudRate = 1000000;
    }

	ftStatus = ftdi_open(BaudRate, ReadTimeout, &ftHandle);
    if(ftStatus)
        return FALSE;

    if(reader_type == 1)
    {
        ftStatus = FT_ClrRts(ftHandle);
        Sleep(10);
        ftStatus = FT_SetRts(ftHandle);
    }
    else
    {
        ftStatus = FT_SetRts(ftHandle);
        Sleep(10);
        ftStatus = FT_ClrRts(ftHandle);
    }

 	return ftStatus == FT_OK;
}

void send_command(unsigned char *cmd)
{
    FT_Purge(ftHandle, FT_PURGE_RX | FT_PURGE_TX);

    ftdi_write(ftHandle, cmd, 7);
}

BOOL receive_response(unsigned char *res_data, unsigned long len)
{
    ftStatus = ftdi_read(ftHandle, res_data, len);

    return ftStatus == FT_OK;
}

void end_of_program(void)
{
    printf("\nPress ENTER to exit\n");
    getchar();
}

BOOL get_reader_type(int *reader_type)
{
    uint32_t ReadTimeout = 1000; // Milliseconds
	uint8_t res_data[5];
    uint32_t BaudRate;
    //try port on 1000000bps
    *reader_type = 0;

    BaudRate = 1000000;

	ftStatus = ftdi_open(BaudRate, ReadTimeout, &ftHandle);
    if(ftStatus)
        return FALSE;

    ftStatus = FT_SetRts(ftHandle);
    if(ftStatus)
    {
        FT_Close(ftHandle);
        return FALSE;
    }
    Sleep(10);
    ftStatus = FT_ClrRts(ftHandle);
    if(ftStatus)
    {
        FT_Close(ftHandle);
        return FALSE;
    }

    if(receive_response(res_data, 4))
    {
       *reader_type = 2;
       FT_Close(ftHandle);
       return TRUE;
    }

    //try port on 115200
    BaudRate = 115200;

	ftStatus = ftdi_open(BaudRate, ReadTimeout, &ftHandle);
    if(ftStatus)
        return FALSE;

    ftStatus = FT_ClrRts(ftHandle);
    if(ftStatus)
    {
        FT_Close(ftHandle);
        return FALSE;
    }
    Sleep(10);
    ftStatus = FT_SetRts(ftHandle);
    if(ftStatus)
    {
        FT_Close(ftHandle);
        return FALSE;
    }

    if(receive_response(res_data, 4))
    {
       *reader_type = 1;
       FT_Close(ftHandle);
       return TRUE;
    }
    else
    {
        FT_Close(ftHandle);
        return FALSE;
    }
}


int main()
{
    char str[10];
    uint32_t baud_rate_1, baud_rate_2, baud_rate;
    uint8_t res_data[10];
    uint8_t i, crc;
    int reader_type, operation_code;

    if(!get_reader_type(&reader_type))
    {
        printf("Reader not found \n");
        end_of_program();
        return EXIT_FAILURE;
    }

    printf("Select operation\n");
    printf(" 1 - Set UART baud rate\n");
    printf(" 2 - Set default baud rate\n");

    scanf("%d%*c", &operation_code);

    if(operation_code != 1 && operation_code != 2)
    {
        printf("Wrong choice\n");
        end_of_program();
        return EXIT_FAILURE;
    }

    if(operation_code == 1)
    {
        //Set UART baud rate
        printf("Enter current baud rate value\n");
        scanf("%[^\n]%*c", str);
        baud_rate_1 = strtol(str, NULL, 10);

        printf("\nEnter new baud rate value\n");
        scanf("%[^\n]%*c", str);
        baud_rate_2 = strtol(str, NULL, 10);

        if(!init_serial_port(baud_rate_1, reader_type))
        {
            printf("\nPort did not open\n");
            end_of_program();
            return EXIT_FAILURE;
        }

        Sleep(100);

        send_command(set_uart_speed_cmd);

        if(!receive_response(res_data, 7))
        {
            printf("\nCommand acknowledgment did not received\n");
            FT_Close(ftHandle);
            end_of_program();
            return EXIT_FAILURE;
        }

        if(memcmp(res_data, set_uart_speed_ack, 7))
        {
            printf("\nCommand acknowledgment error\n");
            FT_Close(ftHandle);
            end_of_program();
            return EXIT_FAILURE;
        }

        memcpy(br_data, &baud_rate_2, 4);
        crc = 0;
        for(i = 0; i < 4; i++)
            crc ^= br_data[i];
        crc += 7;
        br_data[4] = crc;

        ftdi_write(ftHandle, br_data, 5);

        Sleep(10);

        ftStatus = FT_SetBaudRate(ftHandle, baud_rate_2);
        if(ftStatus)
        {
            printf("\nChange baud rate error\n");
            FT_Close(ftHandle);
            end_of_program();
            return EXIT_FAILURE;
        }

        if(!receive_response(res_data, 7))
        {
            printf("\nCommand reply did not received\n");
            FT_Close(ftHandle);
            end_of_program();
            return EXIT_FAILURE;
        }

        if(memcmp(res_data, set_uart_speed_reply, 7))
        {
            printf("\nCommand reply error\n");
            FT_Close(ftHandle);
            end_of_program();
            return EXIT_FAILURE;
        }

        printf("\nBaud rate has been changed successfully\n");
        FT_Close(ftHandle);
        end_of_program();
        return EXIT_SUCCESS;
    }
    else
    {
        if(!init_serial_port_2(reader_type))
        {
            printf("\nPort did not open");
            end_of_program();
            return EXIT_FAILURE;
        }

        if(!receive_response(res_data, 4))
        {
            printf("\nCommand reply did not received");
            FT_Close(ftHandle);
            end_of_program();
            return EXIT_FAILURE;
        }

        send_command(default_speed_cmd);

        if(!receive_response(res_data, 7))
        {
            printf("\nCommand reply did not received");
            FT_Close(ftHandle);
            end_of_program();
            return EXIT_FAILURE;
        }

        if(memcmp(res_data, default_speed_reply, 7))
        {
            printf("\nCommand reply error");
            FT_Close(ftHandle);
            end_of_program();
            return EXIT_FAILURE;
        }

        Sleep(1000);

        send_command(set_uart_speed_cmd);

        if(!receive_response(res_data, 7))
        {
            printf("\nCommand acknowledgment did not received");
            FT_Close(ftHandle);
            end_of_program();
            return EXIT_FAILURE;
        }

        if(memcmp(res_data, set_uart_speed_ack, 7))
        {
            printf("\nCommand acknowledgment error");
            FT_Close(ftHandle);
            end_of_program();
            return EXIT_FAILURE;
        }

        if(reader_type == 1)
            baud_rate = 115200;
        else
            baud_rate = 1000000;

        memcpy(br_data, &baud_rate, 4);
        crc = 0;
        for(i = 0; i < 4; i++)
            crc ^= br_data[i];
        crc += 7;
        br_data[4] = crc;

        ftdi_write(ftHandle, br_data, 5);

        if(!receive_response(res_data, 7))
        {
            printf("\nCommand reply did not received");
            FT_Close(ftHandle);
            end_of_program();
            return EXIT_FAILURE;
        }

        if(memcmp(res_data, set_uart_speed_reply, 7))
        {
            printf("\nCommand reply error");
            FT_Close(ftHandle);
            end_of_program();
            return EXIT_FAILURE;
        }

        printf("\nDefault baud rate has been returned successfully");
        FT_Close(ftHandle);
        end_of_program();
        return EXIT_SUCCESS;
    }
}

