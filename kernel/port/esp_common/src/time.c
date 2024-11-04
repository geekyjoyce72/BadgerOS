
// SPDX-License-Identifier: MIT

#include "time.h"

#include "assertions.h"
#include "interrupt.h"
#include "log.h"
#include "port/hardware.h"
#include "port/hardware_allocation.h"
#include "scheduler/isr.h"
#include "smp.h"

// NOLINTBEGIN
#define __DECLARE_RCC_RC_ATOMIC_ENV 0
#define __DECLARE_RCC_ATOMIC_ENV    0
// NOLINTEND

#include <config.h>
#include <soc/lp_wdt_struct.h>
#include <soc/timer_group_struct.h>

#ifdef CONFIG_TARGET_esp32c6
#include <soc/pcr_struct.h>
#endif


#define GET_TIMER_INFO(timerno)                                                                                        \
    assert_dev_drop((timerno) >= 0 && (timerno) < ESP_TIMG_COUNT * ESP_TIMG_TIMER_COUNT);                              \
    timg_dev_t *timg  = (timerno) / ESP_TIMG_TIMER_COUNT ? &TIMERG1 : &TIMERG0;                                        \
    int         timer = (timerno) % ESP_TIMG_TIMER_COUNT;



// Callback to the timer driver for when a timer alarm fires.
void timer_isr_timer_alarm() {
    int cpu = smp_cur_cpu();
    timer_alarm_disable(cpu);
    timer_int_clear(cpu);
    sched_request_switch_from_isr();
}

// Initialise timer and watchdog subsystem.
void time_init() {
#ifdef CONFIG_TARGET_esp32c6
    // Power up timers.
    PCR.timergroup0_conf.tg0_rst_en                  = false;
    PCR.timergroup0_conf.tg0_clk_en                  = true;
    PCR.timergroup0_timer_clk_conf.tg0_timer_clk_sel = 0;
    PCR.timergroup0_timer_clk_conf.tg0_timer_clk_en  = true;
    PCR.timergroup0_wdt_clk_conf.tg0_wdt_clk_sel     = 0;
    PCR.timergroup0_wdt_clk_conf.tg0_wdt_clk_en      = true;
#endif
    TIMERG0.regclk.clk_en = true;
    TIMERG1.regclk.clk_en = true;

    // Turn off watchdogs.
    LP_WDT.wprotect.val        = 0x50D83AA1;
    LP_WDT.config0.val         = 0;
    TIMERG0.wdtwprotect.val    = 0x50D83AA1;
    TIMERG0.wdtconfig0.val     = 0;
    TIMERG1.wdtwprotect.val    = 0x50D83AA1;
    TIMERG1.wdtconfig0.val     = 0;
    TIMERG0.int_ena_timers.val = 0;
    TIMERG1.int_ena_timers.val = 0;

    // Configure system timers.
    timer_stop(0);
    timer_value_set(0, 0);
    timer_alarm_disable(0);
    timer_int_clear(0);
    timer_int_enable(0, true);
    timer_set_freq(0, 1000000);
#ifdef CONFIG_TARGET_esp32p4
    timer_stop(1);
    timer_value_set(1, 0);
    timer_alarm_disable(1);
    timer_int_clear(1);
    timer_int_enable(1, true);
    timer_set_freq(1, 1000000);
#endif

    // Configure timer interrupts.
#ifdef CONFIG_TARGET_esp32c6
    set_cpu0_timer_irq(ETS_TG0_T0_LEVEL_INTR_SOURCE);
#endif
#ifdef CONFIG_TARGET_esp32p4
    set_cpu0_timer_irq(ETS_TG0_T0_INTR_SOURCE);
    set_cpu1_timer_irq(ETS_TG1_T0_INTR_SOURCE);
#endif

    // Start timers at close to the same time.
    timer_start(0);
#ifdef CONFIG_TARGET_esp32p4
    timer_start(1);
#endif
}

// Sets the alarm time when the next task switch should occur.
void time_set_next_task_switch(timestamp_us_t timestamp) {
    int cpu = smp_cur_cpu();
    timer_alarm_config(cpu, timestamp, false);
}

// Get current time in microseconds.
timestamp_us_t time_us() {
    return timer_value_get(smp_cur_cpu());
}



