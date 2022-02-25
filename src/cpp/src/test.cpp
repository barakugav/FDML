#include "localizator.h"
#include "utils.hpp"

// void test_daemon() {
// 	LocalizatorDaemon daemon;

// 	daemon.load_scene("scene01.json");
// 	daemon.load_scene("scene01.json");
// }

void test_localizator() {
	std::vector<Point> points = {
		Point(1, 1), Point(3, 4), Point(6, 5), Point(5, 3), Point(6, 1),
	};

	Localizator localizator;
	localizator.init(points);

	std::vector<Polygon> res_q1;
	localizator.query(40, res_q1);
	Arrangement res_q2;
	localizator.query(30, 31, res_q2);
}

int main() {
	test_localizator();

	debugln("exit code: 0");
	return 0;
}
