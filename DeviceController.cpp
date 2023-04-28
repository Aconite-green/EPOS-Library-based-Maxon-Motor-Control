#include "DeviceController.h"

// Default constructor for the DeviceController class
DeviceController::DeviceController()
{
    deviceHandle = nullptr;
    errorCode = 0;
    subDeviceHandle = nullptr;
    nodeID1 = 1;
    nodeID2 = 2;
}

DeviceController::~DeviceController() {}

void DeviceController::configureDevice()
{
    // Configure the communication settings
    char deviceName[] = "EPOS4";
    char protocolStackName[] = "CANopen";
    char interfaceName[] = "NI_USB-8502 0";
    char portName[] = "CAN1";

    int functionChoice;
    std::cout << "Choose the function to open the device:\n";
    std::cout << "1. VCS_OpenDevice\n";
    std::cout << "2. VCS_OpenDeviceDlg\n";
    std::cout << "Enter your choice (1 or 2): ";
    std::cin >> functionChoice;

    if (functionChoice == 1)
    {
        deviceHandle = VCS_OpenDevice(deviceName, protocolStackName, interfaceName, portName, &errorCode);
    }
    else if (functionChoice == 2)
    {
        deviceHandle = VCS_OpenDeviceDlg(&errorCode);
    }
    else
    {
        std::cerr << "Invalid choice, exiting..." << std::endl;
        exit(1);
    }

    if (deviceHandle != NULL)
    {
        std::cout << std::endl;
        std::cout << "Device is opened successfully!" << std::endl;

        DWORD currentBaudRate = 0;
        DWORD currentTimeout = 0;

        BOOL success = VCS_GetProtocolStackSettings(deviceHandle, &currentBaudRate, &currentTimeout, &errorCode);

        if (success)
        {
            std::cout << std::endl;
            std::cout << "Current baud rate: " << currentBaudRate << " bit/s" << std::endl;
            std::cout << "Current timeout: " << currentTimeout << " ms" << std::endl;

            DWORD newBaudRate = currentBaudRate;
            DWORD newTimeout = currentTimeout;

            success = VCS_SetProtocolStackSettings(deviceHandle, newBaudRate, newTimeout, &errorCode);

            if (success)
            {
                std::cout << "Protocol stack settings updated successfully!" << std::endl;
            }
            else
            {
                std::cerr << "Failed to set protocol stack settings, error code: " << errorCode << std::endl;
            }
        }
        else
        {
            std::cerr << "Failed to get protocol stack settings, error code: " << errorCode << std::endl;
        }
    }
    }
    else
    {
        // Failed to open the device, handle the error here
        std::cerr << "Failed to open the device, error code: " << errorCode << std::endl;
    }
}

void DeviceController::selectOperationMode()
{
    if (deviceHandle != NULL)
    {
        int operationMode;

        while (true)
        {
            std::cout << "\n";
            std::cout << "Select operation mode (1: PPM, 3: PVM, 6: HM, 7: IPM, -1: PM, -2: VM, -3: CM, -5: MEM, -6: SDM): ";
            std::cin >> operationMode;

            BOOL success = VCS_SetOperationMode(deviceHandle, nodeID1, operationMode, &errorCode);

            if (success)
            {
                system("CLS");
                std::cout << "Operation mode set successfully!" << std::endl;
                std::cout << "Begin Mode setting" << std::endl;

                switch (operationMode)
                {
                case 1: // PPM
                    handleProfilePositionMode();
                    break;
                case 3: // PVM
                    handleProfileVelocityMode();
                    break;
                case 6: // HM
                    //handleHomingMode();
                    break;
                case 7: // IPM
                    //handleInterpolatedPositionMode();
                    break;
                case -1: // PM
                    //handlePositionMode();
                    break;
                case -2: // VM
                    //handleVelocityMode();
                    break;
                case -3: // CM
                    //handleCurrentMode();
                    break;
                case -5: // MEM
                    //handleMasterEncoderMode();
                    break;
                case -6: // SDM
                    //handleStepDirectionMode();
                    break;
                }

                __int8 currentMode;
                success = VCS_GetOperationMode(deviceHandle, nodeID1, &currentMode, &errorCode);
                if (success)
                    std::cout << "Current operation mode: " << (int)currentMode << std::endl;
                else
                    std::cerr << "Failed to get operation mode, error code: " << errorCode << std::endl;
            }
            else
            {
                std::cerr << "Failed to set operation mode, error code: " << errorCode << std::endl;
            }
        }
    }
}

