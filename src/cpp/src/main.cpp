#include "localizator.h"


int main() {
	std::vector<Point> points = {
		Point(1, 1), Point(3, 4), Point(6, 5), Point(5, 3), Point(6, 1),
	};

	Localizator localizator;
	localizator.init(points);

	std::vector<std::pair<Segment, Polygon>> res_q1;
	localizator.query(40, res_q1);
	Arrangement res_q2;
	localizator.query(30, 31, res_q2);

	DEBUG_PRINT("exit code: 0" << std::endl);
	return 0;
}
