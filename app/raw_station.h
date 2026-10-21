/**
 * @file    raw_station.h
 * @brief   原料区事务：靠近 40mm -> 三块取料 -> 退回
 */
#ifndef RAW_STATION_H
#define RAW_STATION_H

#include <stdint.h>
#include <stdbool.h>

bool RawStation_Run(uint8_t batch);

#endif /* RAW_STATION_H */
