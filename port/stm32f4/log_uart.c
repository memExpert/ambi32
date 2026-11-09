/*
 * STM32F4 port: debug log on USART1 TX (Debug builds only), interrupt
 * driven: TXE pulls the next byte through ambi_log_tx_byte(); the interrupt
 * switches itself off when there is nothing left (RM0383 §19.6.1).
 */
#include "f4.h"
#include "ambi_port_log.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_usart.h"

#if AMBI_BOARD_LOG_USART != 1
#error "STM32F4 port: the debug log supports USART1 only"
#endif

void ambi_port_log_init(void)
{
    f4_gpio_af(AMBI_BOARD_LOG_TX, LL_GPIO_SPEED_FREQ_LOW);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
    LL_USART_SetBaudRate(USART1, F4_PCLK2_HZ, LL_USART_OVERSAMPLING_16, AMBI_BOARD_LOG_BAUD);
    LL_USART_ConfigCharacter(USART1, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);
    LL_USART_SetTransferDirection(USART1, LL_USART_DIRECTION_TX);
    LL_USART_Enable(USART1);
    NVIC_SetPriority(USART1_IRQn, F4_PRIO_LOG_UART);
    NVIC_EnableIRQ(USART1_IRQn);
}

void ambi_port_log_kick(void)
{
    LL_USART_EnableIT_TXE(USART1);   /* TXE is set while idle: the interrupt sends the first byte */
}

void USART1_IRQHandler(void)
{
    if (LL_USART_IsEnabledIT_TXE(USART1) && LL_USART_IsActiveFlag_TXE(USART1)) {
        int b = ambi_log_tx_byte();
        if (b < 0) {
            LL_USART_DisableIT_TXE(USART1);
        } else {
            LL_USART_TransmitData8(USART1, (uint8_t)b);
        }
    }
}
