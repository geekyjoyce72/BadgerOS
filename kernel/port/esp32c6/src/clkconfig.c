
// SPDX-License-Identifier: MIT

#include "port/clkconfig.h"

#include "attributes.h"
#include "log.h"
#include "port/hardware.h"
#include "soc/pcr_struct.h"
#include "soc/spi_reg.h"
#include "soc/spi_struct.h"

// UART0 configuration register (Access: R/W)
#define PCR_UART0_CONF_REG                 (PCR_BASE + 0x0000)
// UART0_SCLK configuration register (Access: R/W)
#define PCR_UART0_SCLK_CONF_REG            (PCR_BASE + 0x0004)
// UART0 power control register (Access: R/W)
#define PCR_UART0_PD_CTRL_REG              (PCR_BASE + 0x0008)
// UART1 configuration register (Access: R/W)
#define PCR_UART1_CONF_REG                 (PCR_BASE + 0x000C)
// UART1_SCLK configuration register (Access: R/W)
#define PCR_UART1_SCLK_CONF_REG            (PCR_BASE + 0x0010)
// UART1 power control register (Access: R/W)
#define PCR_UART1_PD_CTRL_REG              (PCR_BASE + 0x0014)
// I2C configuration register (Access: R/W)
#define PCR_I2C_CONF_REG                   (PCR_BASE + 0x0020)
// I2C_SCLK configuration register (Access: R/W)
#define PCR_I2C_SCLK_CONF_REG              (PCR_BASE + 0x0024)
// UHCI configuration register (Access: R/W)
#define PCR_UHCI_CONF_REG                  (PCR_BASE + 0x0028)
// RMT configuration register (Access: R/W)
#define PCR_RMT_CONF_REG                   (PCR_BASE + 0x002C)
// RMT_SCLK configuration register (Access: R/W)
#define PCR_RMT_SCLK_CONF_REG              (PCR_BASE + 0x0030)
// LEDC configuration register (Access: R/W)
#define PCR_LEDC_CONF_REG                  (PCR_BASE + 0x0034)
// LEDC_SCLK configuration register (Access: R/W)
#define PCR_LEDC_SCLK_CONF_REG             (PCR_BASE + 0x0038)
// TIMERGROUP0 configuration register (Access: R/W)
#define PCR_TIMERGROUP0_CONF_REG           (PCR_BASE + 0x003C)
// TIMERGROUP0_TIMER_CLK configuration register (Access: R/W)
#define PCR_TIMERGROUP0_TIMER_CLK_CONF_REG (PCR_BASE + 0x0040)
// TIMERGROUP0_WDT_CLK configuration register (Access: R/W)
#define PCR_TIMERGROUP0_WDT_CLK_CONF_REG   (PCR_BASE + 0x0044)
// TIMERGROUP1 configuration register (Access: R/W)
#define PCR_TIMERGROUP1_CONF_REG           (PCR_BASE + 0x0048)
// TIMERGROUP1_TIMER_CLK configuration register (Access: R/W)
#define PCR_TIMERGROUP1_TIMER_CLK_CONF_REG (PCR_BASE + 0x004C)
// TIMERGROUP1_WDT_CLK configuration register (Access: R/W)
#define PCR_TIMERGROUP1_WDT_CLK_CONF_REG   (PCR_BASE + 0x0050)
// SYSTIMER configuration register (Access: R/W)
#define PCR_SYSTIMER_CONF_REG              (PCR_BASE + 0x0054)
// SYSTIMER_FUNC_CLK configuration register (Access: R/W)
#define PCR_SYSTIMER_FUNC_CLK_CONF_REG     (PCR_BASE + 0x0058)
// TWAI0 configuration register (Access: R/W)
#define PCR_TWAI0_CONF_REG                 (PCR_BASE + 0x005C)
// TWAI0_FUNC_CLK configuration register (Access: R/W)
#define PCR_TWAI0_FUNC_CLK_CONF_REG        (PCR_BASE + 0x0060)
// TWAI1 configuration register (Access: R/W)
#define PCR_TWAI1_CONF_REG                 (PCR_BASE + 0x0064)
// TWAI1_FUNC_CLK configuration register (Access: R/W)
#define PCR_TWAI1_FUNC_CLK_CONF_REG        (PCR_BASE + 0x0068)
// I2S configuration register (Access: R/W)
#define PCR_I2S_CONF_REG                   (PCR_BASE + 0x006C)
// I2S_TX_CLKM configuration register (Access: R/W)
#define PCR_I2S_TX_CLKM_CONF_REG           (PCR_BASE + 0x0070)
// I2S_TX_CLKM_DIV configuration register (Access: R/W)
#define PCR_I2S_TX_CLKM_DIV_CONF_REG       (PCR_BASE + 0x0074)
// I2S_RX_CLKM configuration register (Access: R/W)
#define PCR_I2S_RX_CLKM_CONF_REG           (PCR_BASE + 0x0078)
// I2S_RX_CLKM_DIV configuration register (Access: R/W)
#define PCR_I2S_RX_CLKM_DIV_CONF_REG       (PCR_BASE + 0x007C)
// SARADC configuration register (Access: R/W)
#define PCR_SARADC_CONF_REG                (PCR_BASE + 0x0080)
// SARADC_CLKM configuration register (Access: R/W)
#define PCR_SARADC_CLKM_CONF_REG           (PCR_BASE + 0x0084)
// TSENS_CLK configuration register (Access: R/W)
#define PCR_TSENS_CLK_CONF_REG             (PCR_BASE + 0x0088)
// USB_SERIAL_JTAG configuration register (Access: R/W)
#define PCR_USB_SERIAL_JTAG_CONF_REG       (PCR_BASE + 0x008C)
// INTMTX configuration register (Access: R/W)
#define PCR_INTMTX_CONF_REG                (PCR_BASE + 0x0090)
// PCNT configuration register (Access: R/W)
#define PCR_PCNT_CONF_REG                  (PCR_BASE + 0x0094)
// ETM configuration register (Access: R/W)
#define PCR_ETM_CONF_REG                   (PCR_BASE + 0x0098)
// PWM configuration register (Access: R/W)
#define PCR_PWM_CONF_REG                   (PCR_BASE + 0x009C)
// PWM_CLK configuration register (Access: R/W)
#define PCR_PWM_CLK_CONF_REG               (PCR_BASE + 0x00A0)
// PARL_IO configuration register (Access: R/W)
#define PCR_PARL_IO_CONF_REG               (PCR_BASE + 0x00A4)
// PARL_CLK_RX configuration register (Access: R/W)
#define PCR_PARL_CLK_RX_CONF_REG           (PCR_BASE + 0x00A8)
// PARL_CLK_TX configuration register (Access: R/W)
#define PCR_PARL_CLK_TX_CONF_REG           (PCR_BASE + 0x00AC)
// SDIO_SLAVE configuration register (Access: R/W)
#define PCR_SDIO_SLAVE_CONF_REG            (PCR_BASE + 0x00B0)
// GDMA configuration register (Access: R/W)
#define PCR_GDMA_CONF_REG                  (PCR_BASE + 0x00BC)
// SPI2 configuration register (Access: R/W)
#define PCR_SPI2_CONF_REG                  (PCR_BASE + 0x00C0)
// SPI2_CLKM configuration register (Access: R/W)
#define PCR_SPI2_CLKM_CONF_REG             (PCR_BASE + 0x00C4)
// AES configuration register (Access: R/W)
#define PCR_AES_CONF_REG                   (PCR_BASE + 0x00C8)
// SHA configuration register (Access: R/W)
#define PCR_SHA_CONF_REG                   (PCR_BASE + 0x00CC)
// RSA configuration register (Access: R/W)
#define PCR_RSA_CONF_REG                   (PCR_BASE + 0x00D0)
// RSA power control register (Access: R/W)
#define PCR_RSA_PD_CTRL_REG                (PCR_BASE + 0x00D4)
// ECC configuration register (Access: R/W)
#define PCR_ECC_CONF_REG                   (PCR_BASE + 0x00D8)
// ECC power control register (Access: R/W)
#define PCR_ECC_PD_CTRL_REG                (PCR_BASE + 0x00DC)
// DS configuration register (Access: R/W)
#define PCR_DS_CONF_REG                    (PCR_BASE + 0x00E0)
// HMAC configuration register (Access: R/W)
#define PCR_HMAC_CONF_REG                  (PCR_BASE + 0x00E4)
// IOMUX configuration register (Access: R/W)
#define PCR_IOMUX_CONF_REG                 (PCR_BASE + 0x00E8)
// IOMUX_CLK configuration register (Access: R/W)
#define PCR_IOMUX_CLK_CONF_REG             (PCR_BASE + 0x00EC)
// MEM_MONITOR configuration register (Access: R/W)
#define PCR_MEM_MONITOR_CONF_REG           (PCR_BASE + 0x00F0)
// TRACE configuration register (Access: R/W)
#define PCR_TRACE_CONF_REG                 (PCR_BASE + 0x00FC)
// ASSIST configuration register (Access: R/W)
#define PCR_ASSIST_CONF_REG                (PCR_BASE + 0x0100)
// CACHE configuration register (Access: R/W)
#define PCR_CACHE_CONF_REG                 (PCR_BASE + 0x0104)
// MODEM_APB configuration register (Access: R/W)
#define PCR_MODEM_APB_CONF_REG             (PCR_BASE + 0x0108)
// TIMEOUT configuration register (Access: R/W)
#define PCR_TIMEOUT_CONF_REG               (PCR_BASE + 0x010C)
// SYSCLK configuration register (Access: varies)
#define PCR_SYSCLK_CONF_REG                (PCR_BASE + 0x0110)
// CPU_WAITI configuration register (Access: R/W)
#define PCR_CPU_WAITI_CONF_REG             (PCR_BASE + 0x0114)
// CPU_FREQ configuration register (Access: R/W)
#define PCR_CPU_FREQ_CONF_REG              (PCR_BASE + 0x0118)
// AHB_FREQ configuration register (Access: R/W)
#define PCR_AHB_FREQ_CONF_REG              (PCR_BASE + 0x011C)
// APB_FREQ configuration register (Access: R/W)
#define PCR_APB_FREQ_CONF_REG              (PCR_BASE + 0x0120)
// SPLL DIV clock-gating configuration register (Access: R/W)
#define PCR_PLL_DIV_CLK_EN_REG             (PCR_BASE + 0x0128)
// 32KHz clock configuration register (Access: R/W)
#define PCR_CTRL_32K_CONF_REG              (PCR_BASE + 0x0134)
// HP SRAM/ROM configuration register (Access: R/W)
#define PCR_SRAM_POWER_CONF_REG            (PCR_BASE + 0x0138)
// Reset event bypass backdoor configuration register (Access: R/W)
#define PCR_RESET_EVENT_BYPASS_REG         (PCR_BASE + 0x0FF0)
// SYSCLK frequency query register 0 (Access: HRO)
#define PCR_SYSCLK_FREQ_QUERY_0_REG        (PCR_BASE + 0x0124)
// Version control register (Access: R/W)
#define PCR_DATE_REG                       (PCR_BASE + 0x0FFC)

