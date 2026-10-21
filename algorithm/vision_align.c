/**
 * @file    vision_align.c
 * @brief   K230 视觉对位（精简版）
 *
 * K230 上行偏差：
 *   w/s 轴：px_x > 0 = 参考车体右
 *   a/d 轴：px_y > 0 = 参考车体前
 * 本工程车体：+X 前、+Y 左。因此
 *   base_dx = px_y * mm_per_px
 *   base_dy = -px_x * mm_per_px
 */
#include "vision_align.h"
#include "k230.h"
#include "chassis.h"
#include "manipulator.h"
#include "servo_pwm.h"
#include "robot_config.h"
#include "cmsis_os2.h"
#include <math.h>
#include <stdlib.h>

#define VISION_MM_PER_PX   0.5f

static int32_t axis_x(void)
{
    const K230_Status_t *s = K230_Status();
    if (s->x_dir == 'w') return (int32_t)s->x_px;
    if (s->x_dir == 's') return -(int32_t)s->x_px;
    return 0;
}

static int32_t axis_y(void)
{
    const K230_Status_t *s = K230_Status();
    if (s->y_dir == 'a') return (int32_t)s->y_px;
    if (s->y_dir == 'd') return -(int32_t)s->y_px;
    return 0;
}

static void send_stop_and_wait(uint32_t timeout_ms)
{
    uint32_t t0 = osKernelGetTickCount();
    K230_Stop();
    while ((osKernelGetTickCount() - t0) < timeout_ms) {
        K230_Update(osKernelGetTickCount());
        if (K230_TakeEvents() & K230_EVT_STOPPED) {
            return;
        }
        osDelay(20U);
    }
}

bool Vision_AlignChassis(VisionMode_t mode, uint8_t pos_id,
                         uint8_t color_id, uint32_t timeout_ms)
{
    uint32_t t0 = osKernelGetTickCount();
    uint32_t last_send = 0U;
    bool color_ok = (mode != VISION_DETECT);
    bool target_ok = (mode != VISION_STACK);
    uint32_t consumed_x_ms = 0U;
    uint32_t consumed_y_ms = 0U;
    bool ok = false;

    Servo_MoveSmooth(SERVO_CH_CAROUSEL, K230_ALIGN_CAROUSEL_ANGLE_DEG, 10U, 20U);
    Servo_MoveSmooth(SERVO_CH_GRIPPER, GRIPPER_OPEN_SMALL_DEG, 10U, 20U);
    osDelay(200U);
    (void)K230_TakeEvents();

    for (;;) {
        uint32_t now = osKernelGetTickCount();
        K230_Update(now);
        uint32_t ev = K230_TakeEvents();

        if (ev & K230_EVT_ERROR) {
            break;
        }
        if (ev & K230_EVT_COLOR) {
            color_ok = (K230_Status()->color_id == color_id);
        }
        if (ev & K230_EVT_TARGET) {
            target_ok = (K230_Status()->target_id == pos_id);
        }
        if (mode == VISION_PLACE && (ev & K230_EVT_PLACE)) {
            ok = true;
            break;
        }
        if (mode == VISION_DETECT && (ev & K230_EVT_GRAB)) {
            ok = true;
            break;
        }
        if (mode == VISION_STACK && target_ok && (ev & K230_EVT_PLACE)) {
            ok = true;
            break;
        }

        const K230_Status_t *s = K230_Status();
        bool have_x = (s->x_dir != '\0') && (s->x_ms != consumed_x_ms) &&
                      (s->x_ms >= t0);
        bool have_y = (s->y_dir != '\0') && (s->y_ms != consumed_y_ms) &&
                      (s->y_ms >= t0);

        if (color_ok && (have_x || have_y)) {
            int32_t px_x = have_x ? axis_x() : 0;
            int32_t px_y = have_y ? axis_y() : 0;

            consumed_x_ms = have_x ? s->x_ms : consumed_x_ms;
            consumed_y_ms = have_y ? s->y_ms : consumed_y_ms;

            if (abs(px_x) <= (int)K230_ALIGN_TOLERANCE_PX &&
                abs(px_y) <= (int)K230_ALIGN_TOLERANCE_PX) {
                ok = true;
                break;
            }

            float dx_mm = (float)px_y * VISION_MM_PER_PX;
            float dy_mm = -(float)px_x * VISION_MM_PER_PX;
            float len = sqrtf(dx_mm * dx_mm + dy_mm * dy_mm);
            if (len > K230_ALIGN_STEP_MAX_MM) {
                float k = K230_ALIGN_STEP_MAX_MM / len;
                dx_mm *= k;
                dy_mm *= k;
            }

            if (!Chassis_MoveBody(dx_mm * 0.001f, dy_mm * 0.001f,
                                  0.0f, 8000U)) {
                break;
            }
            continue;
        }

        /* 定期补发命令，避免丢包后干等。 */
        if ((now - last_send) >= K230_ALIGN_RESEND_MS) {
            if (mode == VISION_DETECT) {
                (void)K230_StartDetect(color_id);
            } else if (mode == VISION_STACK) {
                (void)K230_StartStack(pos_id, color_id);
            } else {
                (void)K230_StartPlace(pos_id);
            }
            last_send = now;
        }

        if ((now - t0) >= timeout_ms) {
            break;
        }
        osDelay(20U);
    }

    send_stop_and_wait(1000U);
    return ok;
}

