[![Build](https://github.com/barakugav/FDML/actions/workflows/build.yaml/badge.svg)](https://github.com/barakugav/FDML/actions/workflows/build.yaml)

# FDML: Few Distance Measurements robot Localization

<div align="center">
<img src="https://github.com/barakugav/fdml/blob/master/ect/doc/logo/logo_horizontal.png?raw=true" alt="drawing" width="600"/>
</div>

`FDML` is a software for robot localization processing and queries using few (one or two) distance measurements, written in `C++`.

Imagine the following situation. A robot is placed in an unknown position and unknown orientation in a known polygon (with holes) environment. The robot has a depth sensor, that is a sensor that can measure the scalar length of a ray from it's position to a wall in a specific direction. The depth sensor is used to measure a single distance measurement with the robot unknown position and orientation, where does the robot can be within the environment given the new information? Same question can be asked when the robot is allowed to make a second distance measurement.

The `FDML` library support these types of queries. For two distance measurements, the library support two antipodal (180 &deg;) measurements, in other words, the robot measure a distance measurement in some orientation, then rotate in place to the exact opposite direction and measure a second distance measurement. A polygon environment can be preprocessed and support multiple queries efficiently.

The library is written in `C++`, but bindings exists for `Python`, and an additional `GUI` application.

# FDML-core

The `FDML-core` is the `C++` heart of the library and contains all the logic. It can be used as a `C++` library, from `Python` using the bindings, or using command line application (basic `CLI`, daemon with communication through files). It's built on top of [CGAL](https://www.cgal.org/), which depends on [boost](https://www.boost.org/), [gmp](https://gmplib.org/) and [mpfr](https://www.mpfr.org/).


## Usage

The library interface is done via `Locator` object which support three functions:

```cpp
class Locator {
  Locator();
  void init(const Polygon_with_holes& scene);
  std::vector<Res1d> query(const Kernel::FT& d) const;
  std::vector<Res2d> query(const Kernel::FT& d1, const Kernel::FT& d2) const;
}
```
- `void init(scene)`
	Perform preprocessing on the given polygon scene
- `vector<Res> query(d)`
	Calculates all the positions in the scene a sensor might be, given the information it performed a single distance measurement with a value of `d`.
- `vector<Res> query(d1, d2)`
	Calculates all the positions in the scene a sensor might be, given the information it performed two distance measurements with a values of `d1,d2`.

A simple square scene example:
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


# Installation

The dependencies of the library are [Boost](https://www.boost.org/), [GMP](https://gmplib.org/), [MPFR](https://www.mpfr.org/) and [CGAL](https://www.cgal.org/).
If Python bindings are built, also [nanobind](https://github.com/wjakob/nanobind)  is required.

## Windows

In this guide we will clone the `FDML` repo into `C:\fdml\`, install the dependencies into `C:\fdml_deps\` and build `FDML` atrifacts into `C:\fdml_build\`. You should change these paths according to your needs.

- Install `cmake` and `MSVC`

- Install `GMP` and `MPFR`:
	Download and unzip the binaries from [this link](https://github.com/CGAL/cgal/releases/download/v5.4/CGAL-5.4-win64-auxiliary-libraries-gmp-mpfr.zip) into `C:\fdml_deps\gmp_mpfr`, and set `GMP` and `MPFR` environment variables:
	```
	$env:GMP_DIR="C:\fdml_deps\gmp_mpfr"
	$env:MPFR_DIR="C:\fdml_deps\gmp_mpfr"
	```

- Install `boost` via [installer](https://sourceforge.net/projects/boost/files/boost-binaries/) into `C:\fdml_deps\boost_1_82_0` and set its environment variables (replace `1_82_0` and `14.3` if you are using different versions):
	```
	$env:BOOST_INCLUDEDIR="C:\fdml_deps\boost_1_82_0"
	$env:BOOST_LIBRARYDIR="C:\fdml_deps\boost_1_82_0\lib64-msvc-14.3"
	```

- Install `CGAL` and set its environment variables:
	```
	git clone https://github.com/CGAL/cgal.git C:\fdml_deps\cgal
	mkdir C:\fdml_deps\cgal\build
	cd C:\fdml_deps\cgal\build
	cmake -A x64 ..
	$env:CGAL_DIR="C:\fdml_deps\cgal\build"
	```

- Clone the `FDML` repository:
	```
	git clone https://github.com/barakugav/FDML.git C:\fdml
	```

- Build using `cmake`:
	```
	mkdir C:\fdml_build\
	cd C:\fdml_build\
	cmake -A x64 -DBUILD_SHARED_LIBS:BOOL=ON -DCMAKE_BUILD_TYPE=Release -DFDML_WITH_PYBINDINGS:BOOL=OFF C:\fdml\
	cmake --build . -j
	```
#### Building with Python bindings
The installation follow the same steps as above, with addition dependencies installation before building via `cmake`:
- Install `nanobind`
	```
	git clone https://github.com/wjakob/nanobind.git C:\fdml_deps\nanobind
	cd C:\fdml_deps\nanobind
	git submodule update --init
	$env:nanobind_DIR="C:\fdml_deps\nanobind\"
	```

- install `Python` dependencies
	```
	pip install -r C:\fdml\fdmlpy\requirements.txt
	```

When running `cmake`, pass `-DFDML_WITH_PYBINDINGS:BOOL=ON`, and than build normally via `cmake --build .`.


## Linux

In this guide we will clone the `FDML` repo into `~/fdml/`, install the dependencies into `~/fdml_deps/` and build `FDML` atrifacts into `~/fdml_build/`. You should change these paths according to your needs.

- Install `C++` build tools:
	```bash
	sudo apt-get install build-essential checkinstall m4 g++ cmake
	```

- Install `GMP` and `MPFR`:
	```bash
	sudo apt-get install libgmp3-dev
	sudo apt-get install libmpfr-dev libmpfr-doc
	```

- Install `boost`:
Unfortenetly, the `boost` version installed through `apt-get` is currently `1.71.0` and we require version `1.75.0` and above. Therefore, we need to download and build `boost` manually:
	```
	cd ~/fdml_deps/
	wget -O boost_1_79_0.tar.gz https://sourceforge.net/projects/boost/files/boost/1.79.0/boost_1_79_0.tar.gz/download
	tar xzf boost_1_79_0.tar.gz
	rm boost_1_79_0.tar.gz
	cd boost_1_79_0
	./bootstrap.sh
	./b2 --build-dir=./build --stagedir=./bin architecture=x86 address-model=64 link=static,shared --variant=debug,release --without-python
	export BOOST_INCLUDEDIR="~/fdml_deps/boost_1_79_0"
	export BOOST_LIBRARYDIR="~/fdml_deps/boost_1_79_0/bin/lib"
	```

- Install `CGAL` and set its environment variables:
	```
	git clone https://github.com/CGAL/cgal.git ~/fdml_deps/cgal
	mkdir ~/fdml_deps/cgal/build
	cd ~/fdml_deps/cgal/build
	cmake ..
	export CGAL_DIR="~/fdml_deps/cgal/build"
	```

- Clone the `FDML` repository:
	```
	git clone https://github.com/barakugav/FDML.git ~/fdml
	```

- Build using `cmake` and `make`:
	```
	mkdir ~/fdml_build/
	cd ~/fdml_build/
	cmake -DBUILD_SHARED_LIBS:BOOL=ON -DCMAKE_BUILD_TYPE=Release -DFDML_WITH_PYBINDINGS:BOOL=OFF ~/fdml/
	make -j
	```

#### Building with Python bindings
The installation follow the same steps as above, with addition dependencies installation before the the building using `cmake`:
- Install `nanobind`
	```
	git clone https://github.com/wjakob/nanobind.git ~/fdml_deps/nanobind
	cd ~/fdml_deps/nanobind
	git submodule update --init
	export nanobind_DIR="~/fdml_deps/nanobind/"
	```

- install `Python` dependencies
	```
	pip install -r ~/fdml/fdmlpy/requirements.txt
	```

When running `cmake`, pass `-DFDML_WITH_PYBINDINGS:BOOL=ON`, and than build normally via `make -j`.



# FDML-GUI

## Installation

FDML-core is required, see FDML-core section.
In the future, FDML-py, the Python bindings, will be required as well.

Install python dependencies:
`pip install -r ./fdml-gui/requirements.txt`

## Usage

TODO

```bash
python fdml-gui/fdmlgui/scene_designer.py
python fdml-gui/fdmlgui/locator_gui.py
```
