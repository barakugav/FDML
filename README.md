# FDML: Few Distance Measurements robot Localization

<div align="center">
<img src="https://github.com/barakugav/fdml/blob/master/ect/doc/logo/logo_horizontal.png?raw=true" alt="drawing" width="600"/>
</div>

FDML is a CPP software for robot localization processing and queries using few (one or two) distance measurements.

Imagine the following situation. A robot is placed in an unknown position and unknown orientation in a known polygon (with holes) environment. The robot has a depth sensor, that is a sensor that can measure the scalar length of a ray from it's position to a wall in a specific direction. The depth sensor is used to measure a single distance measurement with the robot unknown position and orientation, where does the robot can be within the environment given the new information? Same question can be asked when the robot is allowed to make a second distance measurement.

The FDML library support these types of queries. For two distance measurements, the library support two antipodal (180 &deg;) measurements, in other words, the robot measure a distance measurement in some orientation, then rotate in place to the exact opposite direction and measure a second distance measurement. A polygon environment can be preprocessed and support multiple queries efficiently.

The library is written in CPP, but bindings exists for Python, and an additional GUI application.

# FDML-core

The FDML-core is the CPP heart of the library and contains all the logic. It can be used as a CPP library, from Python using the bindings, or using command line application (basic CLI, daemon with communication through files). It's built on top of [CGAL](https://www.cgal.org/), which depends on [boost](https://www.boost.org/), [gmp](https://gmplib.org/) and [mpfr](https://www.mpfr.org/).

## Installation

- Clone the repository:
	`git clone https://github.com/barakugav/FDML.git`
