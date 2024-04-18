#ifndef SCREEN_HPP
#define SCREEN_HPP

#include <iostream>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <stdexcept>

class Screen
{
private:
    const char* fbPath;
    int width, height;

public:
    // constructors
    Screen(const char* path = "/dev/fb0");
    Screen(const Screen& copiedScreen);

    // destructor
    ~Screen();

    // operator overloads
    Screen& operator=(const Screen& assignedScreen);

    // get the resolution of the specified frame buffer
    void getResolution();
    int getWidth() const {return width;}
    int getHeight() const {return height;}

    // send image data to the screen
    void send(cv::Mat& image) const;
    void sendSlow(cv::Mat& image) const;
};

#endif // SCREEN_HPP
