#include "fdml/locator.hpp"

using namespace FDML;

int main() {
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
}
