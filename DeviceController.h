#pragma once
#include "Definitions.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <chrono>
#include <thread>
#include <iostream>
#include <conio.h>

class DeviceController
{
public:
    DeviceController();
    ~DeviceController();

    void configureDevice();
    void selectOperationMode();
    void closeDevice();

private:
    // Private member variables
    HANDLE deviceHandle;
    HANDLE subDeviceHandle;
    DWORD errorCode;
    WORD nodeID1;
    WORD nodeID2;
    
    // Set gear ratio and encoder counts per revolution
    constexpr float gearRatio = 35.0f;
    constexpr int encoderCountsPerRevolution = 4096;

    // Private member functions
    void handleProfilePositionMode();
    void handleProfileVelocityMode();
};
