#include "defs.h"
#include <vector>

#ifndef __JSON_UTILS_H__
#define __JSON_UTILS_H__

/**
 * @brief Parse a scene from a JSON file
 *
 * @param filename path to scene file
 * @param scene output polygon for the parsed scene
 */
void parse_scene_from_json(const std::string &filename, Polygon &scene);

/**
 * @brief Write polygons into a JSON file
 *
 * @param polygons list of polygons
 * @param filename output filename
 */
void write_polygons_to_json(const std::vector<Polygon> &polygons, const std::string &filename);

#endif
