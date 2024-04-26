# Build instructions
The build process on Ubuntu is also well documented in the `Dockerfile` in the root of the project.

## Prerequisites Ubuntu
Only follow this section if you're building the project on Ubuntu.

### Installing packages
The necessary packages are installed with the following command:
```
sudo apt update
sudo apt install -y git cmake g++ ninja-build pkg-config libprotoc-dev libprotobuf-dev protobuf-compiler protobuf-compiler-grpc libgrpc-dev libgrpc++-dev imagemagick
```

### Building Qt from source
At the time of writing, Qt v6.7.0 is not available through the Ubuntu package manager. Therefore you will need to get this through other means. The easiest approach is Qt's own online installer tool, but this requires a Qt account. If you use this installer tool, you may skip this step. Make sure to remember the directory in which Qt was installed.

This section describes how to install Qt from source. No Qt account required.

Clone the Qt repository and initialize it with the correct sub-modules. To do this, open the terminal in the directory where you want to store build files. Here, we use the user’s home directory.
```
cd ~
git clone -b 6.7.0 https://code.qt.io/qt/qt5.git qtsrc
cd qtsrc
./init-repository --module-subset="qtbase,qtgrpc,qtrepotools"
```
To compile Qt, produce the finished build into a separate folder `qtbuild` by running the following commands. Expect this step to take some time; in our experience this step would take roughly 15 minutes to compile on an AMD Ryzen 7 3700X processor (8 physical cores,  16 threads) and would consume roughly 14GB of memory.
```
cd ~
mkdir qtbuild
cd qtbuild
../qtsrc/configure -developer-build -nomake examples -nomake tests
ninja
```

#### Optional: Disable OpenGL integration
Note: If building the project for an headless environment (such as a test-runner) we need to explicitly disable OpenGL integration in Qt. This is done by supplying the `-no-opengl` flag. I.e change the configure step to the following:
```
../qtsrc/configure -developer-build -nomake examples -nomake tests -no-opengl
```

## Prerequisites Windows
All our terminal commands on Windows assume you're using PowerShell.

### Installing Git
To download our source code, you will need Git installed on your system. Navigate to https://git-scm.com/download/win and follow the installation instructions to install Git for Windows. 

*Note: It is possible to download the repository directly from GitHub as a .zip archive. In that case you may skip this step.*

### Installing MSVC through Visual Studio
This step is to grab the MSVC compiler.

Go to [https://visualstudio.microsoft.com](https://visualstudio.microsoft.com). Go to the bottom left of the page and download "Community 2022". From the installer, begin installing "Visual Studio Community 2022". You will need the module called "Desktop Development with C++" with the following components:
- MSVC v143
- C++ CMake tools for Windows

### Installing Qt (+ CMake and Ninja) through online installer
*Note: Unlike in the instructions for Linux, we were not able to establish any working commands to build the Qt Grpc module on Windows. Listed below are the instructions on how to download Qt through their online installer tool. A (free) Qt account is required.*

Go to [https://www.qt.io/download-open-source](https://www.qt.io/download-open-source) and download the Qt online installer. From here you will download "Qt 6.7.0" with the following components, some are optional based on platform and compiler:
- MSVC 2019 64-bit
- Additional libraries
  - Qt Protobuf and Qt gRPC (TP)
- Developer and Designer Tools
  - CMake 3.27.7
  - Ninja 1.10.2

*Note: This does not install the MSVC compiler itself, but rather the precompiled Qt libraries that are compatible with MSVC.*

### Installing vcpkg and protobuf
The official installation instructions can be found at [https://vcpkg.io/en/getting-started](https://vcpkg.io/en/getting-started). We have written down all commands necessary below, you do not need to follow the link unless you want to customize your installation. For these instructions, we will install vcpkg to the root of the C: drive. Open Windows PowerShell in any directory and run the following commands:
```
cd C:
git clone https://github.com/Microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat
.\vcpkg\vcpkg install protobuf
```
*Note: The final step might take a while.*

## Regarding building on Windows
We encountered issues when building on Windows related to the path-length limitation on Windows. Some of the build-folders, when including auto-generated code by Qt, use a deep hierarchy of nested folders and files.

This can be remedied by running the following command. Remember to run PowerShell as administrator!
```
New-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem" ` -Name "LongPathsEnabled" -Value 1 -PropertyType DWORD -Force
```
Reboot the Windows system before continuing.

## Building the project from command-line
To build the project, start by downloading the project using git.

At this point we need to supply a few parameters to our CMake build script in order to let CMake find the correct dependencies: 
- Set `CMAKE_PREFIX_PATH` to the directory where the Qt library has been installed.
- If on Windows, set `CMAKE_TOOLCHAIN_FILE` to `<vcpkg_install_dir>/scripts/buildsystems/vcpkg.cmake`

Below are two examples, assuming you followed the previous steps in this guide exactly.

Windows example: 
```
cd ~
git clone https://github.com/cecilianor/Qt-thesis.git
cd Qt-thesis
cmake . -B build -G Ninja -DCMAKE_PREFIX_PATH="C:/Qt/6.7.0/msvc2019_64" -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake" -DBUILD_TESTS=ON  
cd build
ninja
```

Ubuntu example:
```
cd ~
git clone https://github.com/cecilianor/Qt-thesis.git
cd Qt-thesis
cmake . -B build -G Ninja -DCMAKE_PREFIX_PATH="~/qtbuild/qtbase" -DBUILD_TESTS=ON  
cd build
ninja
```

## Building with Qt Creator
Open the CMakeLists.txt file in Qt Creator.

If you are using Windows at this point, you will get an error when running the initial CMake configuration step, similar to the one shown below. This is because we need to point CMake to look at the vcpkg toolchain to find dependant libraries. To fix this, navigate to "Projects", then under the Kit "Desktop Qt 6.7.0 MSVC2019 64bit", select the tab "Initial Configuration". Select the field "Additional CMake options" and input the command-line flag:
```
"-DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake"
```
Then press the button "Re-Configure with Initial Parameters".


# Running the tests
Tests can be ran by navigating to the build folder and running `ctest`.

Note: On headless environments we need to set the `QT_QPA_PLATFORM` environment variable to `offscreen`. This step is optional on some systems (such as regular Windows).
```
export QT_QPA_PLATFORM=offscreen
ctest --rerun-failed --output-on-failure
```