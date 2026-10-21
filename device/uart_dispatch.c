/**
 * @file    uart_dispatch.c
 * @brief   UART 接收分发
 *
 * 目前接入：
 *   UART4 (PC10/PC11) -> K230
 *   USART3 (PB10/PE15) -> HMI（Phase 5）
 *   UART5 (PC12/PD2)  -> VOFA（Phase 6）
 *
 * HAL_UART_RxCpltCallback / HAL_UART_ErrorCallback 全工程只能有一份定义。
 */
#include "uart_dispatch.h"
#include "usart.h"
#include "k230.h"
#include "hmi_protocol.h"
#include "vofa_debug.h"

static uint8_t s_rx4;
static uint8_t s_rx3;
static uint8_t s_rx5;

static void uart4_start(void)
{
    __HAL_UART_CLEAR_FLAG(&huart4, UART_CLEAR_OREF | UART_CLEAR_FEF | UART_CLEAR_NEF);
    (void)HAL_UART_Receive_IT(&huart4, &s_rx4, 1U);
}

static void usart3_start(void)
{
    __HAL_UART_CLEAR_FLAG(&huart3, UART_CLEAR_OREF | UART_CLEAR_FEF | UART_CLEAR_NEF);
    (void)HAL_UART_Receive_IT(&huart3, &s_rx3, 1U);
}

static void uart5_start(void)
{
    __HAL_UART_CLEAR_FLAG(&huart5, UART_CLEAR_OREF | UART_CLEAR_FEF | UART_CLEAR_NEF);
    (void)HAL_UART_Receive_IT(&huart5, &s_rx5, 1U);
}

void UartDispatch_Init(void)
{
    uart4_start();
    usart3_start();
    uart5_start();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4) {
        K230_FeedByte(s_rx4);
        uart4_start();
    } else if (huart->Instance == USART3) {
        HMI_Protocol_RxByte(s_rx3);
        usart3_start();
    } else if (huart->Instance == UART5) {
        Vofa_FeedByte(s_rx5);
        uart5_start();
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4) {
        uart4_start();
    } else if (huart->Instance == USART3) {
        usart3_start();
    } else if (huart->Instance == UART5) {
        uart5_start();
    }
}
