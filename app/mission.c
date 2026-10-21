/**
 * @file    mission.c
 * @brief   搬运比赛主流程
 *
 *   启停区 -> N6 扫码(2s 占位) -> N8 原料1 -> N2 粗加工1 -> N4 暂存1
 *          -> N8 原料2 -> N2 粗加工2 -> N4 暂存2 -> 回程路口 -> 启停区
 */
#include "mission.h"
#include "robot_config.h"
#include "map.h"
#include "chassis.h"
#include "zone_task.h"
#include "raw_station.h"
#include "vision_align.h"
#include "manipulator.h"
#include "hmi_protocol.h"
#include "hmi_display.h"
#include "drv_wheel_odom.h"
#include "cmsis_os2.h"

#if !defined(MISSION_CHASSIS_ONLY)
#define MISSION_CHASSIS_ONLY 0
#endif

typedef enum {
    MISSION_STAGE_QR = 0,
    MISSION_STAGE_RAW_1,
    MISSION_STAGE_ROUGH_1,
    MISSION_STAGE_STORAGE_1,
    MISSION_STAGE_RAW_2,
    MISSION_STAGE_ROUGH_2,
    MISSION_STAGE_STORAGE_2,
    MISSION_STAGE_COUNT
} MissionStage_t;

static const uint8_t k_stage_node[MISSION_STAGE_COUNT] = {
    MISSION_QR_NODE,
    MISSION_RAW_NODE,
    MISSION_ROUGH_NODE,
    MISSION_STORAGE_NODE,
    MISSION_RAW_NODE,
    MISSION_ROUGH_NODE,
    MISSION_STORAGE_NODE,
};

static bool mission_do_stage(MissionStage_t stage)
{
    switch (stage) {
    case MISSION_STAGE_QR:
        osDelay(MISSION_TASK_HOLD_MS);
        return true;

    case MISSION_STAGE_RAW_1:
        return RawStation_Run(0U);

    case MISSION_STAGE_RAW_2:
        return RawStation_Run(1U);

    case MISSION_STAGE_ROUGH_1:
    case MISSION_STAGE_ROUGH_2:
        if (!Vision_AlignChassis(VISION_PLACE, 2U, 0U,
                                 K230_ALIGN_TIMEOUT_MS)) {
            return false;
        }
        return Zone_RunRough(0U);

    case MISSION_STAGE_STORAGE_1:
        if (!Vision_AlignChassis(VISION_PLACE, 2U, 0U,
                                 K230_ALIGN_TIMEOUT_MS)) {
            return false;
        }
        return Zone_RunStorage(0U);

    case MISSION_STAGE_STORAGE_2:
        if (!Vision_AlignChassis(VISION_STACK, 2U, K230_RAW_B1_COLOR_2,
                                 K230_ALIGN_TIMEOUT_MS)) {
            return false;
        }
        return Zone_RunStorage(1U);

    default:
        return false;
    }
}

static const char *const k_stage_name[MISSION_STAGE_COUNT] = {
    "QR", "RAW-1", "ROUGH-1", "STORAGE-1",
    "RAW-2", "ROUGH-2", "STORAGE-2",
};

bool Mission_Run(uint8_t start_corner, uint8_t return_corner)
{
    float park_x, park_y;
    HMI_MapConfig_t cfg;

    /* 串口屏优先：收到过地图就用屏幕给的启停区与障碍。 */
    if (HMI_TakeMap(&cfg)) {
        start_corner = cfg.start_node;
        return_corner = cfg.return_node;
        Chassis_SetObstacleMask(cfg.obstacle_mask);
    }

    if (!Map_IsNode(start_corner) || !Map_IsNode(return_corner)) {
        return false;
    }
    if (!Map_ParkingPose(start_corner, &park_x, &park_y)) {
        return false;
    }

    /* 上电时车停在启停区、车头朝场地 +Y。 */
    Manip_Init();
    Manip_Home();
    Wheel_Odom_Reset();
    osDelay(200U);
    Chassis_SetCurrentNode(start_corner);
    Chassis_SetMapPose(park_x, park_y, 0.0f);

    osDelay(MISSION_START_DELAY_MS);

    /* 启停区 -> 路口（此时 current_node 已经是路口，GotoNode 会判成"已到达"，
     * 所以这里必须直接给路口坐标）。 */
    if (!Chassis_GotoMapWorld(g_map_nodes[start_corner].x,
                              g_map_nodes[start_corner].y,
                              0.0f, 15000U)) {
        HMI_Race_SetResult("EXIT FAIL");
        return false;
    }

    for (uint8_t i = 0U; i < MISSION_STAGE_COUNT; i++) {
        uint8_t node = k_stage_node[i];

        HMI_Race_SetStage(k_stage_name[i]);
        if (!Chassis_GotoNode(node, 60000U)) {
            HMI_Race_SetResult("NAV FAIL");
            return false;
        }

#if !MISSION_CHASSIS_ONLY
        HMI_Race_SetAction("RUN");
        if (!mission_do_stage((MissionStage_t)i)) {
            HMI_Race_SetResult("TASK FAIL");
            return false;
        }
        /* 机构动作后重新对准任务航向。 */
        if (!Chassis_GotoNode(node, 30000U)) {
            return false;
        }
#else
        osDelay(200U);
#endif
    }

    if (!Chassis_GotoNode(return_corner, 60000U)) {
        return false;
    }

    float px, py;
    if (Map_ParkingPose(return_corner, &px, &py)) {
        (void)Chassis_GotoMapWorld(px, py, 0.0f, 20000U);
    }

    Chassis_Stop();
    HMI_Race_SetStage("DONE");
    HMI_Race_SetResult("SUCCESS");
    return true;
}