// Closes the device, either closing only the current device or all opened devices
void DeviceController::closeDevice()
{
    if (deviceHandle != nullptr)
    {
        DWORD errorCode = 0;

        // Ask the user for the choice to close the device
        int closeChoice;
        std::cout << "\n";
        std::cout << "Choose the function to close the device:\n";
        std::cout << "1. VCS_CloseDevice (close only the current device)\n";
        std::cout << "2. VCS_CloseAllDevices (close all opened devices)\n";
        std::cout << "Enter your choice (1 or 2): ";

        while (!(std::cin >> closeChoice) || (closeChoice != 1 && closeChoice != 2))
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cerr << "Invalid input. Please enter 1 or 2: ";
        }

        BOOL closeSuccess = false;

        if (closeChoice == 1)
        {
            closeSuccess = VCS_CloseDevice(deviceHandle, &errorCode);
        }
        else if (closeChoice == 2)
        {
            closeSuccess = VCS_CloseAllDevices(&errorCode);
        }

        if (closeSuccess)
        {
            std::cout << "The device(s) closed successfully!" << std::endl;
        }
        else
        {
            std::cerr << "Failed to close device(s), error code: " << errorCode << std::endl;
        }

        // Clear device handle
        deviceHandle = nullptr;
    }
    else
    {
        std::cout << "Device is not open, nothing to close." << std::endl;
    }
}






// --------------------------------------------------------------------------------------------------------------------
//                                             Basic Motion Control Functions
// --------------------------------------------------------------------------------------------------------------------

// Handles the Profile Position Mode by activating it, setting position profile, enabling/disabling position window, and moving to a target position
float DeviceController::convertPositionToAngle(long position)
{
    return static_cast<float>(-1) * (position) / (encoderCountsPerRevolution * gearRatio) * 360;
}

long DeviceController::convertAngleToPosition(float angle)
{
    return static_cast<long>((-1) * (angle * encoderCountsPerRevolution * gearRatio) / 360);
}

void DeviceController::printCurrentPosition()
{
    long currentPosition = 0;
    BOOL success = VCS_GetPositionIs(deviceHandle, nodeID1, &currentPosition, &errorCode);

    if (success)
    {
        float currentAngle = convertPositionToAngle(currentPosition);
        std::cout << "Current position: " << currentAngle << " degrees" << std::endl;
    }
    else
    {
        std::cerr << "Failed to get current position, error code: " << errorCode << std::endl;
    }
}

void DeviceController::handleProfilePositionMode()
{
    
    DWORD errorCode = 0;
    
    // Activate Profile Position Mode
    BOOL success = VCS_ActivateProfilePositionMode(deviceHandle, nodeID1, &errorCode);

    if (success) {
        std::cout << "\n";
        std::cout << "Profile Position Mode activated." << std::endl;
    }
    else {
        std::cerr << "Failed to activate Profile Position Mode, error code: " << errorCode << std::endl;
        return;
    }

    BOOL enabled = VCS_SetEnableState(deviceHandle, nodeID1, &errorCode);
    if (enabled) {
        std::cout << "Device Enabled" << std::endl;
    }
    else {
        std::cerr << "Failed to Enable Device, error code: " << errorCode << std::endl;
        return;
    }



    // Set the position profile
    DWORD profileVelocity = 1000;
    DWORD profileAcceleration = 1000;
    DWORD profileDeceleration = 1000;

    success = VCS_SetPositionProfile(deviceHandle, nodeID1, profileVelocity, profileAcceleration, profileDeceleration, &errorCode);

    if (success) {
        std::cout << "Position profile set." << std::endl;
    }
    else {
        std::cerr << "Failed to set position profile, error code: " << errorCode << std::endl;
        return;
    }

    // Get the position profile
    success = VCS_GetPositionProfile(deviceHandle, nodeID1, &profileVelocity, &profileAcceleration, &profileDeceleration, &errorCode);

    if (success) {
        std::cout << "Position profile retrieved.\n\n" << std::endl;

    }
    else {
        std::cerr << "Failed to retrieve position profile, error code: " << errorCode << std::endl;
        return;
    }

    
    printCurrentPosition();
    
    // Get target angle from user in degrees
    float targetAngle = 0;
    std::cout << "Enter the target angle in degrees: ";
    std::cin >> targetAngle;

    // Convert target angle to motor position value
    long targetPosition = convertAngleToPosition(targetAngle);
    
    BOOL absolute = TRUE;
    BOOL immediately = TRUE;

    success = VCS_MoveToPosition(deviceHandle, nodeID1, targetPosition, absolute, immediately, &errorCode);

    if (success) {
        std::cout << "Moving to target position.\n" << std::endl;
    }
    else {
        std::cerr << "Failed to move to target position, error code: " << errorCode << std::endl;
    }

    // Wait for the motor to reach the target position
    DWORD timeout = 10000; // Timeout in milliseconds, adjust as needed
    success = VCS_WaitForTargetReached(deviceHandle, nodeID1, timeout, &errorCode);

    if (success) {
        std::cout << "Target position reached." << std::endl;
    }
    else {
        std::cerr << "Failed to reach target position, error code: " << errorCode << std::endl;
    }

    printCurrentPosition();
}

