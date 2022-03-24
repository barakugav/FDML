#include "fdml/localizator.hpp"

using namespace FDML;

int main() {

  Polygon scene;
  scene.push_back(Point(0, 0));
  scene.push_back(Point(0, 10));
  scene.push_back(Point(10, 10));
  scene.push_back(Point(10, 0));

  /* localizator is initiated with simple polygon scene */
  Localizator localizator;
  localizator.init(scene);

  /* Assuming a robot is within the given scene, in an unknown location and an
   * unknown orientation, after performing a single distance measurement, the
   * localizator can be used to calculate all the possible location the robot
   * might be in.
   * Queries of a single measurement result in a collection of areas representing
   * all the possible locations of the robot, each represented as polygon.
   */
  double robot_measurement = 5.7;
  std::vector<Polygon> single_res;
  localizator.query(robot_measurement, single_res);

  std::cout << "Single measurement result:" << std::endl;
  for (Polygon &p : single_res)
    std::cout << '\t' << p << std::endl;

  /* If the robot is able to perform a second distance measurement in the
   * opposite direction of the first measurement, the localizator can be used to
   * calculate a more accurate localization result using the two measurements.
   * Queries of double measurements result in a collection of segments
   * representing all the possible locations of the robot.
   */
  double second_robot_measurement = 2.4;
  std::vector<Segment> double_res;
  localizator.query(robot_measurement, second_robot_measurement, double_res);

  std::cout << "Double measurement result:" << std::endl;
  for (Segment &s : double_res)
    std::cout << '\t' << s << std::endl;
}
