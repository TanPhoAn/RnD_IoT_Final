#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tremo_uart.h"
#include "tremo_gpio.h"
#include "tremo_rcc.h"
#include "tremo_delay.h"

#define UART_RX_BUF_SIZE 150

uint8_t rx_buf[UART_RX_BUF_SIZE];
volatile uint16_t rx_index = 0;

extern uint16_t lora_buf_size;
volatile bool new_data_available = false;


uint8_t test=0;

void uart_init_config(void);
void handle_uart_rx(void);
char* run_uart(void); // prototype

void uart_init_config(void)
{
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOB, true);

    gpio_set_iomux(GPIOB, GPIO_PIN_0, 1); // UART0_RX
    gpio_set_iomux(GPIOB, GPIO_PIN_1, 1); // UART0_TX

    uart_config_t uart_config;
    uart_config_init(&uart_config);

    uart_config.baudrate = UART_BAUDRATE_115200;
    uart_config.data_width = UART_DATA_WIDTH_8;
    uart_config.parity = UART_PARITY_NO;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_control = UART_FLOW_CONTROL_DISABLED;
    uart_config.mode = UART_MODE_TXRX;
    uart_config.fifo_mode = 1;

    uart_init(UART0, &uart_config);
    uart_cmd(UART0, true);

    uart_config_interrupt(UART0, UART_INTERRUPT_RX_DONE, true);

    uart_config_interrupt(UART0, UART_INTERRUPT_RX_TIMEOUT , true);

   // uart_clear_interrupt(UART0, UART_INTERRUPT_RX_DONE);
}

void handle_uart_rx(void)
{
    while (uart_get_flag_status(UART0, UART_FLAG_RX_FIFO_EMPTY) != SET

) {
        char ch = uart_receive_data(UART0);
          //  printf(" %c",ch);

       printf("%c",ch);
      if (rx_index < UART_RX_BUF_SIZE - 1) {
            rx_buf[rx_index++] = ch;

         
            if (ch == '\n' || ch=="\r") {
                printf(" end of line" );
                rx_buf[rx_index] = '\0'; // null-terminate
                lora_buf_size = rx_index;
                rx_index = 0;
                new_data_available = true;
             
            }
        } else {
            // buffer overflow protection
            rx_index = 0;
            new_data_available = false;
        }

       
    }
/*
    lora_buf_size=rx_index;
    rx_buf[rx_index] = '/0';
    rx_index=0;
    new_data_available=true;


*/




/*
    // Nếu không có ký tự kết thúc, vẫn xử lý nếu có dữ liệu
    if (rx_index > 0) {
        rx_buf[rx_index] = '\0';
        lora_buf_size = rx_index;
        rx_index = 0;
        new_data_available = true;
    }
        */
}




/*

char* run_uart(void)
{
    uart_init_config();
    printf("UART Initialized. Waiting for data...\r\n");

    while (1)
    {
        if (uart_get_interrupt_status(UART0, UART_INTERRUPT_RX_DONE) == SET)
        {
            uart_clear_interrupt(UART0, UART_INTERRUPT_RX_DONE);
            handle_uart_rx();
        }

        if (new_data_available)
        {
            new_data_available = false;

            // Make a copy of the buffer to return
            char* data_copy = (char*)malloc(strlen((char*)rx_buf) + 1);
            if (data_copy != NULL)
            {
                strcpy(data_copy, (char*)rx_buf);
                return data_copy;
            }
            else
            {
                printf("Memory allocation failed!\r\n");
                return NULL;
            }
        }
    }
}

*/