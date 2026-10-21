/**
 * @file    app_tasks.c
 * @brief   任务入口与分模块测试
 */
#include "app_tasks.h"
#include "robot_config.h"
#include "imu_world.h"
#include "chassis.h"
#include "manipulator.h"
#include "servo_pwm.h"
#include "k230.h"
#include "mission.h"
#include "vision_align.h"
#include "hwt_imu.h"
#include "uart_dispatch.h"
#include "hmi_protocol.h"
#include "hmi_display.h"
#include "emm_5v.h"
#include "vofa_debug.h"
#include "cmsis_os2.h"

void App_Init(void)
{
    /* 先使能 6 个 EMM 电机，再读编码器/发动作。 */
    for (uint8_t id = 1U; id <= 6U; id++) {
        Emm_V5_En_Control(id, 1, 0);
    }
    osDelay(200U);

    ImuWorld_Init();
    Chassis_Init();
    Manip_Init();
    K230_Init();
    HMI_Protocol_Init();
    HMI_Display_Init();
    Vofa_Init();
    UartDispatch_Init();
}

/* ================================================================
 * 分模块测试
 * ================================================================ */

void Test_Chassis(void)
{
    /* 前后、左右、转向各走一次；轮子架空时先看方向。 */
    (void)Chassis_MoveBody(0.30f, 0.0f, 0.0f, 10000U);
    (void)Chassis_MoveBody(-0.30f, 0.0f, 0.0f, 10000U);
    (void)Chassis_MoveBody(0.0f, 0.30f, 0.0f, 10000U);
    (void)Chassis_MoveBody(0.0f, -0.30f, 0.0f, 10000U);
    (void)Chassis_RotateTo(1.5708f, 10000U);   /* +90° */
    (void)Chassis_RotateTo(0.0f, 10000U);
    Chassis_Stop();
}

void Test_Manipulator(void)
{
    Manip_Home();
    osDelay(500U);

    /* 三路舵机来回。 */
    Servo_MoveSmooth(SERVO_CH_CAROUSEL, CAROUSEL_PICKUP_POS1_DEG, 10U, 20U);
    Servo_MoveSmooth(SERVO_CH_CAROUSEL, CAROUSEL_PICKUP_POS3_DEG, 10U, 20U);

    /* 云台/滑台小行程。 */
    (void)Manip_SetGimbalMm(-30.0f);
    (void)Manip_SetGimbalMm(0.0f);
    (void)Manip_SetSlideMm(40.0f);
    (void)Manip_SetSlideMm(0.0f);

    /* 四条动作各跑一次（位置为参考工程标定值）。 */
    ManipPose_t pose = { CAROUSEL_PICKUP_POS2_DEG,
                         MANIP_PICKUP_SLIDE_POS2_MM,
                         MANIP_PICKUP_TABLE_MM, 0U };
    (void)Manip_PickFromGround(&pose, 0U);
    (void)Manip_PlaceToGround(&pose, 0U);
    (void)Manip_Stack(&pose, 0U);
    Manip_Home();
}

void Test_Vision(void)
{
    (void)Vision_AlignChassis(VISION_PLACE, 2U, 0U, K230_ALIGN_TIMEOUT_MS);

    float angle = CAROUSEL_PICKUP_POS2_DEG;
    float slide = MANIP_PICKUP_SLIDE_POS2_MM;
    (void)Vision_AlignArm(1U, &angle, &slide, RAW_VISION_TIMEOUT_MS);

    Chassis_Stop();
}

void Test_All(void)
{
    /* 所有模块空转，VOFA 手动命令可用。 */
    Vofa_Update(osKernelGetTickCount());
    osDelay(20U);
}

/* ================================================================
 * 任务体
 * ================================================================ */

void IMU_Task(void *argument)
{
    (void)argument;

    for (;;) {
        if (HWT_IMU_Poll() != 0U) {
            ImuWorld_Update(osKernelGetTickCount());
        }
        osDelay(5U);
    }
}

void APP_Task(void *argument)
{
    (void)argument;

    App_Init();
    osDelay(1000U);

    for (;;) {
        /* ---- 分模块调试：只保留下面一行 ---- */
        Test_All();
        // Test_Chassis();
        // Test_Manipulator();
        // Test_Vision();
        // (void)Mission_Run(3U, 3U);

        osDelay(10U);
    }
}

void HMI_Task(void *argument)
{
    (void)argument;

    for (;;) {
        /* HMI 显示刷新在 Phase 5 接入。 */
        osDelay(20U);
    }
}
