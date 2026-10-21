/**
 * @file    vision_align.h
 * @brief   K230 视觉对位（阻塞式）
 *
 *   Vision_AlignChassis()  粗加工/暂存区：K230 偏差 -> 底盘平移
 *   Vision_AlignArm()      原料区：K230 偏差 -> 云台角 + 滑台伸缩
 */
#ifndef VISION_ALIGN_H
#define VISION_ALIGN_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    VISION_PLACE = 0,   /* place,<位置号> */
    VISION_STACK,       /* stack,<位置号>,<颜色号> */
    VISION_DETECT,      /* detect,<颜色号> */
} VisionMode_t;

/** @brief 底盘对位。pos_id 用于 PLACE/STACK，color_id 用于 DETECT/STACK。 */
bool Vision_AlignChassis(VisionMode_t mode, uint8_t pos_id,
                         uint8_t color_id, uint32_t timeout_ms);

/**
 * @brief 云台/滑台二维对位。
 * @param carousel_deg  输出最终云台角
 * @param slide_mm      输出最终滑台伸出量
 */
bool Vision_AlignArm(uint8_t color_id, float *carousel_deg,
                     float *slide_mm, uint32_t timeout_ms);

#endif /* VISION_ALIGN_H */
