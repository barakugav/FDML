#include "defs.h"
#include <vector>

#ifndef __JSON_UTILS_H__
#define __JSON_UTILS_H__

void parse_scene_from_json(const std::string &filename, std::vector<Point> &points);
void write_polygons_to_json(const std::vector<Polygon> &polygons, const std::string &filename);

#endif
