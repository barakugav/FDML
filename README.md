
# FDML: Few Distance Measurements robot Localization

<div align="center">
<img src="https://github.com/barakugav/fdml/blob/master/doc/logo/logo_horizontal.png?raw=true" alt="drawing" width="600"/>
</div>

FDML is a CPP software for robot localization processing and queries using few (one or two) distance measurements.

Imagine the following situation. A robot is placed in an unknown position and unknown orientation in a known polygon (with holes) environment. The robot has a depth sensor, that is a sensor that can measure the scalar length of a ray from it's position to a wall in a specific direction. The depth sensor is used to measure a single distance measurement with the robot unknown position and orientation, where does the robot can be within the environment given the new information? Same question can be asked when the robot is allowed to make a second distance measurement.

The FDML library support these types of queries. For two distance measurements, the library support two antipodal (180 &deg;) measurements, in other words, the robot measure a distance measurement in some orientation, then rotate in place to the exact opposite direction and measure a second distance measurement. A polygon environment can be preprocessed and support multiple queries efficiently.

The library is written in CPP, but bindings exists for Python, and an additional GUI application.

## FDML-core

The FDML-core is the CPP heart of the library and contains all the logic. It can be used as a CPP library, from Python using the bindings, or using command line application (basic CLI, daemon with communication through files). It's built on top of [CGAL](https://www.cgal.org/), which depends on [boost](https://www.boost.org/), [gmp](https://gmplib.org/) and [mpfr](https://www.mpfr.org/).

### Installation

- Clone the repository:
	`git clone https://github.com/barakugav/FDML.git`
- Install all the dependencies (see dependencies section) and set environment variables for `cmake`:
	- TODO
