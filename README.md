# FDML: Few Distance Measurements robot Localization

<div align="center">
<img src="https://github.com/barakugav/fdml/blob/master/doc/logo_horizontal.png?raw=true" alt="drawing" width="600"/>
</div>

FDML is a CPP software for robot localization processing and queries using few (one or two) distance measurements. The project contains few different tools: command line tools at /fdml/bin, GUI tool at /fdml-gui.

## FDML-core
### Installation
#### Linux
Clone the repository, install dependencies (build-essential, cmake, boost and CGAL) and config compilation setup:

```bash
sudo apt-get install git
git clone https://github.com/barakugav/FDML.git; cd fdml/fdml

sudo apt-get install build-essential cmake
sudo apt-get install libboost-system-dev libboost-program-options-dev
sudo apt-get install libcgal-dev
mkdir -p lib/; wget -O boost_1_78_0.tar.gz https://sourceforge.net/projects/boost/files/boost/1.78.0/boost_1_78_0.tar.gz/download; tar xzvf boost_1_78_0.tar.gz --directory lib/; rm boost_1_78_0.tar.gz
```

Compile a command line tool using a specific binary build script (fdml_daemon for example):
```bash
bin/fdml_daemon/build.py
```

#### Windows

Install dependencies (boost ,CGAL) using [vcpkg](https://github.com/microsoft/vcpkg)
```bash
cd 'C:\Program Files\'
git clone https://github.com/microsoft/vcpkg; cd vcpkg; .\bootstrap-vcpkg.bat
.\vcpkg.exe install boost-system boost-json boost-program-options
.\vcpkg.exe install yasm-tool:x86-windows
.\vcpkg.exe install cgal
```

Clone the repository and config compilation setup for a specific command line tool (fdml_daemon for example)
```bash
git clone https://github.com/barakugav/FDML.git; cd fdml/fdml
cd bin/fdml_daemon
mkdir build
cmake-gui
	choose source code folder: bin/fdml_daemon/
	choose build folder: bin/fdml_daemon/build/
	specify the Generator (e.g., Visual Studio 16 2019),
	specify the Optional Platform: win32
	select Specify toolchain file for cross compilation (the file vcpkg.cmake within the directory where you have installed vcpkg, e.g. C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake).
```

Compile using Visual Studio. open /build/fdml.sln, "Build -> Build Solution".

### Usage

```cpp
Polygon scene;
scene.push_back(Point(0, 0));
scene.push_back(Point(0, 10));
scene.push_back(Point(10, 10));
scene.push_back(Point(10, 0));

/* locator is initiated with simple polygon scene */
Locator locator;
locator.init(scene);

/* Assuming a robot is within the given scene, in an unknown location and an
 * unknown orientation, after performing a single distance measurement, the
 * locator can be used to calculate all the possible location the robot
 * might be in.
 * Queries of a single measurement result in a collection of areas representing
 * all the possible locations of the robot, each represented as polygon.
 */
double robot_measurement = 5.7;
std::vector<Polygon> single_res;
locator.query(robot_measurement, single_res);

/* If the robot is able to perform a second distance measurement in the
 * opposite direction of the first measurement, the locator can be used to
 * calculate a more accurate localization result using the two measurements.
 * Queries of double measurements result in a collection of segments
 * representing all the possible locations of the robot.
 */
double second_robot_measurement = 2.4;
std::vector<Segment> double_res;
locator.query(robot_measurement, second_robot_measurement, double_res);
```

## FDML-GUI
### Installation
Install numpy and PyQt5 using [pip](https://pypi.org/project/pip/):
```bash
pip install numpy
pip install PyQt5
```

### Usage

TODO

```bash
python fdml-gui/fdmlgui/scene_designer.py
python fdml-gui/fdmlgui/locator_gui.py
```