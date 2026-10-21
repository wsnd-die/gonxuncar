/**
 * @file    chassis.h
 * @brief   底盘点任务接口（与机械臂/视觉完全解耦）
 *
 * 定位：四轮编码器里程计（locator_wheel）+ HWT906 航向。
 * 控制：速度模式 + 世界系闭环，阻塞到到位/超时。
 *
 * 坐标系：
 *   GotoWorld / MoveBody 使用本工程底盘系：+X 前、+Y 左、yaw 逆时针为正。
 *   GotoNode / GotoMapWorld 使用场地系：+X 右、+Y 上、yaw 0=上、+90=右。
 */
#ifndef CHASSIS_H
#define CHASSIS_H

#include <stdint.h>
#include <stdbool.h>

void Chassis_Init(void);

/**
 * @brief 把当前车辆位置标定为场地坐标 (map_x, map_y, map_yaw_deg)。
 * @note  上电后停在启停区时调用一次；之后 GotoMapWorld/GotoNode 都以此原点换算。
 */
void Chassis_SetMapPose(float map_x, float map_y, float map_yaw_deg);

/** @brief 世界系闭环到目标点（底盘系）。 */
bool Chassis_GotoWorld(float x, float y, float yaw_rad, uint32_t timeout_ms);

/** @brief 场地系任意点。yaw_deg 用场地航向（0=上，+90=右）。 */
bool Chassis_GotoMapWorld(float x, float y, float yaw_deg, uint32_t timeout_ms);

/** @brief 车体相对移动：dx 前、dy 左、dyaw 逆时针。 */
bool Chassis_MoveBody(float dx, float dy, float dyaw_rad, uint32_t timeout_ms);

/** @brief 原地转到指定底盘航向（弧度）。 */
bool Chassis_RotateTo(float yaw_rad, uint32_t timeout_ms);

/** @brief 按 9 节点地图导航到目标节点，终到后执行节点作业航向。 */
bool Chassis_GotoNode(uint8_t node, uint32_t timeout_ms);

/** @brief 最后确认到达的节点。 */
uint8_t Chassis_CurrentNode(void);
void Chassis_SetCurrentNode(uint8_t node);

/** @brief 设置寻路障碍掩码（HMI 地图 bit0~20）。 */
void Chassis_SetObstacleMask(uint32_t mask);

/** @brief 立即零速停车。 */
void Chassis_Stop(void);

/** @brief 当前底盘位姿（世界系，底盘系坐标）。 */
void Chassis_GetPose(float *x, float *y, float *yaw_rad);

#endif /* CHASSIS_H */
