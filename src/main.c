/*
 ============================================================================
 Project Name: project_name
 Name        : file_name.c
 Author      : d-logic (http://www.d-logic.net/nfc-rfid-reader-sdk/)
 Version     :
 Copyright   : 2017.
 Description : Project in C (Language standard: c99)
 Dependencies: uFR firmware - min. version x.y.z {define in ini.h}
               uFRCoder library - min. version x.y.z {define in ini.h}
 ============================================================================
 */

/* includes:
 * stdio.h & stdlib.h are included by default (for printf and LARGE_INTEGER.QuadPart (long long) use %lld or %016llx).
 * inttypes.h, stdbool.h & string.h included for various type support and utilities.
 * conio.h is included for windows(dos) console input functions.
 * windows.h is needed for various timer functions (GetTickCount(), QueryPerformanceFrequency(), QueryPerformanceCounter())
 */
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#if __WIN32 || __WIN64
#	include <conio.h>
#	include <windows.h>
#elif linux || __linux__ || __APPLE__
//#	define __USE_MISC
#	include <unistd.h>
#	include <termios.h>
//#	undef __USE_MISC
#	include "conio_gnu.h"
#else
#	error "Unknown build platform."
#endif
#include <uFCoder.h>
#include "ini.h"
#include "uFR.h"
#include "utils.h"
//------------------------------------------------------------------------------
void usage(void);
void menu(char key);
UFR_STATUS NewCardInField(uint8_t sak, uint8_t *uid, uint8_t uid_size);
void set_baud_rate(void);
void set_default_baud_rate(void);
void test_signal(void);

uint8_t brs = 0;
//------------------------------------------------------------------------------
int main(void)
{
	char key;
	bool card_in_field = false;
	uint8_t old_sak = 0, old_uid_size = 0, old_uid[10];
	uint8_t sak, uid_size, uid[10];
	UFR_STATUS status;

	char mode;
	printf("Select reader opening mode:\n");
	printf(" (1) - Simple Reader Open\n");
	printf(" (2) - Advanced Reader Open\n");
	printf(" (3) - Set default baud rate\n");
	mode = _getch();
	fflush(stdin);
	if (mode == '1')
	{
		status = ReaderOpen();
	}
	else if (mode == '2')
	{
	   uint32_t reader_type = 1;
	   char port_name[1024] = "";
	   uint32_t port_interface = 2;
	   char open_args[1024] = "";
	   char str_interface[2] = "";
	   char port_exist[2] = "";

	   printf("Enter reader type:\n");
	   scanf("%d", &reader_type);
	   fflush(stdin);

	   printf("Does exist port name (y or n)\n");
	   scanf("%s", port_exist);
	   fflush(stdin);
	   if(port_exist[0] == 'y' || port_exist[0] == 'Y')
	   {
		   printf("Enter port name:\n");
		   scanf("%s", port_name);
		   fflush(stdin);
	   }

	   printf("Enter port interface:\n");
	   scanf("%s", str_interface);
	   if (str_interface[0] == 'U')
		   port_interface = 85;
	   else if (str_interface[0] == 'T')
		   port_interface = 84;
	   else
		   port_interface = atoi(str_interface);

	   fflush(stdin);

	   printf("Enter additional argument:\n");
	   printf("Custom baud rate\n");
	   printf("BR_1000000\n");
	   printf("BR_115200\n");
	   printf("BR_250000\n");
	   printf("BR_9600\n");
	   printf("BR_19200\n");
	   printf("BR_38400\n");
	   printf("BR_57600\n");
	   printf("BR_230400\n");
	   printf("BR_460800\n");
	   printf("BR_500000\n");
	   printf("\n");

	   scanf("%s", open_args);
	   fflush(stdin);

	    status = ReaderOpenEx(reader_type, port_name, port_interface, open_args);
	}
	else if(mode == '3')
	{
		set_default_baud_rate();
		printf("Press any key to quit the application...");
		getchar();
		return EXIT_SUCCESS;
	}
	else
	{
		printf("Invalid input. Press any key to quit the application...");
		getchar();
		return EXIT_FAILURE;
	}

	if (status != UFR_OK)
	{
		printf("Error while opening device 1, status is: 0x%08X\n", status);
		getchar();
		return EXIT_FAILURE;
	}

	if (!CheckDependencies())
	{
		ReaderClose();
		getchar();
		return EXIT_FAILURE;
	}

	printf(" --------------------------------------------------\n");
	printf("        uFR NFC reader successfully opened.\n");
	printf(" --------------------------------------------------\n");

	usage();

#if linux || __linux__ || __APPLE__
	_initTermios(0);
#endif
	do
	{
		key = _getch();
		menu(key);
	}
	while (key != '\x1b' && brs == 0);

	if(brs == 1)
	{
		printf("Press any key to quit the application...");
		getchar();
	}

	ReaderClose();
#if linux || __linux__ || __APPLE__
	_resetTermios();
	tcflush(0, TCIFLUSH); // Clear stdin to prevent characters appearing on prompt
#endif
	return EXIT_SUCCESS;
}
//------------------------------------------------------------------------------
void menu(char key)
{

	switch (key)
	{
		case '1':
			set_baud_rate();
			break;

		case '2':
			test_signal();
			break;

		case '\x1b':
			break;

		default:
			usage();
			break;
	}
}
//------------------------------------------------------------------------------
void usage(void)
{
		printf(" +------------------------------------------------+\n"
			   " |       Change reader baud rate example          |\n"
			   " |                 version "APP_VERSION"                    |\n"
			   " +------------------------------------------------+\n"
			   "                              For exit, hit escape.\n");
		printf(" --------------------------------------------------\n");
		printf("  (1) - Set baud rate\n"
			   "  (2) - Test signal\n");
}
//------------------------------------------------------------------------------

