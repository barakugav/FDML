#include <vector>

#include "fdml/defs.h"

#ifndef FDML_JSON_UTILS_H
#define FDML_JSON_UTILS_H

namespace FDML {

class JsonUtils {
public:
  /**
   * @brief Parse a scene from a JSON file
   *
   * @param filename path to scene file
   * @return parsed scene as a polygon object
   */
  static Polygon read_scene(const std::string &filename);

  /**
   * @brief Write polygons into a JSON file
   *
   * @param polygons list of polygons
   * @param filename output filename
   */
  static void write_polygons(const std::vector<Polygon> &polygons, const std::string &filename);

  /**
   * @brief Write segments into a JSON file
   *
   * @param segments list of segments
   * @param filename output file
   */
  static void write_segments(const std::vector<Segment> &segments, const std::string &filename);
};

} // namespace FDML

#endif
