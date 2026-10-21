/**
 * @file    chassis.c
 * @brief   底盘点任务（里程计闭环）
 */
#include "chassis.h"
#include "map.h"
#include "robot_config.h"
#include "mecanum.h"
#include "Send_motor.h"
#include "drv_wheel_odom.h"
#include "imu_world.h"
#include "cmsis_os2.h"
#include <math.h>

#define CHASSIS_PI          3.14159265358979f
#define CHASSIS_DEG2RAD     (CHASSIS_PI / 180.0f)
#define CHASSIS_RAD2DEG     (180.0f / CHASSIS_PI)

#define CHASSIS_VMAX        2.0f
#define CHASSIS_BRK         0.60f     /* 制动减速度 m/s² */
#define CHASSIS_WMAX        2.0f
#define CHASSIS_ARRIVE_VMAX 0.05f

static uint8_t s_current_node = 1U;

/* 场地坐标 -> 里程计坐标的平移/旋转偏移。 */
static float s_off_x;
static float s_off_y;
static float s_off_yaw;
static uint32_t s_obstacle_mask;

static float wrap_pi(float a)
{
    while (a > CHASSIS_PI)  a -= 2.0f * CHASSIS_PI;
    while (a < -CHASSIS_PI) a += 2.0f * CHASSIS_PI;
    return a;
}

