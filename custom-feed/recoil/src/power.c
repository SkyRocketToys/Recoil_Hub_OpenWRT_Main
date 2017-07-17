// ----------------------------------------------------------------------------
//  Copyright (c) 2017, Hotgen Ltd (www.hotgen.com)
//  filename    :- power.c
//  description :- Recoil Power Manager
//  author      :- Rajesh Gunasekaran   (rajg@hotgen.com)
//  modified by Carl Muller (carlm@hotgen.com) to support new protocol
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Includes
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <pthread.h>
#include "recoilnetwork.h"
#include "power.h"

// ----------------------------------------------------------------------------
// Hardware-specific macros for the Atheros AR9331 chip
#define GPIO_ADDR   0x18040000  // base address for GPIO (General Purpose Input/Output)
typedef struct GPIO_s {
	uint32_t GPIO_OE;    // 0x18040000: Direction configuation (output enable) (1=output, 0=input)
	uint32_t GPIO_IN;    // 0x18040004: Input value
	uint32_t GPIO_OUT;   // 0x18040008: Output value
	uint32_t GPIO_SET;   // 0x1804000C: Bit set
	uint32_t GPIO_CLEAR; // 0x18040010: Bit Clear
	uint32_t GPIO_INT;   // 0x18040014: Interrupt enable
	uint32_t GPIO_INT_TYPE;          // 0x18040018: Interrupt Type
	uint32_t GPIO_INT_POLARITY;      // 0x1804001C: Interrupt Polarity
	uint32_t GPIO_INT_PENDING;       // 0x18040020: Interrupt Pending
	uint32_t GPIO_INT_MASK;          // 0x18040024: Interrupt Mask
	uint32_t GPIO_FUNCTION_1;        // 0x18040028: GPIO Function
	uint32_t GPIO_IN_ETH_SWITCH_LED; // 0x1804002C: General Purpose I/O Input Value
	// Secondary functions are outside the mapped in registers
//	uint32_t GPIO_FUNCTION_2:        // 0x18040030: Extended GPIO Function Control	
} GPIO_t;
#define GPIO_BLOCK  48          // memory block size

// Bit masks for GPIO_FUNCTION_1
enum {
	GPIOF1_SPDIF2TCK           = 0x80000000, // Enables SPDIF_OUT on the pin TCK
	GPIOF1_SPDIF_EN            = 0x40000000, // Enables GPIO_23 or TCK as the SPDIF serial output
	GPIOF1_I2SO_22_18_EN       = 0x20000000, // Enables GPIO bits [22:18] as I2S interface pins
		// Bit [18] BITCLK (Input/Output)
		// Bit [19] WS (Input/Output)
		// Bit [20] SD (Output)
		// Bit [21] MCK (Input/Output)
		// Bit [22] MICIN (Input)
	GPIOF1_I2S_MCKEN           = 0x08000000, // Enables the master audio CLK_MCK to be output through GPIO_21.
		// Works only if I2S0_22_18_EN (bit [29]) is also set.
	GPIOF1_I2SO_EN             = 0x04000000, // Enables I2S functions on GPIO pins.
	GPIOF1_ETH_SWITCH_LED_DUPL = 0x02000000, // Link signal to select whether Link, Activity or both must be indicated in the LED
	GPIOF1_ETH_SWITCH_LED_COLL = 0x01000000, // Link signal to select whether Link, Activity or both must be indicated in the LED
	GPIOF1_ETH_SWITCH_LED_ACTV = 0x00800000, // Link signal to select whether Link, Activity or both must be indicated in the LED
	GPIOF1_SPI_EN              = 0x00040000, // Enables SPI SPA Interface signals in GPIO_2, GPIO_3, GPIO_4 and GPIO_5
	GPIOF1_RES1                = 0x00008000, // Reserved. This pin must be written with 1.
	GPIOF1_SPI_CS_EN2          = 0x00004000, // Enables an additional SPI chip select on GPIO_10
	GPIOF1_SPI_CS_EN1          = 0x00002000, // Enables an additional SPI chip select on GPIO_9
	GPIOF1_ETH_SWITCH_LED4_EN  = 0x00000080, // Enables the Ethernet Switch LED data on GPIO_17
	GPIOF1_ETH_SWITCH_LED3_EN  = 0x00000040, // Enables the Ethernet Switch LED data on GPIO_16
	GPIOF1_ETH_SWITCH_LED2_EN  = 0x00000020, // Enables the Ethernet Switch LED data on GPIO_15
	GPIOF1_ETH_SWITCH_LED1_EN  = 0x00000010, // Enables the Ethernet Switch LED data on GPIO_14
	GPIOF1_ETH_SWITCH_LED0_EN  = 0x00000008, // Enables the Ethernet Switch LED data on GPIO_13
	GPIOF1_UART_RTS_CTS_EN     = 0x00000004, // Enables UART RTS/CTS I/O on GPIO_11 (RTS) and GPIO_12 (CTS)
	GPIOF1_UART_EN             = 0x00000002, // Enables UART I/O on GPIO_9 (SIN) and GPIO_10 (SOUT)
	GPIOF1_EJTAG_DISABLE       = 0x00000001, // Disables EJTAG port functionality to enable GPIO functionality (GPIO_18,GPIO_19)
};