// Reset bit for PCR_*_CONF_REG.
#define PCR_CONF_RESET_BIT  0x0002
// Enable bit for PCR_*_CONF_REG.
#define PCR_CONF_ENABLE_BIT 0x0001

// Enable bit for PCR_*_SCLK_CONF_REG.
#define PCR_CONF_SCLK_ENABLE_BIT   0x00400000
// Clock source select mask for PCR_*_SCLK_CONF_REG.
#define PCR_CONF_SCLK_SEL_MASK     0x00300000
// Clock source select mask for PCR_*_SCLK_CONF_REG (bit position).
#define PCR_CONF_SCLK_SEL_POS      20
// Integral part of the clock divider for PCR_*_SCLK_CONF_REG.
#define PCR_CONF_SCLK_DIV_INT_MASK 0x000ff000
// Integral part of the clock divider for PCR_*_SCLK_CONF_REG (bit position).
#define PCR_CONF_SCLK_DIV_INT_POS  12
// Denominator of the clock divider for PCR_*_SCLK_CONF_REG.
#define PCR_CONF_SCLK_DIV_DEN_MASK 0x00000fc0
// Denominator of the clock divider for PCR_*_SCLK_CONF_REG (bit position).
#define PCR_CONF_SCLK_DIV_DEN_POS  6
// Numerator of the clock divider for PCR_*_SCLK_CONF_REG.
#define PCR_CONF_SCLK_DIV_NUM_MASK 0x0000003f
// Numerator of the clock divider for PCR_*_SCLK_CONF_REG (bit position).
#define PCR_CONF_SCLK_DIV_NUM_POS  0

