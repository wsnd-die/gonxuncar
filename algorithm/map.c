/**
 * @file    map.c
 * @brief   9 节点地图与 Dijkstra 寻路
 *
 * 坐标与边权直接取自参考工程（2026-10-01 实测场地）：
 *   水平边 1-2=850, 2-3=900, 4-5=850, 5-6=900, 7-8=850, 8-9=900
 *   垂直边 1-4=850, 4-7=900, 2-5=850, 5-8=900, 3-6=850, 6-9=900
 * 改节点坐标后必须同步改 Map_Build() 里的边权。
 */
#include "map.h"
#include "robot_config.h"

MapNode_t g_map_nodes[MAP_NODE_COUNT + 1] = {
    { 0.0f, 0.0f, MAP_TASK_NONE, 0U, 0.0f },
    { 0.35f, 0.35f, MAP_TASK_NONE,    0U,   0.0f }, /* N1 */
    { 1.20f, 0.35f, MAP_TASK_ROUGH,   1U,  90.0f }, /* N2 粗加工 */
    { 2.10f, 0.35f, MAP_TASK_NONE,    0U,   0.0f }, /* N3 */
    { 0.35f, 1.20f, MAP_TASK_STORAGE, 1U, 180.0f }, /* N4 暂存 */
    { 1.20f, 1.20f, MAP_TASK_NONE,    0U,   0.0f }, /* N5 */
    { 2.10f, 1.20f, MAP_TASK_QR,      1U,   0.0f }, /* N6 扫码 */
    { 0.35f, 2.10f, MAP_TASK_NONE,    0U,   0.0f }, /* N7 */
    { 1.20f, 2.10f, MAP_TASK_RAW,     1U, -90.0f }, /* N8 原料 */
    { 2.10f, 2.10f, MAP_TASK_NONE,    0U,   0.0f }, /* N9 */
};

/* 邻接矩阵，下标 1~9。 */
static uint16_t s_graph[MAP_NODE_COUNT + 1][MAP_NODE_COUNT + 1];

static void map_clear(void)
{
    for (uint8_t i = 0; i <= MAP_NODE_COUNT; i++) {
        for (uint8_t j = 0; j <= MAP_NODE_COUNT; j++) {
            s_graph[i][j] = (i == j) ? 0U : MAP_INF_COST;
        }
    }
}

static void map_add_edge(uint8_t a, uint8_t b, uint16_t cost)
{
    s_graph[a][b] = cost;
    s_graph[b][a] = cost;
}

static void map_remove_edge(uint8_t a, uint8_t b)
{
    s_graph[a][b] = MAP_INF_COST;
    s_graph[b][a] = MAP_INF_COST;
}

static void map_block_node(uint8_t node)
{
    for (uint8_t i = 1; i <= MAP_NODE_COUNT; i++) {
        if (i != node) {
            s_graph[node][i] = MAP_INF_COST;
            s_graph[i][node] = MAP_INF_COST;
        }
    }
}

uint8_t Map_IsNode(uint8_t node)
{
    return (node >= 1U && node <= MAP_NODE_COUNT) ? 1U : 0U;
}

void Map_Build(uint32_t obstacle_mask)
{
    map_clear();

    /* 完整拓扑。 */
    map_add_edge(1U, 2U, 850U);
    map_add_edge(2U, 3U, 900U);
    map_add_edge(4U, 5U, 850U);
    map_add_edge(5U, 6U, 900U);
    map_add_edge(7U, 8U, 850U);
    map_add_edge(8U, 9U, 900U);
    map_add_edge(1U, 4U, 850U);
    map_add_edge(4U, 7U, 900U);
    map_add_edge(2U, 5U, 850U);
    map_add_edge(5U, 8U, 900U);
    map_add_edge(3U, 6U, 850U);
    map_add_edge(6U, 9U, 900U);

    /* bit0~8：节点障碍。 */
    for (uint8_t n = 1U; n <= MAP_NODE_COUNT; n++) {
        if (obstacle_mask & (1UL << (n - 1U))) {
            map_block_node(n);
        }
    }

    /* bit9~20：12 条边障碍，顺序与参考工程一致。 */
    static const uint8_t edges[12][2] = {
        {1U, 2U}, {2U, 3U}, {4U, 5U}, {5U, 6U},
        {7U, 8U}, {8U, 9U}, {1U, 4U}, {4U, 7U},
        {2U, 5U}, {5U, 8U}, {3U, 6U}, {6U, 9U},
    };
    for (uint8_t i = 0U; i < 12U; i++) {
        if (obstacle_mask & (1UL << (9U + i))) {
            map_remove_edge(edges[i][0], edges[i][1]);
        }
    }
}