static float clampf(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void Chassis_Init(void)
{
    locator_wheel.init();
    Map_Build(0U);
    s_current_node = 1U;
    s_off_x = 0.0f;
    s_off_y = 0.0f;
    s_off_yaw = 0.0f;
    s_obstacle_mask = 0U;
}

void Chassis_SetObstacleMask(uint32_t mask)
{
    s_obstacle_mask = mask;
}

void Chassis_SetMapPose(float map_x, float map_y, float map_yaw_deg)
{
    locator_wheel.update();
    PoseData_t p;
    locator_wheel.get_pose(&p);

    float bx = map_y;
    float by = -map_x;
    float byaw = -map_yaw_deg * CHASSIS_DEG2RAD;

    s_off_x = bx - p.x;
    s_off_y = by - p.y;
    s_off_yaw = wrap_pi(byaw - p.yaw);
}

void Chassis_Stop(void)
{
    MecanumResult zero = Mecanum_Calc(0.0f, 0.0f);
    Send_commandmotor(&zero);
}

void Chassis_GetPose(float *x, float *y, float *yaw_rad)
{
    PoseData_t p;
    locator_wheel.get_pose(&p);
    if (x) *x = p.x;
    if (y) *y = p.y;
    if (yaw_rad) *yaw_rad = p.yaw;
}

uint8_t Chassis_CurrentNode(void)
{
    return s_current_node;
}

void Chassis_SetCurrentNode(uint8_t node)
{
    if (Map_IsNode(node)) {
        s_current_node = node;
    }
}

/* ================================================================
 * 世界系闭环
 * ================================================================ */
bool Chassis_GotoWorld(float tx, float ty, float tyaw, uint32_t timeout_ms)
{
    uint32_t t0 = osKernelGetTickCount();
    uint8_t  arrive = 0U;
    uint8_t  invalid = 0U;

    for (;;) {
        locator_wheel.update();

        PoseData_t p;
        locator_wheel.get_pose(&p);

        if (p.valid == 0U) {
            if (++invalid > CHASSIS_MAX_INVALID_TICKS) {
                Chassis_Stop();
                return false;
            }
            osDelay(CHASSIS_CTRL_PERIOD_MS);
            continue;
        }
        invalid = 0U;

        float eyaw = wrap_pi(tyaw - p.yaw);
        float ex = tx - p.x;
        float ey = ty - p.y;
        float dist = sqrtf(ex * ex + ey * ey);

        if (dist < CHASSIS_XY_TOL_M &&
            fabsf(eyaw) < (CHASSIS_YAW_TOL_DEG * CHASSIS_DEG2RAD)) {
            if (++arrive >= CHASSIS_ARRIVE_TICKS) {
                Chassis_Stop();
                return true;
            }
        } else {
            arrive = 0U;
        }

        /* 平移：近场 P，远场限制在制动曲线内。 */
        float v = CHASSIS_KP_XY * dist;
        float v_brk = sqrtf(2.0f * CHASSIS_BRK * dist);
        if (v > v_brk) v = v_brk;
        v = clampf(v, 0.0f, CHASSIS_VMAX);
        if (dist < 0.02f) {
            v = 0.0f;
        }

        float vx_body = 0.0f;
        float vy_body = 0.0f;
        if (dist > 1e-4f) {
            float c = cosf(p.yaw);
            float s = sinf(p.yaw);
            vx_body = v * (ex * c + ey * s) / dist;
            vy_body = v * (-ex * s + ey * c) / dist;
        }

        float gyro = ImuWorld_GyroZDps() * CHASSIS_DEG2RAD;
        float w = CHASSIS_KP_YAW * eyaw - CHASSIS_KD_YAW * gyro;
        w = clampf(w, -CHASSIS_WMAX, CHASSIS_WMAX);

        MecanumResult cmd = Mecanum_Calc_Full_V(vx_body, vy_body, w);
        Send_commandmotor(&cmd);

        if ((osKernelGetTickCount() - t0) >= timeout_ms) {
            Chassis_Stop();
            return false;
        }
        osDelay(CHASSIS_CTRL_PERIOD_MS);
    }
}

bool Chassis_GotoMapWorld(float x, float y, float yaw_deg, uint32_t timeout_ms)
{
    /* 场地(+X右,+Y上,CW+) -> 底盘(+X前,+Y左,CCW+)：
     *   base_x = map_y, base_y = -map_x, base_yaw = -map_yaw */
    float bx = y + s_off_x;
    float by = -x + s_off_y;
    float byaw = wrap_pi(-yaw_deg * CHASSIS_DEG2RAD + s_off_yaw);
    return Chassis_GotoWorld(bx, by, byaw, timeout_ms);
}

bool Chassis_MoveBody(float dx, float dy, float dyaw_rad, uint32_t timeout_ms)
{
    locator_wheel.update();
    PoseData_t p;
    locator_wheel.get_pose(&p);

    float c = cosf(p.yaw);
    float s = sinf(p.yaw);
    float tx = p.x + dx * c - dy * s;
    float ty = p.y + dx * s + dy * c;
    return Chassis_GotoWorld(tx, ty, wrap_pi(p.yaw + dyaw_rad), timeout_ms);
}

bool Chassis_RotateTo(float yaw_rad, uint32_t timeout_ms)
{
    float x, y, yaw;
    Chassis_GetPose(&x, &y, &yaw);
    return Chassis_GotoWorld(x, y, yaw_rad, timeout_ms);
}

/* ================================================================
 * 节点导航
 * ================================================================ */
static uint32_t leg_timeout(float distance_m)
{
    /* 速度按 0.6 m/s 估，加 4s 余量，最长 40s。 */
    uint32_t t = (uint32_t)(distance_m / 0.6f * 1000.0f) + 4000U;
    if (t > 40000U) t = 40000U;
    return t;
}

bool Chassis_GotoNode(uint8_t node, uint32_t timeout_ms)
{
    (void)timeout_ms;

    if (!Map_IsNode(node)) {
        return false;
    }
    if (node == s_current_node) {
        /* 已经在目标节点：只做作业航向。 */
        if (g_map_nodes[node].yaw_valid) {
            return Chassis_GotoMapWorld(g_map_nodes[node].x,
                                        g_map_nodes[node].y,
                                        g_map_nodes[node].yaw_deg,
                                        8000U);
        }
        return true;
    }

    Map_Build(s_obstacle_mask);
    MapPath_t path;
    Map_FindPath(s_current_node, node, &path);
    if (!path.valid) {
        return false;
    }

    for (uint8_t i = 1U; i < path.length; i++) {
        uint8_t n = path.node[i];
        bool last = (i == (uint8_t)(path.length - 1U));

        float mx = g_map_nodes[n].x;
        float my = g_map_nodes[n].y;
        float myaw_deg;

        if (last && g_map_nodes[n].yaw_valid) {
            myaw_deg = g_map_nodes[n].yaw_deg;
        } else {
            /* 中间节点不改变朝向。 */
            float x, y, yaw;
            Chassis_GetPose(&x, &y, &yaw);
            myaw_deg = -yaw * CHASSIS_RAD2DEG;
        }

        float from_x = g_map_nodes[path.node[i - 1U]].x;
        float from_y = g_map_nodes[path.node[i - 1U]].y;
        float dist = hypotf(mx - from_x, my - from_y);

        if (!Chassis_GotoMapWorld(mx, my, myaw_deg, leg_timeout(dist))) {
            s_current_node = path.node[i - 1U];
            return false;
        }
        s_current_node = n;
    }

    return true;
}
