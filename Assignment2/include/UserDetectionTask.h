#ifndef USER_DETECTION_TASK_H
#define USER_DETECTION_TASK_H

#include <Arduino.h>

class UserDetectionTask {
public:
    // 构造函数，传入 PIR 传感器引脚
    UserDetectionTask(int pirPin);

    // 初始化任务，设置传感器引脚模式
    void init();

    // 更新任务，检查是否检测到用户
    bool isUserDetected();

    // 获取最近一次检测到用户的时间
    unsigned long getLastDetectedTime();

private:
    int pirPin;                     // PIR传感器引脚
    bool userDetected;              // 当前是否检测到用户
    unsigned long lastDetectedTime; // 上次检测到用户的时间
};

#endif