void Map_FindPath(uint8_t start, uint8_t target, MapPath_t *out)
{
    uint16_t dist[MAP_NODE_COUNT + 1];
    uint8_t  visited[MAP_NODE_COUNT + 1];
    uint8_t  prev[MAP_NODE_COUNT + 1];
    uint8_t  center[MAP_NODE_COUNT + 1];
    uint8_t  rev[MAP_PATH_MAX];

    if (out == 0) {
        return;
    }
    out->length = 0U;
    out->valid = 0U;
    out->total_cost = 0U;
    for (uint8_t i = 0U; i < MAP_PATH_MAX; i++) {
        out->node[i] = 0U;
    }

    if (!Map_IsNode(start) || !Map_IsNode(target)) {
        return;
    }

    if (start == target) {
        out->node[0] = start;
        out->length = 1U;
        out->valid = 1U;
        return;
    }

    for (uint8_t i = 1U; i <= MAP_NODE_COUNT; i++) {
        dist[i] = MAP_INF_COST;
        visited[i] = 0U;
        prev[i] = 0U;
        center[i] = 0U;
    }
    dist[start] = 0U;

    for (uint8_t iter = 1U; iter <= MAP_NODE_COUNT; iter++) {
        uint16_t best = MAP_INF_COST;
        uint8_t  best_center = 0U;
        uint8_t  u = 0U;

        for (uint8_t j = 1U; j <= MAP_NODE_COUNT; j++) {
            if (!visited[j] &&
                ((dist[j] < best) ||
                 (dist[j] == best && center[j] > best_center))) {
                best = dist[j];
                best_center = center[j];
                u = j;
            }
        }

        if (u == 0U || u == target) {
            break;
        }
        visited[u] = 1U;

        for (uint8_t j = 1U; j <= MAP_NODE_COUNT; j++) {
            if (visited[j] || s_graph[u][j] >= MAP_INF_COST) {
                continue;
            }
            uint16_t nd = (uint16_t)(dist[u] + s_graph[u][j]);
            uint8_t nc = center[u];
#if ROUTE_CENTER_TIE_BREAK
            if (j == ROUTE_CENTER_NODE) {
                nc++;
            }
#endif
            if (nd < dist[j] || (nd == dist[j] && nc > center[j])) {
                dist[j] = nd;
                center[j] = nc;
                prev[j] = u;
            }
        }
    }

    if (dist[target] >= MAP_INF_COST) {
        return;
    }

    uint8_t n = 0U;
    uint8_t u = target;
    while (u != 0U && n < MAP_PATH_MAX) {
        rev[n++] = u;
        if (u == start) {
            break;
        }
        u = prev[u];
    }
    if (rev[n - 1U] != start) {
        return;
    }

    out->length = n;
    for (uint8_t i = 0U; i < n; i++) {
        out->node[i] = rev[n - 1U - i];
    }
    out->total_cost = dist[target];
    out->valid = 1U;
}

uint8_t Map_ParkingPose(uint8_t corner_node, float *x, float *y)
{
    if (x == 0 || y == 0) {
        return 0U;
    }
    if (corner_node == 3U) {
        *x = 2.25f;
        *y = 0.15f;
        return 1U;
    }
    if (corner_node == 9U) {
        *x = 2.25f;
        *y = 2.25f;
        return 1U;
    }
    return 0U;
}
