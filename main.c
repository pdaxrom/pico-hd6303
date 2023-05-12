#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <hardware/clocks.h>

//#define USB_UART

#ifdef USB_UART
#include <tusb.h>
#include <bsp/board.h>
#include "tusb_config.h"
#endif

#include "hd6303.h"
#include "hardware.h"
#include "hd6303_pi.pio.h"

#include "support/BOOTROM/bootrom.h"

#define CPU_CLOCK_HZ 2000000

static uint8_t cpu_memory[65536];

#ifdef USB_UART
static uint8_t cdc_rx_buf[64];
static uint8_t cdc_tx_buf[64];
static int cdc_rx_cnt;
static int cdc_tx_cnt;
static mutex_t cdc_rx_mutex;
static mutex_t cdc_tx_mutex;
#endif

void hd6303_pi()
{
    sleep_ms(4000);

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

    uint32_t clk_div = clock_get_hz(clk_sys) / (CPU_CLOCK_HZ * 4);
    uint32_t clk_hz = clock_get_hz(CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_SYS);

    printf("SYS clock is %d, current GPIO clock is %d\n", clock_get_hz(clk_sys), clk_hz);

    clock_gpio_init(CPU_CLK, CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_SYS, clk_div);
    clk_hz = clock_get_hz(CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_SYS);

    printf("GPIO clock is %d, clock div is %d\n", clk_hz, clk_div);

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

#if 0
    memset(cpu_memory, 0x01, sizeof(cpu_memory));
    cpu_memory[0x0100] = 0x86;
    cpu_memory[0x0101] = 0xa5;
    cpu_memory[0x0102] = 0xb7;
    cpu_memory[0x0103] = 0xe6;
    cpu_memory[0x0104] = 0xa0;
    cpu_memory[0x0105] = 0x4a;
    cpu_memory[0x0106] = 0x26;
    cpu_memory[0x0107] = 0xfd;
    cpu_memory[0x0108] = 0xb7;
    cpu_memory[0x0109] = 0xe6;
    cpu_memory[0x010a] = 0xa0;
    cpu_memory[0x010b] = 0x4a;
    cpu_memory[0x010c] = 0x26;
    cpu_memory[0x010d] = 0xfd;
    cpu_memory[0x010e] = 0x20;
    cpu_memory[0x010f] = 0xf0;
    cpu_memory[0xfffe] = 0x01;
    cpu_memory[0xffff] = 0x00;
    cpu_memory[0xffee] = 0x66;
    cpu_memory[0xffef] = 0x99;
#else
    memmove(&cpu_memory[0x10000 - bootrom_file_len], bootrom_file, bootrom_file_len);
#endif

    gpio_put(CPU_RST, 0);
    sleep_ms(100);
    gpio_put(CPU_RST, 1);

    uint32_t cpu_bus;
    uint32_t cpu_addr;
    uint8_t  cpu_data;
    while (true) {
	cpu_bus = pio_sm_get_blocking(pio, 0);
	cpu_addr = cpu_bus & 0xffff;
	if (cpu_bus & (1 << CPU_RW)) {
	    if (cpu_addr == HW_UART_DATA) {
#ifdef USB_UART
		uint8_t byte = 0;
		mutex_enter_blocking(&cdc_rx_mutex);
		if (cdc_rx_cnt > 0) {
		    byte = cdc_rx_buf[0];
		    cdc_rx_cnt--;
		    memmove(cdc_rx_buf, &cdc_rx_buf[1], cdc_rx_cnt);
		}
		mutex_exit(&cdc_rx_mutex);
		pio_sm_put(pio, 0, byte);
#else
		pio_sm_put(pio, 0, uart_get_hw(UART_ID)->dr);
#endif
	    } else if (cpu_addr == HW_UART_CONFIG) {
#ifdef USB_UART
		uint8_t byte = 0;
		mutex_enter_blocking(&cdc_rx_mutex);
		if (cdc_rx_cnt > 0) {
		    byte |= HW_UART_RRD;
		}
		mutex_exit(&cdc_rx_mutex);
		mutex_enter_blocking(&cdc_tx_mutex);
		if (cdc_tx_cnt < sizeof(cdc_tx_buf)) {
		    byte |= HW_UART_TRD;
		}
		mutex_exit(&cdc_tx_mutex);
		pio_sm_put(pio, 0, byte);
#else
		pio_sm_put(pio, 0,
			    ((uart_get_hw(UART_ID)->fr & UART_UARTFR_TXFF_BITS) ? 0x0 : HW_UART_TRD) |
			    ((uart_get_hw(UART_ID)->fr & UART_UARTFR_RXFE_BITS) ? 0x0 : HW_UART_RRD)
			  );
#endif
	    } else {
		pio_sm_put(pio, 0, cpu_memory[cpu_addr]);
	    }
	} else {
	    cpu_data = pio_sm_get_blocking(pio, 0);
	    if ((cpu_addr) == 0xe6a0) {
		gpio_put(LED_PIN, cpu_data & 1);
		cpu_memory[cpu_addr] = cpu_data;
	    } else if (cpu_addr == HW_UART_DATA) {
#ifdef USB_UART
		mutex_enter_blocking(&cdc_tx_mutex);
		if (cdc_tx_cnt < sizeof(cdc_tx_buf)) {
		    cdc_tx_buf[cdc_tx_cnt++] = cpu_data;
		} else {
		    // set overflow tx bit
		}
		mutex_exit(&cdc_tx_mutex);
#else
		uart_get_hw(UART_ID)->dr = cpu_data;
#endif
	    } else {
		cpu_memory[cpu_addr] = cpu_data;
	    }
	}

    }
}

int main()
{
    set_sys_clock_khz(220000, true);

#ifdef USB_UART
    board_init();
    tud_init(BOARD_TUD_RHPORT);
#endif

//    stdio_init_all();
    stdio_uart_init_full(UART_ID, BAUD_RATE, UART_TX_PIN, UART_RX_PIN);


#ifdef USB_UART
    cdc_rx_cnt = 0;
    cdc_tx_cnt = 0;

    mutex_init(&cdc_rx_mutex);
    mutex_init(&cdc_tx_mutex);
#endif

    multicore_launch_core1(hd6303_pi);

    while (true) {
#ifdef USB_UART
	tud_task(); // tinyusb device task

	if (tud_cdc_n_available(0)) {
	    uint8_t buf[64];
	    uint32_t count = tud_cdc_n_read(0, buf, sizeof(buf));
	    mutex_enter_blocking(&cdc_rx_mutex);
	    int accepted = sizeof(cdc_rx_buf) - cdc_rx_cnt;
	    if (accepted > 0) {
		accepted = (accepted > count) ? count : accepted;
		memmove(&cdc_rx_buf[cdc_rx_cnt], buf, accepted);
		cdc_rx_cnt += accepted;
	    }
	    if (count > accepted) {
		// set overflow rx bit
	    }
	    mutex_exit(&cdc_rx_mutex);
	}
	mutex_enter_blocking(&cdc_tx_mutex);
	for (int i = 0; i < cdc_tx_cnt; i++) {
	    tud_cdc_n_write_char(0, cdc_tx_buf[i]);
	}
	tud_cdc_n_write_flush(0);
	cdc_tx_cnt = 0;
	mutex_exit(&cdc_tx_mutex);
#else
        tight_loop_contents();
#endif
    }

    return 0;
}
