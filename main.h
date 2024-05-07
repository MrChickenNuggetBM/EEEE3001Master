#ifndef MAIN_HPP
#define MAIN_HPP

#define PWM_PIN 18

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

Screen screen("/dev/fb1");

int  sWidth = 3840; // screen.getWidth ();
int sHeight = 2400; // screen.getHeight();

using namespace std;
using namespace mqtt;
using namespace cv;

namespace mqtt
{
// defining useful constants
// the topics to subscribe to on the MQTT broker
const string TOPICS[] =
{
    "parameters/xCenter",
    "parameters/yCenter",
    "parameters/xDiameter",
    "parameters/yDiameter",
    "parameters/thickness",
    "parameters/isCircle",
    "parameters/modality",
    "parameters/angle",
    "parameters/isGUIControl",
    "brightness/isAutomaticBrightness",
    "brightness/dutyCycle",
    "cv/x-correction",
    "cv/y-correction",
    "cv/angle-correction",
    "cv/minRad-correction",
    "cv/majRad-correction",
    "cv/isNewValues",
    "cv/isPauseRendering"
};
const int numTopics = sizeof(TOPICS)/sizeof(string);

// mqtt broker definition
const string SERVER_ADDRESS("mqtt://192.168.2.1:1883");
async_client CLIENT(SERVER_ADDRESS, "Master");
// connection OPTIONS
connect_options OPTIONS;
// callback
Callback CALLBACK(CLIENT, OPTIONS, TOPICS, numTopics);
}

// stores the loop number
using ullint = unsigned long long int;

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
