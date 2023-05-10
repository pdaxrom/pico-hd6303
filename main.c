#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hd6303.h"
#include "hd6303_pi.pio.h"

#define CPU_CLOCK_HZ (50000 * 4)

#define UART_TX_PIN (28)
#define UART_RX_PIN (29)	/* not available on the pico */
#define UART_ID     uart0
#define BAUD_RATE   115200

static uint8_t cpu_memory[65536];

int main()
{
    stdio_init_all();
    stdio_uart_init_full(UART_ID, BAUD_RATE, UART_TX_PIN, UART_RX_PIN);

    for (int i = 0; i <= 27; i++) {
	gpio_init(i);
	gpio_set_dir(i, GPIO_IN);
	gpio_set_pulls(i, false, false);
    }

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    gpio_init(CPU_RST);
    gpio_set_dir(CPU_RST, GPIO_OUT);
    gpio_pull_up(CPU_RST);
    gpio_put(CPU_RST, 1);

    uint32_t clk_hz = clock_get_hz(CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_SYS);
    uint32_t clk_div = clk_hz / CPU_CLOCK_HZ;

    printf("GPIO clock is %d, div is %d, cpu clock is %d\n", clk_hz, clk_div, clk_hz / clk_div / 4);

    clock_gpio_init(CPU_CLK, CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_SYS, clk_div);

    printf("Resetting...\n");

    PIO pio = pio0;
    pio_clear_instruction_memory(pio);
    uint offset = pio_add_program(pio, &pi_program);
    pi_program_init(pio, 0, offset);
    pio_sm_set_enabled(pio, 0, true);

    int count = 0;
    bool cpu_rw;

    uint32_t trace[64];
    uint32_t tdata[512];
    int trace_pos = 0;
    int tdata_pos = 0;

    memset(cpu_memory, 0x01, sizeof(cpu_memory));
    cpu_memory[0x0100] = 0x86;
    cpu_memory[0x0101] = 0xa5;
    cpu_memory[0x0102] = 0xb7;
    cpu_memory[0x0103] = 0xef;
    cpu_memory[0x0104] = 0x00;
    cpu_memory[0x0105] = 0x4a;
    cpu_memory[0x0106] = 0x26;
    cpu_memory[0x0107] = 0xfd;
    cpu_memory[0x0108] = 0xb7;
    cpu_memory[0x0109] = 0xef;
    cpu_memory[0x010a] = 0x00;
    cpu_memory[0x010b] = 0x4a;
    cpu_memory[0x010c] = 0x26;
    cpu_memory[0x010d] = 0xfd;
    cpu_memory[0x010e] = 0x20;
    cpu_memory[0x010f] = 0xf0;
    cpu_memory[0xfffe] = 0x01;
    cpu_memory[0xffff] = 0x00;
    cpu_memory[0xffee] = 0x66;
    cpu_memory[0xffef] = 0x99;

    gpio_put(CPU_RST, 0);
    sleep_ms(100);
    gpio_put(CPU_RST, 1);

    while (true) {
	uint32_t cpu_addr = pio_sm_get_blocking(pio, 0);
//	trace[trace_pos++] = cpu_addr;
//	if (trace_pos == 64) {
//	    break;
//	}

	if (cpu_addr & (1 << CPU_RW)) {
	    pio_sm_put(pio, 0, cpu_memory[cpu_addr & 0xffff]);
	} else {
	    uint32_t cpu_data = pio_sm_get_blocking(pio, 0);
//	    tdata[tdata_pos++] = cpu_data;
	    if ((cpu_addr & 0xffff) == 0xef00) {
		if (cpu_data & 0xff) {
		    gpio_put(LED_PIN, 1);
		} else {
		    gpio_put(LED_PIN, 0);
		}
	    }
	}

    }

/*
    for (int i = 0; i < 64; i++) {
	printf("%d: %05X\n", i, trace[i]);
    }

    for (int i = 0; i < tdata_pos; i++) {
	printf("data: %08X\n", tdata[i]);
    }

    while(true) {}
 */
    return 0;
}