// How is the Atheros chip wired to the Tritan chip?
#define GPIO_EXTERNAL_CHIP_OUTPUT_PIN_NO    19      // CPU to Tritan
#define GPIO_EXTERNAL_CHIP_INPUT_PIN_NO     18      // Tritan to CPU

#define HIGH    true
#define LOW     false

// ----------------------------------------------------------------------------
// High-level macros for the tritan protocol
#define TIME_STEP 50 // Time step in milliseconds
#define TIME_STEP_FAIL 12 // If the time step is out by more than this we have failed this packet
#define POWER_THREAD_SLEEP_FAST 3 // Time step in milliseconds when we are transmitting
#define POWER_THREAD_SLEEP_SLOW 10 // Time step in milliseconds when we are not transmitting


// ----------------------------------------------------------------------------
// Commands for the Tritan chip
#define CMD_NULL        0x001EA
#define CMD_REBOOTING   0x003D6
#define CMD_BOOTED      0x003D2
#define CMD_ACK_POWER   0x007A6
#define CMD_ERROR_1     0x003CA
#define CMD_ERROR_2     0x00792
#define CMD_ERROR_3     0x00F26
#define CMD_POWER_OFF   0x00F36

// Pulses from the Tritan chip
#define TIME_PULSE_PW_MIN	(100-20) // 100ms pulse means reset password
#define TIME_PULSE_PW_MAX	(100+20)
#define TIME_PULSE_OFF_MIN	(200) // Longer than 200ms means power off

// ------------------------------------ User data types -----------------------

// ----------------------------------------------------------------------------
// Module variables
static UTILS_Thread_t gHelperThread; // thread id for the Helper thread - which is responsible for incoming request for discovery and manages them
volatile GPIO_t *gGPIO = NULL;
static uint32_t txData; // The current packet
static uint32_t txDataPkt; // The whole packet
static uint32_t txDataLast; // The packet when we last sent it
static uint32_t txTime;

static volatile uint32_t cmdRequest; // Request from external thread to helper thread

// ----------------------------------------------------------------------------
// Prototypes
static bool power_GPIO_Setup(void);
static bool power_GPIO_SetDirection(int gpio, bool output);
static bool power_GPIO_Write(int gpio, bool high);
static bool power_GPIO_Read(int gpio);
static void* power_HelperThread (void *pArg);


// ----------------------------------------------------------------------------
// Events called from the power thread to the main app
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Low level GPIO functions
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Setup the GPIO pins used for the power management
// Instead of using the (generic) user mode filesystem ("/sys/class/gpio/gpio13/direction")
// We memory map the (very hardware specific) GPIO registers into a file
static bool power_GPIO_Setup(void)
{
	int m_mfd;
	bool retval = false;

	DEBUG_INFO("Tritan GPIO setup");
	if (SYSTEM_CALL_ERROR == (m_mfd = open("/dev/mem", O_RDWR)))
	{
		DEBUG_ERROR("GPIO open failed(%d, %s)", errno, strerror(errno));
	}
	else if (MAP_FAILED == (gGPIO = (GPIO_t*)mmap(NULL, GPIO_BLOCK, PROT_READ|PROT_WRITE, MAP_SHARED, m_mfd, GPIO_ADDR)))
	{
		DEBUG_ERROR("GPIO mmap failed(%d, %s)", errno, strerror(errno));
	}
	else
	{
		DEBUG_INFO("GPIO Setup success!");
		retval = true;
	}

	if (m_mfd != SYSTEM_CALL_ERROR)
	{
		close(m_mfd);
	}
	return retval;
}

// ----------------------------------------------------------------------------
// Set the direction of a specified GPIO port (on this specific hardware)
static bool power_GPIO_SetDirection(int gpio, bool output)
{
	bool retval = false;
	DEBUG_INFO("Tritan GPIO %d set direction %d", gpio, (int) output);
	if (gGPIO)
	{
		if (gpio == 18 || gpio == 19) // Disable special pins
		{
			DEBUG_INFO("Tritan GPIO disable JTAG");
			gGPIO->GPIO_FUNCTION_1 |= GPIOF1_EJTAG_DISABLE;
			gGPIO->GPIO_CLEAR |= (1u << 18) | (1u << 19);
		}
		if (output)
		{
			gGPIO->GPIO_OE |= (1u << gpio); // set bit to 1
		}
		else
		{
			gGPIO->GPIO_OE &= ~(1u << gpio); // clear bit
//			gGPIO->GPIO_INT_POLARITY |= (1u << gpio); // Try active low
		}
		retval = true;
	}
	return retval;
}

