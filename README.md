# Meniscus Distortion Correction Project
This GitHub repository stores files for the Master only, relating to the Meniscus Distortion project. This project is designed to work on the Raspbian operating system.

To clone into this repository, write the `git clone https://github.com/MrChickenNuggetBM/EEEE3001Master.git` command into the terminal.

To ensure the shared submodule is included, write the `git update submodule --init --recursive` command into the terminal.

Move into the `Shared` directory and create a build directory and move into it:

```
cd Shared
mkdir build
cd build
```

Then, to build the project and run it:

```
cmake ..
make
sudo bin/MeniscusDistortion
```