void set_baud_rate(void)
{
	printf(" -------------------------------------------------------------------\n");
	printf("                           Set baud rate                            \n");
	printf(" -------------------------------------------------------------------\n");

	char key;
	uint32_t baud_rate;
	UFR_STATUS status;

	printf("\n Select baud rate\n");
	printf(" (1) - 9600 bps\n");
	printf(" (2) - 19200 bps\n");
	printf(" (3) - 38400 bps\n");
	printf(" (4) - 57600 bps\n");
	printf(" (5) - 115200 bps\n");
	printf(" (6) - 230400 bps\n");
	printf(" (7) - 250000 bps\n");
	printf(" (8) - 460800 bps\n");
	printf(" (9) - 500000 bps\n");
	printf(" (a) - 1000000 bps\n");

	while (!_kbhit())
			;
	key = _getch();

	switch(key)
	{
	case '1':
		baud_rate = 9600;
		break;
	case '2':
		baud_rate = 19200;
		break;
	case '3':
		baud_rate = 38400;
		break;
	case '4':
		baud_rate = 57600;
		break;
	case '5':
		baud_rate = 115200;
		break;
	case '6':
		baud_rate = 230400;
		break;
	case '7':
		baud_rate = 250000;
		break;
	case '8':
		baud_rate = 460800;
		break;
	case '9':
		baud_rate = 500000;
		break;
	case 'a':
	case 'A':
		baud_rate = 1000000;
		break;

	default:
		baud_rate = 0;
	}

	status = SetUartSpeed(baud_rate);

	if(status)
	{
		printf("\nChange baud rate failed\n");
		printf("Error code = 0x%08X\n", status);
	}
	else
	{
		printf("\nBaud rate changed successfully\n");
		printf("New baud rate is %d\n", baud_rate);
		brs = 1;
	}
}
//------------------------------------------------------------------------------
void set_default_baud_rate(void)
{
	printf(" -------------------------------------------------------------------\n");
	printf("                     Set default baud rate                          \n");
	printf("              uFR nano and CS 1 Mbps, RS232 115200 bps              \n");
	printf(" -------------------------------------------------------------------\n");

	UFR_STATUS status;
	uint8_t reader_type;
	int reader_type_int;
    char port_name[100] = "";
    uint8_t comm_type;
    int comm_type_int;

    printf("Enter reader type (1 - USB, 2 - RS232):\n");
    scanf("%d%*c", &reader_type_int);
    reader_type = reader_type_int;
    if(reader_type != 1 && reader_type != 2)
    {
    	printf("\nWrong choice\n");
    	return;
    }

    printf("Enter port interface type (1 - COM port, 2 - FTDI):\n");
	scanf("%d%*c", &comm_type_int);
	comm_type = comm_type_int;
	if(comm_type != 1 && comm_type != 2)
	{
		printf("\nWrong choice\n");
		return;
	}

	if(comm_type == 1)
	{
	   printf("Enter port name:\n");
	   scanf("%s", port_name);
	   fflush(stdin);
	}

	status = SetDefaultUartSpeed(reader_type, comm_type, port_name);

   if(status)
   {
	   printf("\nSet default baud rate failed\n");
	   printf("Error code = 0x%08X\n", status);
   }
   else
   {
	   printf("\nBaud rate changed to default value\n");
   }
}
//------------------------------------------------------------------------------
void test_signal(void)
{
	printf(" -------------------------------------------------------------------\n");
	printf("                           Test Signal                              \n");
	printf(" -------------------------------------------------------------------\n");

	UFR_STATUS status;

	status = ReaderUISignal(3,5);

	if(status)
	{
		printf("\nTest signal failed\n");
		printf("Error code = 0x%08X\n", status);
   }
   else
   {
	   printf("\nSignal OK\n");
   }
}
//------------------------------------------------------------------------------
