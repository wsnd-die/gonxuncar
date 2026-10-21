/**
 * @file    k230.h
 * @brief   K230 视觉模块 —— UART4 ASCII 行协议
 *
 * TX: detect,<1..6>\n  place,<1..3>\n  stack,<1..3>,<1..6>\n  stop\n
 * RX: 每行一条消息，'\r' 或 '\n' 结束：
 *       1..6            颜色号
 *       target,<1..3>   当前目标位置号
 *       a/d/w/s,<px>    像素偏差（a/d=图像纵轴，w/s=横轴）
 *       grab / place / stopped / heartbeat,1 / error,<text>
 *
 * 仅接受可打印 ASCII，超长或非法行整行丢弃。
 */
#ifndef K230_H
#define K230_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32g4xx_hal.h"

#define K230_DATA_TIMEOUT_MS    3000U

#define K230_EVT_COLOR      0x01U
#define K230_EVT_TARGET     0x02U
#define K230_EVT_OFFSET     0x04U
#define K230_EVT_GRAB       0x08U
#define K230_EVT_PLACE      0x10U
#define K230_EVT_STOPPED    0x20U
#define K230_EVT_ERROR      0x40U

typedef struct {
    uint8_t  color_id;
    uint8_t  target_id;
    char     dir;              /* 'a' / 'd' / 'w' / 's' */
    uint16_t offset_px;
    uint32_t offset_ms;

    /* 两轴分别缓存，避免后一行覆盖前一行。 */
    char     x_dir;            /* w / s */
    uint16_t x_px;
    uint32_t x_ms;
    char     y_dir;            /* a / d */
    uint16_t y_px;
    uint32_t y_ms;

    char     error[20];
    uint32_t last_ms;
    uint32_t message_count;
    uint8_t  online;
} K230_Status_t;

void K230_Init(void);
void K230_FeedByte(uint8_t b);          /* UART 中断里调用 */
void K230_Update(uint32_t now_ms);      /* 任务里周期调用 */

uint32_t K230_TakeEvents(void);         /* 读取并清除事件位 */
const K230_Status_t *K230_Status(void);
void K230_ClearAxes(void);              /* 清两轴偏差，等下一帧 */

HAL_StatusTypeDef K230_StartDetect(uint8_t color_id);
HAL_StatusTypeDef K230_StartPlace(uint8_t position_id);
HAL_StatusTypeDef K230_StartStack(uint8_t position_id, uint8_t anchor_color);
HAL_StatusTypeDef K230_Stop(void);

#endif /* K230_H */
