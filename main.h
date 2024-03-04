#ifndef MAIN_HPP
#define MAIN_HPP

#include "include/Screen.h"
#include "include/MQTT++.h"
#include "include/CV++.h"
#include <pigpio.h>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <csignal>
#include <fstream>
#include <chrono>
#include <thread>

#define PWM_PIN 13

using ullint = unsigned long long int;

using namespace std;
using namespace mqtt;
using namespace cv;

VideoCapture videoCapture(0);

bool setup();
bool loop();
void teardown();
void teardown(int signal);

ullint i(0);
int main(int argc, char *argv[]) {
    // run setup, if failure return
    if (!setup())
    {
        cerr << "Error in setup" << endl;
        return(-1);
    }

    // run loop until loop() returns false
    while (loop(), ++i) {
        // cout << "Frame: " << i << endl;
    }

    return 0;
}

#endif // MAIN_HPP
