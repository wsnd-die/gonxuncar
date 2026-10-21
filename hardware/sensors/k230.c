/**
 * @file    k230.c
 * @brief   K230 视觉模块 ASCII 行协议（UART4）
 */
#include "k230.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

#define K230_LINE_MAX   64U

static K230_Status_t s_status;
static uint8_t  s_line[K230_LINE_MAX];
static uint16_t s_line_len;
static uint8_t  s_discard;
static volatile uint32_t s_events;

/* ---------------- 解析 ---------------- */

static bool prefix(const char *s, const char *p)
{
    return strncmp(s, p, strlen(p)) == 0;
}

static uint16_t parse_unsigned(const char *s)
{
    uint32_t v = 0U;
    while (*s >= '0' && *s <= '9') {
        v = v * 10U + (uint32_t)(*s - '0');
        if (v > 65535U) {
            v = 65535U;
        }
        s++;
    }
    return (uint16_t)v;
}

static void process_line(char *line)
{
    s_status.last_ms = HAL_GetTick();
    s_status.message_count++;

    if (line[0] >= '1' && line[0] <= '6' && line[1] == '\0') {
        s_status.color_id = (uint8_t)(line[0] - '0');
        s_events |= K230_EVT_COLOR;
        return;
    }

    if (prefix(line, "target,")) {
        s_status.target_id = (uint8_t)parse_unsigned(line + 7);
        s_events |= K230_EVT_TARGET;
        return;
    }

    if ((line[0] == 'a' || line[0] == 'd' ||
         line[0] == 'w' || line[0] == 's') && line[1] == ',') {
        char dir = line[0];
        uint16_t px = parse_unsigned(line + 2);
        uint32_t now = HAL_GetTick();

        s_status.dir = dir;
        s_status.offset_px = px;
        s_status.offset_ms = now;

        if (dir == 'w' || dir == 's') {
            s_status.x_dir = dir;
            s_status.x_px = px;
            s_status.x_ms = now;
        } else {
            s_status.y_dir = dir;
            s_status.y_px = px;
            s_status.y_ms = now;
        }
        s_events |= K230_EVT_OFFSET;
        return;
    }

    if (strcmp(line, "grab") == 0) {
        s_events |= K230_EVT_GRAB;
        return;
    }
    if (strcmp(line, "place") == 0) {
        s_events |= K230_EVT_PLACE;
        return;
    }
    if (prefix(line, "place,")) {
        s_status.target_id = (uint8_t)parse_unsigned(line + 6);
        s_events |= (K230_EVT_PLACE | K230_EVT_TARGET);
        return;
    }
    if (strcmp(line, "stopped") == 0) {
        s_events |= K230_EVT_STOPPED;
        return;
    }
    if (strcmp(line, "heartbeat,1") == 0) {
        return;
    }
    if (prefix(line, "error,")) {
        strncpy(s_status.error, line + 6, sizeof(s_status.error) - 1U);
        s_status.error[sizeof(s_status.error) - 1U] = '\0';
        s_events |= K230_EVT_ERROR;
        return;
    }
    /* 其它行忽略。 */
}

/* ---------------- 字节状态机 ---------------- */

void K230_Init(void)
{
    memset(&s_status, 0, sizeof(s_status));
    s_line_len = 0U;
    s_discard = 0U;
    s_events = 0U;
    /* 接收由 device/uart_dispatch.c 统一挂接并调用 K230_FeedByte()。 */
}

void K230_FeedByte(uint8_t b)
{
    if (b == '\r' || b == '\n') {
        if (!s_discard && s_line_len > 0U) {
            s_line[s_line_len] = '\0';
            process_line((char *)s_line);
        }
        s_line_len = 0U;
        s_discard = 0U;
        return;
    }

    if (b < 0x20U || b > 0x7EU) {
        s_discard = 1U;
        s_line_len = 0U;
        return;
    }

    if (s_discard) {
        return;
    }
    if (s_line_len >= (K230_LINE_MAX - 1U)) {
        s_discard = 1U;
        s_line_len = 0U;
        return;
    }
    s_line[s_line_len++] = b;
}

void K230_Update(uint32_t now_ms)
{
    s_status.online = ((now_ms - s_status.last_ms) <= K230_DATA_TIMEOUT_MS) ? 1U : 0U;
}

uint32_t K230_TakeEvents(void)
{
    uint32_t ev = s_events;
    s_events = 0U;
    return ev;
}

const K230_Status_t *K230_Status(void)
{
    return &s_status;
}

void K230_ClearAxes(void)
{
    s_status.x_dir = '\0';
    s_status.y_dir = '\0';
}

/* ---------------- 发送 ---------------- */

static HAL_StatusTypeDef k230_send(const char *text)
{
    return HAL_UART_Transmit(&huart4, (uint8_t *)text,
                             (uint16_t)strlen(text), 100U);
}

HAL_StatusTypeDef K230_StartDetect(uint8_t color_id)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "detect,%u\n", (unsigned)color_id);
    return k230_send(buf);
}

HAL_StatusTypeDef K230_StartPlace(uint8_t position_id)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "place,%u\n", (unsigned)position_id);
    return k230_send(buf);
}

HAL_StatusTypeDef K230_StartStack(uint8_t position_id, uint8_t anchor_color)
{
    char buf[24];
    snprintf(buf, sizeof(buf), "stack,%u,%u\n",
             (unsigned)position_id, (unsigned)anchor_color);
    return k230_send(buf);
}

HAL_StatusTypeDef K230_Stop(void)
{
    return k230_send("stop\n");
}