bool Vision_AlignArm(uint8_t color_id, float *carousel_deg,
                     float *slide_mm, uint32_t timeout_ms)
{
    uint32_t t0 = osKernelGetTickCount();
    uint32_t last_send = 0U;
    float angle = K230_ALIGN_CAROUSEL_ANGLE_DEG;
    float slide = RAW_VISION_SLIDE_START_MM;
    bool color_ok = false;
    uint8_t stable = 0U;
    bool ok = false;

    Servo_MoveSmooth(SERVO_CH_CAROUSEL, angle, 10U, 20U);
    Servo_MoveSmooth(SERVO_CH_GRIPPER, GRIPPER_OPEN_SMALL_DEG, 10U, 20U);
    (void)Manip_SetSlideMm(slide);
    osDelay(200U);
    (void)K230_TakeEvents();

    for (;;) {
        uint32_t now = osKernelGetTickCount();
        K230_Update(now);
        uint32_t ev = K230_TakeEvents();

        if (ev & K230_EVT_ERROR) {
            break;
        }
        if (ev & K230_EVT_COLOR) {
            color_ok = (K230_Status()->color_id == color_id);
        }
        if (color_ok && (ev & K230_EVT_GRAB)) {
            ok = true;
            break;
        }

        const K230_Status_t *s = K230_Status();
        bool have_x = (s->x_dir != '\0');
        bool have_y = (s->y_dir != '\0');

        if (color_ok && (have_x || have_y)) {
            int32_t px_x = have_x ? axis_x() : 0;
            int32_t px_y = have_y ? axis_y() : 0;

            /* 清掉本帧，等下一帧。 */
            K230_ClearAxes();

            if (abs(px_x) <= (int)RAW_VISION_TOLERANCE_PX &&
                abs(px_y) <= (int)RAW_VISION_TOLERANCE_PX) {
                if (++stable >= RAW_VISION_STABLE_FRAMES) {
                    ok = true;
                    break;
                }
            } else {
                stable = 0U;

                float da = RAW_VISION_ANGLE_KP * (float)px_y;
                float ds = RAW_VISION_SLIDE_KP * (float)px_x;

                if (da > RAW_VISION_ANGLE_STEP_MAX) da = RAW_VISION_ANGLE_STEP_MAX;
                if (da < -RAW_VISION_ANGLE_STEP_MAX) da = -RAW_VISION_ANGLE_STEP_MAX;
                if (ds > RAW_VISION_SLIDE_STEP_MAX) ds = RAW_VISION_SLIDE_STEP_MAX;
                if (ds < -RAW_VISION_SLIDE_STEP_MAX) ds = -RAW_VISION_SLIDE_STEP_MAX;

                angle += da;
                slide += ds;
                if (slide < RAW_VISION_SLIDE_MIN_MM) slide = RAW_VISION_SLIDE_MIN_MM;
                if (slide > RAW_VISION_SLIDE_MAX_MM) slide = RAW_VISION_SLIDE_MAX_MM;

                Servo_SetAngle(SERVO_CH_CAROUSEL, angle);
                osDelay(100U);
                if (!Manip_SetSlideMm(slide)) {
                    break;
                }
            }

            last_send = 0U;   /* 调完立刻重新要一帧 */
            continue;
        }

        if ((now - last_send) >= K230_ALIGN_RESEND_MS) {
            (void)K230_StartDetect(color_id);
            last_send = now;
        }

        if ((now - t0) >= timeout_ms) {
            break;
        }
        osDelay(20U);
    }

    send_stop_and_wait(1000U);
    if (carousel_deg) *carousel_deg = angle;
    if (slide_mm) *slide_mm = slide;
    return ok;
}