// Set timer frequency.
void timer_set_freq(int timerno, frequency_hz_t freq) {
    GET_TIMER_INFO(timerno)
    frequency_hz_t base_freq;
#ifdef CONFIG_TARGET_esp32p4
    // TODO: Determine what selects timer clock source.
    base_freq = 40000000;
#endif
#ifdef CONFIG_TARGET_esp32c6
    uint32_t clksrc;
    if (timerno) {
        clksrc = PCR.timergroup1_timer_clk_conf.tg1_timer_clk_sel;
    } else {
        clksrc = PCR.timergroup0_timer_clk_conf.tg0_timer_clk_sel;
    }
    switch (clksrc) {
        case 0: base_freq = ESP_FREQ_XTAL_CLK; break;
        case 1: base_freq = 80000000; break;
        case 2: base_freq = ESP_FREQ_RC_FAST_CLK; break;
        default: __builtin_unreachable();
    }
#endif

    uint32_t divider = base_freq / freq;
    if (divider < 1) {
        logkf(LOG_WARN, "Timer clock divider unreachable: %{u32;d}", divider);
        divider = 1;
    } else if (divider > 32767) {
        logkf(LOG_WARN, "Timer clock divider unreachable: %{u32;d}", divider);
        divider = 32767;
    }
    timg->hw_timer[timer].config.tx_divider = divider;
}

// Start timer.
void timer_start(int timerno) {
    GET_TIMER_INFO(timerno)
    timg->hw_timer[timer].config.tx_divcnt_rst = false;
    timg->hw_timer[timer].config.tx_increase   = true;
    timg->hw_timer[timer].config.tx_en         = true;
}

// Stop timer.
void timer_stop(int timerno) {
    GET_TIMER_INFO(timerno)
    timg->hw_timer[timer].config.tx_en = false;
}

// Configure timer alarm.
void timer_alarm_config(int timerno, int64_t threshold, bool reset_on_alarm) {
    GET_TIMER_INFO(timerno)
    timg->hw_timer[timer].alarmlo.val = threshold;
    timg->hw_timer[timer].alarmhi.val = threshold >> 32;
    timg_txconfig_reg_t tmp           = timg->hw_timer[timer].config;
    tmp.tx_autoreload                 = reset_on_alarm;
    tmp.tx_alarm_en                   = true;
    timg->hw_timer[timer].config      = tmp;
}

// Disable timer alarm.
void timer_alarm_disable(int timerno) {
    GET_TIMER_INFO(timerno)
    timg->hw_timer[timer].config.tx_alarm_en = false;
}

// Get timer value.
int64_t timer_value_get(int timerno) {
    GET_TIMER_INFO(timerno)
    uint32_t lo                = timg->hw_timer[timer].lo.val;
    timg->hw_timer->update.val = true;
    for (int div = 32; lo == timg->hw_timer[timer].lo.val && div; div--) continue;
    return ((int64_t)timg->hw_timer[timer].hi.val << 32) | timg->hw_timer[timer].lo.val;
}

// Set timer value.
void timer_value_set(int timerno, int64_t time) {
    GET_TIMER_INFO(timerno)
    timg->hw_timer[timer].loadlo.val = time;
    timg->hw_timer[timer].loadhi.val = time >> 32;
    timg->hw_timer[timer].load.val   = true;
}



// Check whether timer has interrupts enabled.
bool timer_int_enabled(int timerno) {
    GET_TIMER_INFO(timerno)
    return (timg->int_ena_timers.val >> timer) & 1;
}

// Enable / disable timer interrupts.
void timer_int_enable(int timerno, bool enable) {
    GET_TIMER_INFO(timerno)
    if (enable) {
        timg->int_ena_timers.val |= 1 << timer;
    } else {
        timg->int_ena_timers.val &= ~(1 << timer);
    }
}

// Check whether timer interrupt had fired.
bool timer_int_pending(int timerno) {
    GET_TIMER_INFO(timerno)
    return (timg->int_raw_timers.val >> timer) & 1;
}

// Clear timer interrupt.
void timer_int_clear(int timerno) {
    GET_TIMER_INFO(timerno)
    timg->int_clr_timers.val = 1 << timer;
}
