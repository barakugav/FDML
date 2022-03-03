# FDML: Few Distance Measurements robot Localization

FDML is a CPP software for robot localization processing and queries using few (one or two) distance measurements.

## Installation

### Linux
install [CGAL](https://doc.cgal.org/latest/Manual/usage.html)

download boost json dependency:
```bash
mkdir -p lib/; cd lib; wget -O boost_1_78_0.tar.gz https://sourceforge.net/projects/boost/files/boost/1.78.0/boost_1_78_0.tar.gz/download; tar xzvf boost_1_78_0.tar.gz; rm boost_1_78_0.tar.gz; cd ../
```

### Windows

install CGAL, boost using [vcpkg](https://github.com/microsoft/vcpkg)

```bash
git clone https://github.com/microsoft/vcpkg
cd vcpkg; .\bootstrap-vcpkg.bat
.\vcpkg.exe install yasm-tool:x86-windows
.\vcpkg.exe install cgal
.\vcpkg.exe install boost-system
.\vcpkg.exe install boost-json
.\vcpkg.exe install boost-program-options
```

### Python (linux and windows) for GUI
Use the package manager [pip](https://pip.pypa.io/en/stable/) to install the following:
```bash
pip install numpy
pip install PyQt5
```

## Compilation

### Linux

setup:
```bash
mkdir -p build/linux/debug/; cd build/linux/debug/; cmake -DCMAKE_BUILD_TYPE=Debug ../../../; cd ../../../
mkdir -p build/linux/release/; cd build/linux/release/; cmake -DCMAKE_BUILD_TYPE=Release ../../../; cd ../../../
```

compilation:
```bash
make -C build/linux/release
```

### Windows

setup:
```bash
mkdir build/win/
cmake-gui
	choose source code folder: {ROOT}/
	choose build folder: {ROOT}/build/win
	specify the Generator (e.g., Visual Studio 16 2019),
	specify the Optional Platform: win32
	select Specify toolchain file for cross compilation (the file vcpkg.cmake within the directory where you have installed vcpkg, e.g. C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake).
```

compilation:

compile using Visual Studio. open /build/Win/fdml_daemon.sln, "Build -> Build Solution".


## Usage

### CPP

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

### Python GUI

TODO

```bash
python src/py/scene_designer.py
python src/py/localizator_gui.py
```