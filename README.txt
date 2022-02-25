
====== Setup ======
	=== CPP Linux ===
		== dependencies: (TODO CGAL, boost package, gmp, gcc, ect.)
			mkdir -i lib/; cd lib; wget -O boost_1_78_0.tar.gz https://sourceforge.net/projects/boost/files/boost/1.78.0/boost_1_78_0.tar.gz/download; tar xzvf boost_1_78_0.tar.gz; rm boost_1_78_0.tar.gz; cd ../
		== compilation setup:
			mkdir -p build/linux/debug/; cd build/linux/debug/; cmake -DCMAKE_BUILD_TYPE=Debug ../../../; cd ../../../
			mkdir -p build/linux/release/; cd build/linux/release/; cmake -DCMAKE_BUILD_TYPE=Release ../../../; cd ../../../
		== compilation:
			make -C build/linux/release
		== execution: ( TODO )
			build/release/robo_local_daemon

	=== CPP Windows ===
		== dependencies:
			https://doc.cgal.org/latest/Manual/windows.html
				git clone https://github.com/microsoft/vcpkg
				cd vcpkg; .\bootstrap-vcpkg.bat
				.\vcpkg.exe install yasm-tool:x86-windows
				.\vcpkg.exe install cgal
				.\vcpkg.exe install boost-system
				.\vcpkg.exe install boost-json
				.\vcpkg.exe install boost-program-options
			mkdir -i lib/; cd lib; wget -O boost_1_78_0.tar.gz https://sourceforge.net/projects/boost/files/boost/1.78.0/boost_1_78_0.tar.gz/download; tar xzvf boost_1_78_0.tar.gz; rm boost_1_78_0.tar.gz; cd ../
		== compilation setup:
			mkdir build/win/
			cmake-gui
				choose source code folder: {ROOT}/
				choose build folder: {ROOT}/build/win
				specify the Generator (e.g., Visual Studio 16 2019),
				specify the Optional Platform: win32
				select Specify toolchain file for cross compilation (the file vcpkg.cmake within the directory where you have installed vcpkg, e.g. C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake).

	== CMakeLists creation (internal):
		cd src/cpp; /usr/bin/cgal_create_CMakeLists -s robo_local_daemon; cd ../../

	=== Python ===
		install pip install PyQt5