// ----------------------------------------------------------------------------
// Set the pin value of a specified (output) GPIO pin (on this specific hardware)
static bool power_GPIO_Write(int gpio, bool high)
{
	bool retval = false;
	DEBUG_INFO("Tritan GPIO %d write %d", gpio, (int) high);
	if (gGPIO)
	{    
		if (high)
		{
			gGPIO->GPIO_SET = (1u << gpio);
		}
		else
		{
			gGPIO->GPIO_CLEAR = (1u << gpio);
		}
		retval = true;
	}
	return retval;          
}

// ----------------------------------------------------------------------------
// Read the pin value of a specified (input) GPIO pin (on this specific hardware)
static bool power_GPIO_Read(int gpio)
{
	uint32_t value = 0;
	if (gGPIO)
	{
		value = gGPIO->GPIO_IN;
	}
	return ((value & (1 << gpio)) ? true : false);
}

// ----------------------------------------------------------------------------
// Return the current time in milliseconds
static uint32_t power_Time(void)
{
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	uint32_t v = tv.tv_sec * 1000;
	v += tv.tv_nsec / 1000000;
	return v;
}

// ----------------------------------------------------------------------------
// Protocol functions
// ----------------------------------------------------------------------------


// Start sending a command to the tritan chip
static void power_SetCommand(uint8_t cmd)
{
	switch (cmd) {
	case PWR_CMD_NULL:
		txDataPkt = CMD_NULL;
		break;
	case PWR_CMD_GLOW:
		txDataPkt = CMD_REBOOTING;
		break;
	case PWR_CMD_SOLID:
		txDataPkt = CMD_BOOTED;
		break;
	case PWR_CMD_ACK:
		txDataPkt = CMD_ACK_POWER;
		break;
	case PWR_CMD_FLASH1:
		txDataPkt = CMD_ERROR_1;
		break;
	case PWR_CMD_FLASH2:
		txDataPkt = CMD_ERROR_2;
		break;
	case PWR_CMD_FLASH3:
		txDataPkt = CMD_ERROR_3;
	    break;
   case PWR_CMD_POWEROFF:
		txDataPkt = CMD_POWER_OFF;
		break;
	};
	txData = txDataPkt;
	txDataLast = txData;
	txTime = power_Time()+POWER_THREAD_SLEEP_SLOW+TIME_STEP;
}

// ----------------------------------------------------------------------------
// Higher level functions
// ----------------------------------------------------------------------------


// ----------------------------------------------------------------------------
// Helper thread for the power management routine - runs long-term
static void* power_HelperThread (void *pArg)
{
	bool bTerm = false;
	bool bResetPwd = false;
	bool bLastRx = false;
	uint32_t rxTime = 0;
	uint32_t rxHeartTime = 0;
	
	while (gHelperThread.run || txData || txDataLast || cmdRequest) // while the system is allowed to run
	{
		uint32_t now = power_Time();
		if (txData | txDataLast) // We are writing a packet to the Tritan
		{
			if (now > txTime)
			{
				if (now > txTime+TIME_STEP_FAIL)
				{
					// Linux has failed to call us fast enough
					// Retrigger the command
					txTime = now + TIME_STEP;
					txData = txDataPkt << 8; // Give it a chance to idle first
					txDataLast = txData;
				}
				else // Send the packet bit to the GPIO
				{
					txTime += TIME_STEP;
					power_GPIO_Write(GPIO_EXTERNAL_CHIP_OUTPUT_PIN_NO, (txData & 1) != 0);
					txDataLast = txData;
					txData >>= 1;
				}
			}
		}
		else // Not sending a packet right now
		{
			uint32_t cmd = cmdRequest; // Cache it on this thread
			if (cmd != 0)
			{
				cmdRequest = 0;
				power_SetCommand(cmd);
			}
		}
		bool bRx = power_GPIO_Read(GPIO_EXTERNAL_CHIP_INPUT_PIN_NO);
		if (now > rxHeartTime)
		{
			rxHeartTime += 1000; // Wait a second between heartbeats
//			DEBUG_INFO("GPIO %d from Tritan = %d", GPIO_EXTERNAL_CHIP_INPUT_PIN_NO, (int) bRx);
		}
		if (bRx && !bLastRx)
		{
			rxTime = now;
		}
		if (!bRx && bLastRx)
		{
			uint32_t delta = now - rxTime;
			if ((delta >= TIME_PULSE_PW_MIN) && (delta <= TIME_PULSE_PW_MAX) && !bResetPwd)
			{
				// We want to start resetting the password
				DEBUG_INFO("System received signal to reset password from Power CPU(Tritan)...");
				RECOILAPP_ResetBaseStationPassword();
				bResetPwd = true;
			}
		}
		if (bRx && (now - rxTime > TIME_PULSE_OFF_MIN))
		{
			// Signal to turn the power off
			if (!bTerm)
			{
				DEBUG_INFO("System received signal to shutdown from Power CPU(Tritan)...");
				RECOIL_WantPowerOff();
				RECOIL_TermSignalHandler(-1);
				bTerm = true;
			}                
		}
		bLastRx = bRx;
		if ((txData | txDataLast) || bRx) // OK, rush now
		{
			usleep (POWER_THREAD_SLEEP_FAST*1000); // sleep until the next poll or command
		}
		else
		{
			usleep (POWER_THREAD_SLEEP_SLOW*1000); // sleep until the next poll or command
		}
	}

	DEBUG_MIL("Power [helper thread] shutdown complete...");

	DEBUG_OUT;
	pthread_exit(NULL);    
}

