/**
 * @file    mission.h
 * @brief   搬运比赛主流程（阻塞式，8 个 stage）
 */
#ifndef MISSION_H
#define MISSION_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 跑完整场比赛。
 * @param start_corner  启停区对应路口：3（启停区2）或 9（启停区1）
 * @param return_corner 回程路口，一般与 start_corner 相同
 * @return true 全流程完成
 */
bool Mission_Run(uint8_t start_corner, uint8_t return_corner);

#endif /* MISSION_H */
