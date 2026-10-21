/**
 * @file    map.h
 * @brief   9 节点场地地图 + Dijkstra 寻路（合并参考工程的
 *          map_graph / path_planner / navigation / path_executor）
 *
 * 节点坐标沿用参考工程：+X 向右、+Y 向上，航向 0=上、+90=右、-90=左。
 * 起终点停车位姿不在路网里，单独由 Map_ParkingPose() 给出。
 */
#ifndef MAP_H
#define MAP_H

#include <stdint.h>

#define MAP_NODE_COUNT      9
#define MAP_PATH_MAX        10
#define MAP_INF_COST        0x3FFF

typedef enum {
    MAP_TASK_NONE = 0,
    MAP_TASK_QR,
    MAP_TASK_RAW,
    MAP_TASK_ROUGH,
    MAP_TASK_STORAGE,
} MapTaskId_t;

typedef struct {
    float   x;              /* 场地 X，单位 m */
    float   y;              /* 场地 Y，单位 m */
    uint8_t task_id;        /* MapTaskId_t */
    uint8_t yaw_valid;
    float   yaw_deg;        /* 场地航向，0=上，+90=右 */
} MapNode_t;

typedef struct {
    uint8_t node[MAP_PATH_MAX];
    uint8_t length;
    uint8_t valid;
    uint16_t total_cost;
} MapPath_t;

extern MapNode_t g_map_nodes[MAP_NODE_COUNT + 1];

/** 按障碍掩码建图。bit0~8 = 节点 1~9；bit9~20 = 12 条边。 */
void Map_Build(uint32_t obstacle_mask);

/** Dijkstra：从 start 到 target。结果写入 out。 */
void Map_FindPath(uint8_t start, uint8_t target, MapPath_t *out);

/** N3/N9 对应的启停区中心坐标。返回 1 成功。 */
uint8_t Map_ParkingPose(uint8_t corner_node, float *x, float *y);

/** 节点编号是否合法。 */
uint8_t Map_IsNode(uint8_t node);

#endif /* MAP_H */
