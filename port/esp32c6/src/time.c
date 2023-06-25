
#include <time.h>



#define LP_WDT_CONFIG0_REG			(LP_WDT_BASE + 0x0000)
#define LP_WDT_CONFIG1_REG			(LP_WDT_BASE + 0x0004)
#define LP_WDT_CONFIG2_REG			(LP_WDT_BASE + 0x0008)
#define LP_WDT_CONFIG3_REG			(LP_WDT_BASE + 0x000c)
#define LP_WDT_CONFIG4_REG			(LP_WDT_BASE + 0x0010)
#define LP_WDT_FEED_REG				(LP_WDT_BASE + 0x0014)
#define LP_WDT_WRPROTECT_REG		(LP_WDT_BASE + 0x0018)
#define LP_WDT_SWD_CONFIG_REG		(LP_WDT_BASE + 0x001c)
#define LP_WDT_SWD_WRPROTECT_REG	(LP_WDT_BASE + 0x0020)
#define LP_WDT_INT_RAW_REG			(LP_WDT_BASE + 0x0024)
#define LP_WDT_IN_ST_REG			(LP_WDT_BASE + 0x0028)
#define LP_WDT_INT_ENA_REG			(LP_WDT_BASE + 0x002c)
#define LP_WDT_INT_CLR_REG			(LP_WDT_BASE + 0x0030)
#define LP_WDT_DATE_REG				(LP_WDT_BASE + 0x03fc)

// SPDX-License-Identifier: MIT

#define LD_WDT_WRPROTECT_MAGIC 0x50D83AA1

// Timer 0 configuration register (Access: varies)
#define TIMG0_T0CONFIG_REG			(TIMG0_BASE + 0x0000)
// Timer 0 current value, low 32 bits (Access: RO)
#define TIMG0_T0LO_REG				(TIMG0_BASE + 0x0004)
// Timer 0 current value, high 22 bits (Access: RO)
#define TIMG0_T0HI_REG				(TIMG0_BASE + 0x0008)
// Write to copy current timer value to TIMG0_T0LO_REG or TIMG0_T0HI_REG (Access: R/W/SC)
#define TIMG0_T0UPDATE_REG			(TIMG0_BASE + 0x000C)
// Timer 0 alarm value, low 32 bits (Access: R/W)
#define TIMG0_T0ALARMLO_REG			(TIMG0_BASE + 0x0010)
// Timer 0 alarm value, high bits (Access: R/W)
#define TIMG0_T0ALARMHI_REG			(TIMG0_BASE + 0x0014)
// Timer 0 reload value, low 32 bits (Access: R/W)
#define TIMG0_T0LOADLO_REG			(TIMG0_BASE + 0x0018)
// Timer 0 reload value, high 22 bits (Access: R/W)
#define TIMG0_T0LOADHI_REG			(TIMG0_BASE + 0x001C)
// Write to reload timer from TIMG0_T0LOADLO_REG or TIMG0_T0LOADHI_REG (Access: WT)
#define TIMG0_T0LOAD_REG			(TIMG0_BASE + 0x0020)
// Watchdog timer configuration register (Access: varies)
#define TIMG0_WDTCONFIG0_REG		(TIMG0_BASE + 0x0048)
// Watchdog timer prescaler register (Access: varies)
#define TIMG0_WDTCONFIG1_REG		(TIMG0_BASE + 0x004C)
// Watchdog timer stage 0 timeout value (Access: R/W)
#define TIMG0_WDTCONFIG2_REG		(TIMG0_BASE + 0x0050)
// Watchdog timer stage 1 timeout value (Access: R/W)
#define TIMG0_WDTCONFIG3_REG		(TIMG0_BASE + 0x0054)
// Watchdog timer stage 2 timeout value (Access: R/W)
#define TIMG0_WDTCONFIG4_REG		(TIMG0_BASE + 0x0058)
// Watchdog timer stage 3 timeout value (Access: R/W)
#define TIMG0_WDTCONFIG5_REG		(TIMG0_BASE + 0x005C)
// Write to feed the watchdog timer (Access: WT)
#define TIMG0_WDTFEED_REG			(TIMG0_BASE + 0x0060)
// Watchdog write protect register (Access: R/W)
#define TIMG0_WDTWPROTECT_REG		(TIMG0_BASE + 0x0064)
// RTC frequency calculation configuration register 0 (Access: varies)
#define TIMG0_RTCCALICFG_REG		(TIMG0_BASE + 0x0068)
// RTC frequency calculation configuration register 1 (Access: RO)
#define TIMG0_RTCCALICFG1_REG		(TIMG0_BASE + 0x006C)
// RTC frequency calculation configuration register 2 (Access: varies)
#define TIMG0_RTCCALICFG2_REG		(TIMG0_BASE + 0x0080)
// Interrupt enable bits (Access: R/W)
#define TIMG0_INT_ENA_TIMERS_REG	(TIMG0_BASE + 0x0070)
// Raw interrupt status (Access: R/SS/WTC)
#define TIMG0_INT_RAW_TIMERS_REG	(TIMG0_BASE + 0x0074)
// Masked interrupt status (Access: RO)
#define TIMG0_INT_ST_TIMERS_REG		(TIMG0_BASE + 0x0078)
// Interrupt clear bits (Access: WT)
#define TIMG0_INT_CLR_TIMERS_REG	(TIMG0_BASE + 0x007C)
// Timer version control register (Access: R/W)
#define TIMG0_NTIMERS_DATE_REG		(TIMG0_BASE + 0x00F8)
// Timer group clock gate register (Access: R/W)
#define TIMG0_REGCLK_REG			(TIMG0_BASE + 0x00FC)



// Initialise timer and watchdog subsystem.
void time_init() {
	// Disable LP WDT.
	WRITE_REG(LP_WDT_WRPROTECT_REG, LD_WDT_WRPROTECT_MAGIC);
	WRITE_REG(LP_WDT_CONFIG0_REG, 0);
	
	// Set up TIMG0 T0 as a 1MHz counter.
	WRITE_REG(TIMG0_T0CONFIG_REG, 0);
	WRITE_REG(TIMG0_T0LOADLO_REG, 0);
	WRITE_REG(TIMG0_T0LOADHI_REG, 0);
	WRITE_REG(TIMG0_T0LOAD_REG, -1);
	WRITE_REG(TIMG0_T0CONFIG_REG, 0xc0050000);
}

// Get current time in microseconds.
int64_t time_us() {
	uint32_t lo = READ_REG(TIMG0_T0LO_REG);
	int div = 32;
	WRITE_REG(TIMG0_T0UPDATE_REG, -1);
	while (READ_REG(TIMG0_T0LO_REG) == lo && --div);
	return READ_REG(TIMG0_T0LO_REG) | ((uint64_t) READ_REG(TIMG0_T0HI_REG) << 32LLU);
}