void DeviceController::handleProfileVelocityMode() {
    DWORD errorCode = 0;

    
    // Activate Profile Velocity Mode
    BOOL success = VCS_ActivateProfileVelocityMode(deviceHandle, nodeID1, &errorCode);

    if (success) {
        std::cout << "\n";
        std::cout << "Profile Velocity Mode activated." << std::endl;
    }
    else {
        std::cerr << "Failed to activate Profile Velocity Mode, error code: " << errorCode << std::endl;
        return;
    }

    // Enable the device
    BOOL enabled = VCS_SetEnableState(deviceHandle, nodeID1, &errorCode);

    if (enabled) {
        std::cout << "Device Enabled" << std::endl;
    }
    else {
        std::cerr << "Failed to Enable Device, error code: " << errorCode << std::endl;
        return;
    }

    // Set the velocity profile
    DWORD profileAcceleration = 1000;
    DWORD profileDeceleration = 10000;

    success = VCS_SetVelocityProfile(deviceHandle, nodeID1, profileAcceleration, profileDeceleration, &errorCode);

    if (success) {
        std::cout << "Velocity profile set." << std::endl;
    }
    else {
        std::cerr << "Failed to set velocity profile, error code: " << errorCode << std::endl;
        return;
    }

    // Get the velocity profile
    success = VCS_GetVelocityProfile(deviceHandle, nodeID1, NULL, NULL, &errorCode);

    if (success) {
        std::cout << "Velocity profile retrieved.\n\n" << std::endl;
    }
    else {
        std::cerr << "Failed to retrieve velocity profile, error code: " << errorCode << std::endl;
        return;
    }

    // Get target velocity from user in rpm
    float targetVelocity = 0;
    std::cout << "Enter the target velocity in rpm: ";
    while (!(std::cin >> targetVelocity)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cerr << "Invalid input. Please enter a valid velocity: ";
    }


    // Convert target velocity to motor velocity value
    long targetMotorVelocity = static_cast<long>((targetVelocity / 60) * encoderCountsPerRevolution * gearRatio);

    // Move motor with target velocity
    success = VCS_MoveWithVelocity(deviceHandle, nodeID1, targetMotorVelocity, &errorCode);

    if (success) {
        std::cout << "Moving with target velocity.\n" << std::endl;

        // Wait for the motor to move for a sufficient amount of time
        DWORD waitTime = 5000; // Wait for 5 seconds, adjust as needed
        Sleep(waitTime);

        // Stop the motor
        success = VCS_HaltVelocityMovement(deviceHandle, nodeID1, &errorCode);

        if (success) {
            std::cout << "Movement stopped." << std::endl;
        }
        else {
            std::cerr << "Failed to stop movement, error code: " << errorCode << std::endl;
            return;
        }
    }
    else {
        std::cerr << "Failed to move with target velocity, error code: " << errorCode << std::endl;
        return;
    }

}


