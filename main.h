#ifndef MAIN_H
#define MAIN_H

#include "Ellipse/Ellipse.h"
#include <iostream>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

using namespace cv;
using namespace std;

using ullint = unsigned long long int;

int main(int argc, char *argv[]);

#endif // MAIN_H
