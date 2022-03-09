# FDML: Few Distance Measurements robot Localization

<div align="center">
<img src="https://github.com/barakugav/fdml/blob/master/doc/logo_horizontal.png?raw=true" alt="drawing" width="600"/>
</div>

FDML is a CPP software for robot localization processing and queries using few (one or two) distance measurements.

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

mkdir -p build/linux/debug/; cmake -DCMAKE_BUILD_TYPE=Debug -Bbuild/linux/debug/ -S./
mkdir -p build/linux/release/; cmake -DCMAKE_BUILD_TYPE=Release -Bbuild/linux/release/ -S./
```
Compile using make:
```bash
make -C build/linux/release
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

Clone the repository and config compilation setup
```bash
git clone https://github.com/barakugav/FDML.git; cd fdml/fdml
mkdir build/win/
cmake-gui
	choose source code folder: {ROOT}/
	choose build folder: {ROOT}/build/win
	specify the Generator (e.g., Visual Studio 16 2019),
	specify the Optional Platform: win32
	select Specify toolchain file for cross compilation (the file vcpkg.cmake within the directory where you have installed vcpkg, e.g. C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake).
```

Compile using Visual Studio. open /build/Win/fdml.sln, "Build -> Build Solution".

### Usage

```cpp
Polygon scene;
scene.push_back(Point(0, 0));
scene.push_back(Point(0, 1));
scene.push_back(Point(1, 1));
scene.push_back(Point(1, 0));

/* localizator is initiated with simple polygon scene */
Localizator localizator();
localizator.init(scene);

/* queries of a single measurement result in a collection of areas representing all
 * the possible locations of the robot, each represented as polygon */
double robot_measurement = 5.7;
std::vector<Polygon> res;
localizator.query(robot_measurement, res);
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
python fdml-gui/src/py/scene_designer.py
python fdml-gui/src/py/localizator_gui.py
```