== CPP setup: (TODO boost package, gmp, gcc, ect.)
cd src/cpp; /usr/bin/cgal_create_CMakeLists -s robo_local; cd ../../
mkdir -p build/debug/; cd build/debug/; cmake -DCMAKE_BUILD_TYPE=Debug ../../src/cpp/; cd ../../
mkdir -p build/release/; cd build/release/; cmake -DCMAKE_BUILD_TYPE=Release ../../src/cpp; cd ../../

== CPP compilation:
make -C build/release

== CPP execution:
build/release/robo_local
