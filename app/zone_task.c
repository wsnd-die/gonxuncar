/**
 * @file    zone_task.c
 * @brief   原料/粗加工/暂存三个区域的动作序列
 */
#include "zone_task.h"
#include "robot_config.h"
#include "manipulator.h"
#include "vision_align.h"

static const uint8_t k_raw_colors[2][3] = {
    { K230_RAW_B1_COLOR_1, K230_RAW_B1_COLOR_2, K230_RAW_B1_COLOR_3 },
    { K230_RAW_B2_COLOR_1, K230_RAW_B2_COLOR_2, K230_RAW_B2_COLOR_3 },
};

static const ManipPose_t k_rough_pose[3] = {
    ZONE_ROUGH_POS1_POSE,
    ZONE_ROUGH_POS2_POSE,
    ZONE_ROUGH_POS3_POSE,
};

static const ManipPose_t k_storage_pose[3] = {
    ZONE_STORAGE_POS1_POSE,
    ZONE_STORAGE_POS2_POSE,
    ZONE_STORAGE_POS3_POSE,
};

bool Zone_RunRaw(uint8_t batch)
{
    if (batch > 1U) return false;

    for (uint8_t slot = 0U; slot < 3U; slot++) {
        float angle = CAROUSEL_PICKUP_POS2_DEG;
        float slide = MANIP_PICKUP_SLIDE_POS2_MM;

        if (!Vision_AlignArm(k_raw_colors[batch][slot],
                             &angle, &slide,
                             RAW_VISION_TIMEOUT_MS)) {
            return false;
        }

        ManipPose_t pose = {
            .carousel_deg = angle,
            .slide_mm = slide,
            .gimbal_mm = MANIP_PICKUP_TABLE_MM,
            .layer = 0U,
        };
        if (!Manip_PickFromGround(&pose, slot)) {
            return false;
        }
    }
    return true;
}

bool Zone_RunRough(uint8_t batch)
{
    (void)batch;

    /* 先放下三块，再按原槽位取回。 */
    for (uint8_t i = 0U; i < 3U; i++) {
        if (!Manip_PlaceToGround(&k_rough_pose[i], i)) {
            return false;
        }
    }
    for (uint8_t i = 0U; i < 3U; i++) {
        if (!Manip_PickFromGround(&k_rough_pose[i], i)) {
            return false;
        }
    }
    return true;
}

bool Zone_RunStorage(uint8_t batch)
{
    for (uint8_t i = 0U; i < 3U; i++) {
        bool ok = (batch == 0U)
                  ? Manip_PlaceToGround(&k_storage_pose[i], i)
                  : Manip_Stack(&k_storage_pose[i], i);
        if (!ok) {
            return false;
        }
    }
    return true;
}
