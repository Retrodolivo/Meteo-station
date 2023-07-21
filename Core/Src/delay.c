#include "delay.h"


static volatile uint32_t g_current_tick = 0;

void SysTick_Handler(void)
{
  g_current_tick++;
}

uint32_t get_tick(void)
{
	return g_current_tick;
}

void delay_ms_init(uint32_t sys_core_clock)
{
	SysTick_Config(sys_core_clock/ 1000);
}

void delay_ms(uint32_t ms)
{
	uint32_t first_tick  = g_current_tick;
	while ((g_current_tick - first_tick) < ms);
}
