/**
 * @file    app_tasks.h
 * @brief   FreeRTOS 任务入口与分模块测试
 *
 * 分模块调试：打开 app_tasks.c 的 APP_Task()，注释/取消注释其中一行即可。
 */
#ifndef APP_TASKS_H
#define APP_TASKS_H

#include "cmsis_os2.h"

void App_Init(void);

void APP_Task(void *argument);
void IMU_Task(void *argument);
void HMI_Task(void *argument);

void Test_Chassis(void);
void Test_Manipulator(void);
void Test_Vision(void);
void Test_All(void);

#endif /* APP_TASKS_H */