- Build FDML
	- Linux
		```bash
		cd {FDML_BUILD_DIR}
		cmake -DBUILD_SHARED_LIBS:BOOL=ON -DCMAKE_BUILD_TYPE=Release {FDML_SRC_DIR}
		make -j
		```
		If the Python bindings are required, replace the `cmake` command with the following:
		```bash
		cmake -DBUILD_SHARED_LIBS:BOOL=ON -DFDML_WITH_PYBINDINGS:BOOL=ON -DCMAKE_BUILD_TYPE=Release {FDML_SRC_DIR}
		```
		If you need a statically linked library, replace the `cmake` command with the following:
		```bash
		cmake -DBUILD_SHARED_LIBS:BOOL=OFF -DFDML_USE_STATIC_LIBS:BOOL=ON -DFDML_WITH_PYBINDINGS:BOOL=ON -DCMAKE_BUILD_TYPE=Release {FDML_SRC_DIR}
		```
	- Windows
		The configuration is doe via [cmake-gui](https://cmake.org/cmake/help/latest/manual/cmake-gui.1.html)
		TODO
		Compile using Visual Studio. open /build/fdml.sln, "Build -> Build Solution".

### Usage

```cpp
Polygon_with_holes scene;
scene.push_back(Point(0, 0));
scene.push_back(Point(0, 10));
scene.push_back(Point(10, 10));
scene.push_back(Point(10, 0));

/* locator is initiated with a polygon scene */
Locator locator;
locator.init(scene);

/* Assuming a robot is within the given scene, in an unknown location and an
 * unknown orientation, after performing a single distance measurement, the
 * locator can be used to calculate all the possible location the robot
 * might be in.
 * Queries of a single measurement result in a collection of areas representing
 * all the possible locations of the robot, each represented as polygon and the
 * measured edge.
 */
double robot_measurement = 5.7;

std::vector<Res1d> single_res = locator.query(robot_measurement);
std::count << "Single measurement possible positions:" << std::endl;
for (const auto& res_area : single_res)
  std::count << "\tPossible positions of measuring edge (" << res_area.edge << "): " << res_area.pos << std::endl;

/* If the robot is able to perform a second distance measurement in the
 * opposite direction of the first measurement, the locator can be used to
 * calculate a more accurate localization result using the two measurements.
 * Queries of double measurements result in a collection of segments
 * representing all the possible locations of the robot.
 */
double second_robot_measurement = 2.4;
std::vector<Res2d> double_res = locator.query(robot_measurement, second_robot_measurement);
std::count << "Double measurements possible positions:" << std::endl;
for (const auto& res_area : double_res)
  std::count << "\tPossible positions of measuring edges (" << res_area.edge1 << "),(" << res_area.edge2
             << "): " << res_area.pos << std::endl;
```

## FDML-GUI

### Installation

FDML-core is required, see FDML-core section.
In the future, FDML-py, the Python bindings, will be required as well.

Install python dependencies:
`pip install -r ./fdml-gui/requirements.txt`

### Usage

TODO

```bash
python fdml-gui/fdmlgui/scene_designer.py
python fdml-gui/fdmlgui/locator_gui.py
```



## Dependencies

The dependencies can be installed globally on the machine, by defining the `cmake` variables as environment variables. Alternatively, one can create a directory `fdml/libs` add download all dependencies to it (this is also the default of *get_boost.py*), and define the `cmake` variable before building FDML.

### Python

Python3 is used as the main scripting language of the library, the GUI application and the library Python bindings. Not all of the following are required for some uses of the library, dependencies  can be installed the only for the used module.

- Install [python](https://www.python.org/) and [pip](https://pypi.org/project/pip/):

	- Linux:
		`sudo apt-install python3 python3-pip`

	- Windows:
	Download python from the [official website](https://www.python.org/downloads/) and install pip:
	`python -m ensurepip`

- Scripts dependencies:
	`pip install -r ./scripts/requirements.txt`

- GUI dependencies:
	`pip install -r ./fdml-gui/requirements.txt`

- Python bindings dependencies:
	For Python bindings, [boost-python](https://www.boost.org/doc/libs/1_70_0/libs/python/doc/html/index.html) and [boost-numpy](https://www.boost.org/doc/libs/1_64_0/libs/python/doc/html/numpy/index.html) are required, and boost should be built with some special flags, more on that in the boost section. Before building boost, [numpy](https://numpy.org/) installation is needed:
		`pip install numpy`
	In addition, install additional python packages
	`pip install -r ./fdml-py/requirements.txt`


### Installing A Development Environment and  `cmake`

A compiler and `cmake` should be installed.
- Linux:
	```bash
	sudo apt-get install build-essential checkinstall g++ cmake
	```
- Windows
	Install MSVC from the [official website]()

### Boost

Boost is a dependency of CGAL and FDML-core, and it's also used to generate the Python bindings. If the Python bindings are required, python (3.8+) should be installed, along with numpy before boost is built.

To install boost, you have a few options:
1. Use the get_boost.py script in fdml/scripts
	- Linux:
	```bash
	./scripts/get_boost.py --cmd download --boost-top ./libs/boost
	./scripts/get_boost.py --cmd build --boost-top ./libs/boost --python /usr/bin/python3.8
	```
	- Windows
	```bash
	.\scripts\get_boost.py --cmd download --boost-top .\libs\boost\
	.\scripts\get_boost.py --cmd build --boost-top .\libs\boost\ --python C:\programs\Python39\python.exe
	```

	The path to the Python executable **should not contains spaces**! Boost configuration doesn't handle these space well. This is a usually mistake in Windows, as Python is usually installed at `C:\Program Files\Python\`.
	If the Python bindings are not required, `--python {Python executable path}` should be omitted.

2. Download and install boost manually
	- Download boost archive and extract it
		- Linux:
		```bash
		wget -O boost_1_79_0.tar.gz https://sourceforge.net/projects/boost/files/boost/1.79.0/boost_1_79_0.tar.gz/download
		tar xzvf boost_1_79_0.tar.gz
		rm boost_1_79_0.tar.gz
		```

		- Windows:
		```bash
		curl -o boost_1_79_0.zip https://boostorg.jfrog.io/artifactory/main/release/1.79.0/source/boost_1_79_0.zip
		Expand-Archive -LiteralPath boost_1_79_0.zip -DestinationPath boost_1_79_0
		rm boost_1_79_0.zip
		```

	- If Python bindings are required, create a configuration file *user-config.jam* which will tell boost where to find Python. The file should contain the following (spaces matter!):
		```bash
		import toolset ;

		# using python : {ver_maj}.{ver_min} : "{path to python executable}" ;
		using python : 3.9 : "C:\\programs\\Python\\Python39\\python.exe" ;
		```
		The above example is for Windows, but should be identical for Linux except the path to the Python executable.
- Build boost:
	Run the build commands from the newly downloaded boost source directory.
	- Linux:
		```bash
		./bootstrap.sh --with-python={Python executable path} --with-python-version={ver_maj}.{ver_min}
		./b2 --user-config={path to user-config.jam} --build-dir=./build --stagedir=./bin architecture=x86 address-model=64 link=static,shared --variant=debug,release --debug-configuration
		```
	- Windows:
		```bash
		./bootstrap.bat--with-python={Python executable path} --with-python-version={ver_maj}.{ver_min}
		./b2 --user-config={path to user-config.jam} --build-dir=./build --stagedir=./bin architecture=x86 address-model=64 link=static,shared runtime-link=static,shared --variant=debug,release --debug-configuration
		```


### gmp
TODO

### mpfr
TODO

### CGAL
CGAL is a header only library, therefore there is no to do anything except cloning the repository
```bash
git clone https://github.com/CGAL/cgal.git
```
The `cmake` variable required for CGAL is `CGAL_DIR`, which should point to the root of the CGAL repository.