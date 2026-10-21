/**
 * @file    imu_world.c
 * @brief   HWT906 航向/角速度薄封装
 */
#include "imu_world.h"
#include "hwt_imu.h"

static volatile uint32_t s_last_ms;
static volatile uint8_t  s_have_data;

void ImuWorld_Init(void)
{
    s_last_ms = 0U;
    s_have_data = 0U;
}

void ImuWorld_Update(uint32_t now_ms)
{
    if (g_hwt_imu_data_ready != 0U) {
        s_last_ms = now_ms;
        s_have_data = 1U;
        g_hwt_imu_data_ready = 0U;
    }
}

void ImuWorld_SetWorldZero(void)
{
    HWT_IMU_SetZero();
}

float ImuWorld_YawDeg(void)
{
    return g_hwt_imu_yaw;
}

float ImuWorld_YawRad(void)
{
    return g_hwt_imu_yaw_rad;
}

float ImuWorld_GyroZDps(void)
{
    /* g_hwt_imu_gyro_z 的安装方向若与航向相反，只改这里一个符号。 */
    return g_hwt_imu_gyro_z;
}

bool ImuWorld_IsFresh(uint32_t now_ms, uint32_t timeout_ms)
{
    if (s_have_data == 0U) {
        return false;
    }
    return ((now_ms - s_last_ms) <= timeout_ms);
}

bool ImuWorld_IsOnline(void)
{
    return (g_hwt_imu_online != 0U);
}
