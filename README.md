# Manipura_CSX
 A 7x7 touchpad matrix using CSX mutual capacitive sensing with Capsense. The embedded IC on the touchpad is a PSOC 4: CY8C4125AZI-S433 

#Requirements and env setup
Install:
- PSOC Programmer
- PSOC Creator

# Option 1: Manipura board connected to the miniprog4 I2C
This method lets you use Capsense tuner directly to evaluate manipura's performance.

# Setup
- Open the Manipura_Stand-alone_firmware PSOC Creator project file (.cyprj) in the repository's root folder
- Build the project (shift+F6)
- Open PSoC Programmer, load the built project's hex file (<Firmware folder>\CortexM0p\ARM_GCC_541\Debug)
- Plug in the miniprog, connected to the Manipura PCB, and power up the device (from the UI button)
- Program manipura using the loaded hex file
- In PSoC Creator, open up the project's topdesign file, lauch capsense tuner
- setup communication according to the EZI2C block's settings
- In Touchpad View tab, check flip Y axis and swap axes, since RX lines are the rows on manipura, but cols in the tuner GUI.

# Option 2: Manipura board interfacing with a PSoC 6 (PSOC 63) devkit
This setup lets you use a Serial monitor like PuTTy to retreive manipura's capsense data through
the PSOC 6's USB-UART bridge (there are no UART lines out on manipura, but will be added in next
version, Sahasrara)

## Setup

###PSoC 63 Slave firmware setup
Right click on the project in PSoC Creator and click on Build Settings.
Go to the ARM GCC compiler tab, and select the General sub tab. add the absolute path to the 
Manipura_Touchpad_Firmware.cydsn folder in the Manipura_CSX repository. This will give access to the
custom header files shared between the two PSoCs, like message.h.
Still in project build settings, select Peripheral Driver library and check Retarget I/O under
utilities. This will generate the stdio_user files in the Shared Files folder upon project build.
Generate and build the application. Then, navigate to stdio_user.h, type in #include "project.h" 
and change the values of both IO_STDOUT_UART
and IO_STDIN_UART for UART_HW (Or the name of your TopDesign's UART block if you decide to change it).