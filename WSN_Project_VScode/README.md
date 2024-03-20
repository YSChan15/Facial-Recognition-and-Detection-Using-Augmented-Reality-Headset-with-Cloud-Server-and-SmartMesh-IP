# SCSU Senior Design GCC Project
This project utilizes the GCC ARM compiler to generate binaries and OpenOCD to facilitate SAML21 microcontroller debugging. CMAKE is utilized to set up the makefile and configure the build process. The application-level source files are located in the 'src' directory, while the RTOS library source files are contained in the 'FreeRTOS' directory. Low-level register includes are located in the 'CMSIS' directory, and the device 'directory' houses the necessary startup files, linker files, and SVD files required for linking and debugging.

## Windows Setup
In order for the project to be built, the following tools must be installed on Windows.

- ARM GCC Compiler - Download and install from [ARM's website](https://developer.arm.com/downloads/-/gnu-rm).
- CMake - Download and install from [CMake's website](https://cmake.org/download/).
- Chocolatey - Follow the instructions on [Chocolatey's website](https://chocolatey.org/install) to install Chocolatey.
- OpenOCD - Run the following command in an elevated PowerShell or command prompt: 
```console
        choco install openocd
```
- VSCode - Download Visual Studio Code from the official [VSCode website](https://code.visualstudio.com/) and install it.
- STMCubeIDE - Download and install from [STMicroelectronics website](https://www.st.com/en/development-tools/stm32cubeide.html).

### Step 1: VSCode Extension Setup
- Open the VSCode application.
- To open a new terminal within the application, go to Terminal > New Terminal in the VSCode menu.

### Step 2: Configure the build process
- Open the project folder in VSCode.
- In the left-hand menu, click on the Explorer icon and navigate to the project folder.
- Within the project folder, locate the .vscode folder.
- Within the .vscode folder, locate the tasks.json file and open it in the VSCode editor.
- In the tasks.json file, you can configure the build process for your system. This file contains tasks for building, cleaning, and other tasks related to the project build process. Make sure the tasks are set up correctly for your system.

### Step 3: Configure the debug process
- Open the project folder in VSCode.
- In the left-hand menu, click on the Explorer icon and navigate to the project folder.
- Within the project folder, locate the .vscode folder.
- Within the .vscode folder, locate the launch.json file and open it in the VSCode editor.
- In the launch.json file, you can configure the debug process for your system. This file contains configurations for debugging the project. Make sure the configurations are set up correctly for your system.
- Find the "pre debug" task and look for the command field. You have to make sure the path to the stlink is correct. This is usually set up in the launch.json file.

## MacOS Setup
In order for the project to be built, the following tools must be installed on Mac.

- Homebrew - Open terminal and run the following command: 
```console
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
```
- ARM GCC Compiler - Run:
```console
        brew install gcc-arm-none-eabi
```
- CMake - Run:
```console
        brew install cmake
```
- OpenOCD - Run:
```console
        brew install openocd
```
- VSCode - Download Visual Studio Code from the official [VSCode Website](https://code.visualstudio.com/) and install it on your Mac.
- STMCubeIDE - Download and install from [STMicroelectronics website](https://www.st.com/en/development-tools/stm32cubeide.html).

### Step 1: VSCode Extension Setup

- Open the VSCode application.
- To open a new terminal within the application, go to Terminal > New Terminal in the VSCode menu.

### Step 2: Configure the build process

- Open the project folder in VSCode.
- In the left-hand menu, click on the Explorer icon and navigate to the project folder.
- Within the project folder, locate the .vscode folder.
- Within the .vscode folder, locate the tasks.json file and open it in the VSCode editor.
- In the tasks.json file, you can configure the build process for your system. This file contains tasks for building, cleaning, and other tasks related to the project build process. Make sure the tasks are set up correctly for your system.

### Step 3: Configure the debug process

- Open the project folder in VSCode.
- In the left-hand menu, click on the Explorer icon and navigate to the project folder.
- Within the project folder, locate the .vscode folder.
- Within the .vscode folder, locate the launch.json file and open it in the VSCode editor.
- In the launch.json file, you can configure the debug process for your system. This file contains configurations for debugging the project. Make sure the configurations are set up correctly for your system.
- Find the "pre debug" task and look for the command field. You have to make sure the path to the stlink is correct. This is usually set up in the launch.json file.

## Current Issues Encountered to be Fixed
- Currently, there is no SVD file specifically for the SAML21 so the reset function within the debugger does not work properly
- There is no good way to start the openocd background debugger task so you need to Ctrl+C it after each debug run
