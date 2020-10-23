# Manipura_CSX
 A 7x7 touchpad matrix using CSX mutual capacitive sensing with Capsense. The embedded IC on the touchpad is a PSOC 4: CY8C4125AZI-S433 

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