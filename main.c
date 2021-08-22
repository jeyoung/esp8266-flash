#include <stdio.h>
#include <stdint.h>

#include "mem.h"
#include "osapi.h"
#include "uart.h"
#include "user_interface.h"
#include "spi_flash.h"

#include "main.h"

#define BUTTON_PIN 2
#define LED_PIN 0

static os_timer_t os_timer;

/* This program demonstrates how to write data to flash storage.
 */

static volatile uint16_t bounces = 0xFFFF;
static volatile uint32_t presses = 0;

static void main_on_timer(void *arg)
{
    uint32_t button_state = GPIO_INPUT_GET(BUTTON_PIN);
    uint32_t presses_read;
    uint8_t button_on;

    bounces = (bounces << 1) | button_state;
    button_on = bounces < 0xFF00;
    if (button_on) {
	++presses;
	spi_flash_erase_sector(0x7E);
	spi_flash_write(0x7E000, (uint32 *)&presses, sizeof(uint32_t));

	// This small delay appears necessary for stability
	os_delay_us(1000);

	spi_flash_read(0x7E000, (uint32 *)&presses_read, sizeof(uint32_t));
	os_printf("Number of presses: %d\n", presses_read);
    }
    GPIO_OUTPUT_SET(LED_PIN, button_on);
    os_timer_arm(&os_timer, 25, 0);
}

void ICACHE_FLASH_ATTR user_init(void)
{
    /* UART0 is the default debugging interface, so we must initialise UART
     * with the desired baud rate
     */
    uart_init(BIT_RATE_921600, BIT_RATE_921600);
    
    gpio_init();
    GPIO_DIS_OUTPUT(BUTTON_PIN);

    os_timer_disarm(&os_timer);
    os_timer_setfn(&os_timer, &main_on_timer, (void *)NULL);
    os_timer_arm(&os_timer, 25, 0);
}