// Enable bit for PCR_*_CLKM_CONF_REG.
#define PCR_CONF_CLKM_ENABLE_BIT       0x00400000
// Clock source select mask for PCR_*_CLKM_CONF_REG.
#define PCR_CONF_CLKM_SEL_MASK         0x00300000
// Clock source select mask for PCR_*_CLKM_CONF_REG (bit position).
#define PCR_CONF_CLKM_SEL_POS          20
// Clock source select XTAL_CLK for PCR_*_CLKM_CONF_REG
#define PCR_CONF_CLKM_SEL_XTAL_CLK     0
// Clock source select PLL_F80M_CLK for PCR_*_CLKM_CONF_REG
#define PCR_CONF_CLKM_SEL_PLL_F80M_CLK 1
// Clock source select RC_FAST_CLK for PCR_*_CLKM_CONF_REG
#define PCR_CONF_CLKM_SEL_RC_FAST_CLK  2

// Nominal frequency of PLL_CLK.
#define FREQ_PLL_CLK       480000000
// Nominal frequency of PLL_F160M_CLK.
#define FREQ_PLL_F160M_CLK 160000000
// Nominal frequency of PLL_F80M_CLK.
#define FREQ_PLL_F80M_CLK  80000000
// Nominal frequency of XTAL_CLK.
#define FREQ_XTAL_CLK      40000000
// Nominal frequency of RC_FAST_CLK.
#define FREQ_RC_FAST_CLK   17500000
// Nominal frequency of RC_SLOW_CLK.
#define FREQ_RC_SLOW_CLK   136000
// Nominal frequency of RC32K_CLK.
#define FREQ_RC32K_CLK     32000
// Nominal frequency of OSC_SLOW_CLK.
#define FREQ_OSC_SLOW_CLK  32000