- Install all the dependencies and create `config.json` file (see dependencies section), and run the `configure` script:
	`./configure --config ./config.json`
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
		Initialize the build directory using [cmake-gui](https://cmake.org/cmake/help/latest/manual/cmake-gui.1.html):
		- Run `cmake-gui` in windows shell, and click `Configure`
		- Verify dependencies path are correct
		- Set `FDML_WITH_PYBINDINGS` if Python bindings are needed
		- Click `Generate`

		Open `{BUILD_DIR}/fdml.sln` with Visual Studio, "Build -> Build Solution".

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



# Dependencies

The dependencies of the library are [Boost](https://www.boost.org/), [GMP](https://gmplib.org/), [MPFR](https://www.mpfr.org/) and [CGAL](https://www.cgal.org/), along with some use of [Python3](https://www.python.org/).
This section provide a guide on how to install all of these. The dependencies can be installed globally on the machine, or alternatively, one can create a directory `fdml/libs` for the dependencies, if there are multiple versions of these dependencies on the machine. The scripts *get_boost.py*, *get_mpfr.py* and *get_gmp.py* can be used to download these dependencies into the local `libs` directory.

After installing all dependencies, any dependency which was not installed globally, should be specified in a *config.json* file:
```json
{
	"dependencies": {
		"boost_inc": "/home/user/code/fdml/libs/boost/boost_1_79_0/",
		"boost_lib": "/home/user/code/fdml/libs/boost/boost_1_79_0_bin/lib",
		"cgal_dir": "/home/user/code/fdml/libs/cgal"
	}
}
```

The following configurations are supported:
- `dependencies` which specifies paths to dependencies. The supported keys are:
	- `gmp_inc`, `gmp_lib`, `mpfr_inc`, `mpfr_lib`, `boost_inc`, `boost_lib`, `cgal_dir`

Before initializing the project with `cmake` and building, the `configure` script should be run to read the `config.json` file:
`./configure.py --config ./config.json`
 Any dependencies which are not found in the config file, will be searched globally.

## Python

Python3 is used as the main scripting language of the library, the GUI application and the library Python bindings. Not all of the following are required for some uses of the library, dependencies  can be installed the only for the used module.

- Install [python](https://www.python.org/) and [pip](https://pypi.org/project/pip/):
	- Linux: `sudo apt-install python3 python3-pip`
	- Windows: download Python from the [official website](https://www.python.org/downloads/) and install pip: `python -m ensurepip`

- Scripts dependencies:
	`pip install -r ./scripts/requirements.txt`

- GUI dependencies:
	`pip install -r ./fdml-gui/requirements.txt`

- Python bindings dependencies:
	For Python bindings, [boost-python](https://www.boost.org/doc/libs/1_70_0/libs/python/doc/html/index.html) and [boost-numpy](https://www.boost.org/doc/libs/1_64_0/libs/python/doc/html/numpy/index.html) are required, and boost should be built with some special flags, more on that in the boost section. Before building boost, [numpy](https://numpy.org/) installation is needed:
		`pip install numpy`
	In addition, some additional packages are required:
	```bash
	pip install -r ./fdml-py/requirements.txt
	sudo apt-get install python3.8-venv
	```


## Installing a Development Environment and  `cmake`

A compiler and `cmake` should be installed.
- Linux:
	```bash
	sudo apt-get install build-essential checkinstall m4 g++ cmake
	```
- Windows:
	Install MSVC compiler from the [official website](https://visualstudio.microsoft.com/downloads/).
	Install `cmake` from [here](https://cmake.org/download/).

## Boost

Boost is a dependency of CGAL and FDML-core, and it's also used to generate the Python bindings. If the Python bindings are required, python (3.8+) should be installed, along with numpy before boost is built.

To install boost, you have a few options:
1. Use the *get_boost.py* script in *fdml/scripts*
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

	The path to the Python executable **should not contains spaces**! Boost configuration doesn't handle these space well. This is a usual mistake in Windows, as Python is usually installed at `C:\Program Files\Python\`.
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
		The above example is for Windows, but should be identical for Linux except the Python executable path.
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

		If Python binding are required, to verify `boost-numpy` was installed successfully, you can look for "NumPy enabled" (instead of "NumPy disabled. Reason:...") during the build. Alternatively, you can look for *boost_numpy...dll* in the boost lib directory.

## GMP (GNU Multiple Precision arithmetic library)

- Linux:
	To install [GMP](https://gmplib.org/), you have a few options:
	1. Install via *apt-get*
		```bash
		sudo apt-get install libgmp3-dev
		```
	2. Use the *get_gmp.py* script in *fdml/scripts*
		```bash
		./scripts/get_gmp.py --cmd download --gmp-top ./libs/gmp
		./scripts/get_gmp.py --cmd build --gmp-top ./libs/gmp
		```

	3. Download and install GMP manually
		```bash
		mkdir $FDML_TOP/libs/gmp
		cd $FDML_TOP/libs/gmp
		wget -O gmp-6.2.1.tar.xz https://gmplib.org/download/gmp/gmp-6.2.1.tar.xz
		tar xf gmp-6.2.1.tar.xz
		rm gmp-6.2.1.tar.xz

		cd gmp-6.2.1
		./configure --prefix=$FDML_TOP/libs/gmp/gmp-6.2.1_installed
		make all
		make check
		make install
		```

- Windows:
	The way to build GMP on windows is clumsy, therefore it's easier to download binaries directly. GMP and MPFR can be downloaded together by using the script *get_gmp.py* in *fdml/scripts*:
	```bash
	.\scripts\get_gmp.py --top-dir .\libs\gmp_mpfr
	```
	Alternatively, you can download the binaries manually from [CGAL releases page](https://github.com/CGAL/cgal/releases).


## MPFR (Multiple-Precision Floating-point Reliable library)

- Linux:
	To install [MPFR](https://www.mpfr.org/), you have a few options:
	1. Install via *apt-get*
		```bash
		sudo apt-get install libmpfr-dev libmpfr-doc
		```
	2. Use the *get_mpfr.py* script in *fdml/scripts*
		```bash
		./scripts/get_mpfr.py --cmd download --mpfr-top ./libs/mpfr
		./scripts/get_mpfr.py --cmd build --mpfr-top ./libs/mpfr --gmp-dir $FDML_TOP/libs/gmp/gmp-6.2.1_installed/
		```

	3. Download and install MPFR manually
		```bash
		mkdir $FDML_TOP/libs/mpfr
		cd $FDML_TOP/libs/mpfr
		wget -O mpfr-4.1.0.tar.gz.xz https://www.mpfr.org/mpfr-current/mpfr-4.1.0.tar.gz
		tar xf mpfr-4.1.0.tar.gz
		rm mpfr-4.1.0.tar.gz

		cd mpfr-4.1.0
		./configure --prefix=$FDML_TOP/libs/mpfr/mpfr-4.1.0_installed --with-gmp=$FDML_TOP/libs/gmp/gmp-6.2.1_installed/
		make all
		make check
		make install
		```

- Windows:
	The way to build MPFR on windows is clumsy, therefore it's easier to download binaries directly. GMP and MPFR can be downloaded together. See the GMP section.


## CGAL
CGAL is a header only library, therefore there is no need to do anything except cloning the repository
```bash
cd $FDML_TOP/libs/
git clone https://github.com/CGAL/cgal.git
```
