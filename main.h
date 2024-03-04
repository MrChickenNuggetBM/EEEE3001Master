#ifndef MAIN_HPP
#define MAIN_HPP

#define PWM_PIN 13

#include <pigpio.h>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <csignal>
#include <fstream>
#include <chrono>
#include <thread>
#include "include/Screen.h"
#include "Shared/include/CV++.h"

namespace mqtt
{
    // defining useful constants
    const string TOPICS[] = {
        "cv/threshold",
        "cv/noiseKernel",
        "cv/adaptiveSize"};

    // mqtt broker definition
    const string SERVER_ADDRESS("mqtt://192.168.2.1:1883");
    async_client CLIENT(SERVER_ADDRESS, "raspberrypi2");
    // connection OPTIONS
    connect_options OPTIONS;
    // callback
    Callback CALLBACK(CLIENT, OPTIONS, TOPICS, 3);
}

using ullint = unsigned long long int;

using namespace std;
using namespace mqtt;
using namespace cv;

VideoCapture videoCapture(0);

bool setup();
bool loop();
void teardown();
void teardown(int signal)
{
    exit(EXIT_SUCCESS);
}

ullint i(0);
int main(int argc, char *argv[])
{
    // run setup, if failure return
    if (!setup())
    {
        cerr << "Error in setup" << endl;
        return (-1);
    }

    // run loop until loop() returns false
    while (loop(), ++i)
    {
        // cout << "Frame: " << i << endl;
    }

    return 0;
}

#endif // MAIN_HPP
