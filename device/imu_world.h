/**
 * @file    imu_world.h
 * @brief   HWT906 航向/角速度的薄封装
 *
 * 参考工程的 edge_motion / heading_control 直接读 HWT101；本工程保留 HWT906，
 * 所以用这一层把"航向 + 陀螺 + 新鲜度 + 世界零点"统一成简单接口。
 *
 * 坐标系：+X 前、+Y 左、yaw 逆时针为正（与 locator_wheel 一致）。
 */
#ifndef IMU_WORLD_H
#define IMU_WORLD_H

#include <stdint.h>
#include <stdbool.h>

/** @brief 初始化（只清状态；HWT906 本身由 HWT_IMU_Init 初始化）。 */
void ImuWorld_Init(void);

/** @brief 每读到一帧新数据调用一次（HWT_IMU_Poll 返回 1 后）。 */
void ImuWorld_Update(uint32_t now_ms);

/** @brief 把当前航向记为世界 0°。 */
void ImuWorld_SetWorldZero(void);

/** @brief 当前航向，单位度，CCW 正，范围 -180..180。 */
float ImuWorld_YawDeg(void);

/** @brief 当前航向，单位弧度。 */
float ImuWorld_YawRad(void);

/** @brief 陀螺 Z 轴角速度，单位 deg/s，CCW 正。 */
float ImuWorld_GyroZDps(void);

/** @brief 最近一帧是否在 timeout_ms 内。 */
bool ImuWorld_IsFresh(uint32_t now_ms, uint32_t timeout_ms);

/** @brief 模块是否在线。 */
bool ImuWorld_IsOnline(void);

#endif /* IMU_WORLD_H */