// Compute frequency dividers for a certain target frequency and source
// frequency.
static uint32_t        i2c_uart_clk_compute_div(uint32_t source_hz, uint32_t target_hz) PURE;
static spi_clock_reg_t spi_clk_compute_div(uint32_t source_hz, uint32_t target_hz, uint8_t duty) PURE;

static uint32_t i2c_uart_clk_compute_div(uint32_t source_hz, uint32_t target_hz) {
    // Divider integral part.
    uint32_t integral   = source_hz / target_hz;
    // Divider fractional part.
    uint32_t fractional = (source_hz % target_hz) * 0x01000000LLU / target_hz;

    // Closest found error.
    int32_t      closest_err = INT32_MAX;
    // Closest found denominator.
    uint_fast8_t closest_den = 2;
    // Closest found numerator.
    uint_fast8_t closest_num = 0;

    // Perform a search.
    for (uint_fast8_t num = 2; num < 64; num++) {
        uint_fast8_t den  = ((uint64_t)fractional * num) >> 24;
        uint32_t     frac = (den << 24) / num;
        int32_t      err  = (int32_t)(frac - fractional);
        if (err < 0)
            err = -err;
        if (err < closest_err) {
            closest_err = err;
            closest_den = den;
            closest_num = num;
        }
    }

    // Pack the FOUND VALUES.
    return ((integral - 1) << PCR_CONF_SCLK_DIV_INT_POS) | (closest_num << PCR_CONF_SCLK_DIV_NUM_POS) |
           (closest_den << PCR_CONF_SCLK_DIV_DEN_POS);
}

static spi_clock_reg_t spi_clk_compute_div(uint32_t source_hz, uint32_t target_hz, uint8_t duty) {
    uint32_t const pre_max = SPI_CLKDIV_PRE_V + 1;
    uint32_t const n_max   = SPI_CLKCNT_N_V + 1;

    spi_clock_reg_t spi_clock_reg = {.val = 0};

    uint32_t best_n = 1, best_pre = 1;

    if ((target_hz < source_hz)) {
        if ((source_hz / (pre_max * n_max)) > target_hz) {
            logkf(LOG_DEBUG, "Maximum SPI clock pre-scaling");
            best_pre = pre_max;
            best_n   = n_max;
        } else if (target_hz < source_hz) {
            logkf(LOG_DEBUG, "Finding optimal SPI clock pre-scaling");
            int32_t best_err = (int32_t)target_hz; // assume worst possible

            // Try to get the most counter resolution and lowest pre-divider
            for (uint32_t n = n_max; n >= 2; n--) {
                uint32_t pre = (source_hz / n) / target_hz;

                if (pre < 1)
                    pre = 1;
                // Prefer to err on the side of too low clock frequency to not damage peripherals
                if ((source_hz / n / pre) > target_hz) {
                    pre += 1;
                }
                if (pre > pre_max)
                    continue;

                int32_t err = (int32_t)target_hz - (int32_t)(source_hz / n / pre);
                if (err < best_err) {
                    best_n   = n;
                    best_pre = pre;
                    best_err = err;
                }
            }
        }

        // Map duty from [0..255] to  [1..n-1]
        uint32_t h = 1 + ((duty * ((best_n - 1) - 1)) / 255);

        spi_clock_reg.clkdiv_pre = best_pre - 1;
        spi_clock_reg.clkcnt_n   = best_n - 1;
        spi_clock_reg.clkcnt_l   = best_n - 1;
        spi_clock_reg.clkcnt_h   = h - 1;
    } else {
        logkf(LOG_DEBUG, "No SPI clock pre-scaling");
        spi_clock_reg.clk_equ_sysclk = 1;
    }

    return spi_clock_reg;
}

