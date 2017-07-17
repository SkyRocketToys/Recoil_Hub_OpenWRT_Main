// ----------------------------------------------------------------------------
//  Copyright (c) 2017, Hotgen Ltd (www.hotgen.com)
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// These values are used by the power LED
enum {
	PWR_CMD_NULL = 0,
	PWR_CMD_GLOW,
	PWR_CMD_SOLID,
	PWR_CMD_FLASH1,
	PWR_CMD_FLASH2,
	PWR_CMD_FLASH3,
	// For internal use
	PWR_CMD_POWEROFF,
	PWR_CMD_ACK,
};

bool POWER_Start(void); // Start the power management thread
bool POWER_Stop(void); // Turns the power off
bool POWER_SetStatus(uint8_t pwr_cmd); // Sends a command to the tritan LED
bool POWER_SendShutdownBeginToTritan(void);
bool POWER_SendShutdownCompleteToTritan(void);