// ----------------------------------------------------------------------------
// Entry point for the power manager sub system
// This returns after initialisation, running its code in a helper thread
bool POWER_Start(void)
{
	bool retval = false;
	int status;

	DEBUG_IN;

	// signal to run immedietely
	DEBUG_INFO("Starting Tritan power management");
	gHelperThread.run = true;

	if (!power_GPIO_Setup())   // initialise GPIO module
	{
		DEBUG_FATAL("Power GPIO initialisation failed");
	}
	else if (!power_GPIO_SetDirection(GPIO_EXTERNAL_CHIP_INPUT_PIN_NO, false))  // configure Tritan to CPU pin as INPUT
	{
		DEBUG_FATAL("Power GPIO initialisation failed (cannot set input Tritan pin to input)");
	}
	else if (!power_GPIO_SetDirection(GPIO_EXTERNAL_CHIP_OUTPUT_PIN_NO, true))  // configure CPU to Tritan pin as OUTPUT
	{
		DEBUG_FATAL("Power GPIO initialisation failed (cannot set output Tritan pin to output)");
	}
	// create the helper thread    
	else if (SYSTEM_CALL_ERROR == (status = UTILS_CreateThread (&gHelperThread.thread, DEFAULT_THREAD_STACK_SIZE, power_HelperThread, (void*)NULL)))
	{
		DEBUG_FATAL("Power helper thread create failed (%d, %s)", errno, strerror(errno));
	}
	else
	{
		DEBUG_MIL("Power Manager started successfully...");
		retval = true;
	}

	DEBUG_OUT;
	return (retval);
}

// ----------------------------------------------------------------------------
// CPU initiates the shutdown sequence and wait for the acknowledgment
//  CPU      x---------------y
//  ---------                -----------
//  Tritan     a--------------b
//  -----------               -----
//
// CPU sends the shutdown start signal at 'x' and Tritan sense the shutdown signal at 'a' and waits until CPU sends a shutdown complete signal
// Tritan sense the shutdown complete at 'b' and cuts the power to the Wifi hub
bool POWER_SendShutdownBeginToTritan(void) // x
{
	DEBUG_IN;
	DEBUG_INFO("Telling tritan start shutdown");
	cmdRequest = PWR_CMD_POWEROFF;
	DEBUG_OUT;
	return true;
}

bool POWER_SendShutdownCompleteToTritan(void) // y
{
	DEBUG_IN;
	DEBUG_INFO("Telling tritan chip shutdown complete");
	cmdRequest = PWR_CMD_ACK;
	DEBUG_OUT;
	return true;
}

// ----------------------------------------------------------------------------
// From the main thread, tell the helper thread to stop and wait until it does so
// (this may take a second if it is transmitting data)
bool POWER_Stop(void)
{
	void *H_retval = NULL;
	int status;
	bool retval = false;

	DEBUG_IN;
	DEBUG_MIL ("Request to shut down [power] arrived...");

	// signal helper thread to terminate
	gHelperThread.run = false;
	if (0 != (status = pthread_join(gHelperThread.thread, &H_retval)))
	{
		DEBUG_ERROR("Unable to wait for helper thread to exit...err(%d, %s)", errno, strerror(errno));
	}
	else
	{
		DEBUG_MIL ("Request to shut down [power] is complete...");
		retval = true;
	}

	DEBUG_OUT;
	return (retval);
}

// ----------------------------------------------------------------------------
// Sends a command to the tritan LED
bool POWER_SetStatus(uint8_t pwr_cmd)
{
	DEBUG_IN;
	DEBUG_INFO("Telling tritan chip status %d", pwr_cmd);
	cmdRequest = pwr_cmd;
	DEBUG_OUT;
	return true;
}