// Configure I2C0 clock.
void clkconfig_i2c0(uint32_t freq_hz, bool enable, bool reset) {
    // I2C0 is configured on XTAL_CLK.
    WRITE_REG(PCR_I2C_CONF_REG, PCR_CONF_ENABLE_BIT + reset * PCR_CONF_RESET_BIT);
    WRITE_REG(
        PCR_I2C_SCLK_CONF_REG,
        enable * PCR_CONF_SCLK_ENABLE_BIT + i2c_uart_clk_compute_div(FREQ_XTAL_CLK, freq_hz)
    );
    logkf(LOG_DEBUG, "PCR_I2C_CONF_REG:      %{u32;x}", READ_REG(PCR_I2C_CONF_REG));
    logkf(LOG_DEBUG, "PCR_I2C_SCLK_CONF_REG: %{u32;x}", READ_REG(PCR_I2C_SCLK_CONF_REG));
}

void clkconfig_spi2(uint32_t freq_hz, bool enable, bool reset) {
    // SPI2 is configured on PLL_F80M_CLK.
    WRITE_REG(PCR_SPI2_CONF_REG, PCR_CONF_ENABLE_BIT + reset * PCR_CONF_RESET_BIT);
    WRITE_REG(
        PCR_SPI2_CLKM_CONF_REG,
        enable * PCR_CONF_CLKM_ENABLE_BIT + (PCR_CONF_CLKM_SEL_PLL_F80M_CLK << PCR_CONF_CLKM_SEL_POS)
    );
    GPSPI2.clock = spi_clk_compute_div(FREQ_PLL_F80M_CLK, freq_hz, 256 / 2);
    // GPSPI2.clk_gate.mst_clk_active = true;
    // GPSPI2.clk_gate.mst_clk_sel = 1;
    logkf(LOG_DEBUG, "PCR_SPI2_CONF_REG:      %{u32;x}", READ_REG(PCR_SPI2_CONF_REG));
    logkf(LOG_DEBUG, "PCR_SPI2_CLKM_CONF_REG: %{u32;x}", READ_REG(PCR_SPI2_CLKM_CONF_REG));
    logkf(
        LOG_DEBUG,
        "CLKDIV_PRE %{u32;d}, CLKCNT_N: %{u32;d}, CLKCNT_L: %{u32;d}, CLKCNT_H: %{u32;d}",
        GPSPI2.clock.clkdiv_pre,
        GPSPI2.clock.clkcnt_n,
        GPSPI2.clock.clkcnt_l,
        GPSPI2.clock.clkcnt_h
    );
}

// Configure UART0 clock.
void clkconfig_uart0(uint32_t freq_hz, bool enable, bool reset) {
    // UART0 is configured on PLL_F80M_CLK.
    WRITE_REG(PCR_I2C_CONF_REG, PCR_CONF_ENABLE_BIT + reset * PCR_CONF_RESET_BIT);
    PCR.uart0_sclk_conf.uart0_sclk_sel = 1;
    WRITE_REG(
        PCR_UART0_SCLK_CONF_REG,
        (1 << PCR_CONF_SCLK_SEL_POS) + enable * PCR_CONF_SCLK_ENABLE_BIT +
            i2c_uart_clk_compute_div(FREQ_PLL_F80M_CLK, freq_hz)
    );
}
