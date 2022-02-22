== CPP setup: (TODO boost package, gmp, gcc, ect.)
mkdir -p build/debug/; cd build/debug/; cmake -DCMAKE_BUILD_TYPE=Debug ../../; cd ../../
mkdir -p build/release/; cd build/release/; cmake -DCMAKE_BUILD_TYPE=Release ../../; cd ../../
mkdir -i lib/; cd lib; wget -O boost_1_78_0.tar.gz https://sourceforge.net/projects/boost/files/boost/1.78.0/boost_1_78_0.tar.gz/download; tar xzvf boost_1_78_0.tar.gz; rm boost_1_78_0.tar.gz; cd ../

== CPP compilation:
make -C build/release

== CPP execution:
build/release/robo_local


== Python setup:
install pip install PyQt5




== internal
cd src/cpp; /usr/bin/cgal_create_CMakeLists -s robo_local; cd ../../
