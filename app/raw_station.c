/**
 * @file    raw_station.c
 * @brief   原料区停靠与取料
 *
 * 参考工程在 N8 完成 -90° 作业航向后，车体向 +X（参考车体右）靠近 40mm。
 * 本工程车体 +X 是前方，所以参考 +X 对应本工程车体 -Y（右），
 * 见 Chassis_MoveBody 的 (0, -40mm)。
 */
#include "raw_station.h"
#include "robot_config.h"
#include "chassis.h"
#include "zone_task.h"
#include "manipulator.h"
#include "cmsis_os2.h"

bool RawStation_Run(uint8_t batch)
{
    /* 机械臂必须完全缩回再靠近。 */
    Manip_Home();

    if (!Chassis_MoveBody(0.0f, -RAW_DOCK_BODY_X_M, 0.0f, 8000U)) {
        return false;
    }
    osDelay(RAW_DOCK_SETTLE_MS);

    bool ok = Zone_RunRaw(batch);

    /* 不论取料成功与否都要退回，避免卡在圆盘边。 */
    (void)Chassis_MoveBody(0.0f, RAW_DOCK_BODY_X_M, 0.0f, 8000U);
    osDelay(RAW_DOCK_SETTLE_MS);

    return ok;
}
