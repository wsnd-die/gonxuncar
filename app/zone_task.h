/**
 * @file    zone_task.h
 * @brief   三个区域的动作序列（阻塞式）
 */
#ifndef ZONE_TASK_H
#define ZONE_TASK_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    ZONE_RAW = 0,
    ZONE_ROUGH,
    ZONE_STORAGE,
} ZoneId_t;

/** @brief 原料区：三块料依次识别 -> 夹取 -> 放入料盘 1/2/3。 */
bool Zone_RunRaw(uint8_t batch);

/** @brief 粗加工区：料盘 -> 三个圆环，再圆环 -> 料盘。 */
bool Zone_RunRough(uint8_t batch);

/** @brief 暂存区：料盘 -> 三个暂存位；batch=1 时为第二层叠放。 */
bool Zone_RunStorage(uint8_t batch);

#endif /* ZONE_TASK_H */
