# kokopelli-c
Kokopelli partially re-written in C with imgui interface with lua 5.1 binding instead of python

## Gears demo from original python to lua

![alt text](https://github.com/samawati/kokopelli-c/blob/main/screenshots/gears-demo.png?raw=true)

## Alien demo from original python to lua

![alt text](https://github.com/samawati/kokopelli-c/blob/main/screenshots/alien-demo.png?raw=true)

## Castle demo from original python to lua

![alt text](https://github.com/samawati/kokopelli-c/blob/main/screenshots/castle-demo.png?raw=true)


### Todo (hopefully since I am currently done as it is):

- PCB library porting from python to lua
- Fix parallel processing of render tasks
- Implement dynamic rendering so that voxels are optimised for final export and display
- Fix all the remaining bugs.
- Someday change the scripting from lua to tiny-scheme and add generic CAM capability for g-code output to fit custom kinematics and machine configurations.
- Train an LLM to generate CAD designs based on the scripting language (lol)

# To build just CMake everything!

***dependencies to build are opengl, glfw, libpng, lua 5.1 , imgui, imgui-color-text-edit ...etc all found inside CMake script files.***

once built run ***frep <filename.lua>*** to execute existing script or just run ***frep*** without any argumnent to open a blank screen.

Use, update, share and enjoy! (as per the original authors license), mostly to learn CSG and CAD/CAM developpment.
