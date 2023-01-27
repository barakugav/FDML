#!/usr/bin/env python3

import os
import argparse
import json
import scripts.common as common


ALL_DEPENDENCIES_REQUIRED = False

def _write_dependencies_cmake(installed_paths, include_paths, library_paths):
    deps_cmake_filename = os.path.join(common.FDML_TOP, "cmake", "dependencies.cmake")
    with open(deps_cmake_filename, "w") as deps_cmake:
        for installed_path in installed_paths:
            deps_cmake.write(
                "SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} %s)\n" % installed_path)
        for include_path in include_paths:
            deps_cmake.write(
                "SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} %s)\n" % include_path)
        for library_path in library_paths:
            deps_cmake.write(
                "SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} %s)\n" % library_path)


DEPENDENCIES = ["gmp_inc", "gmp_lib", "mpfr_inc",
                "mpfr_lib", "boost_inc", "boost_lib", "cgal_dir"]


def configure(config_filename):
    config = {}
    if config_filename is not None:
        with open(config_filename, "r") as config_file:
            config = json.load(config_file)

    dependencies = {}
    if "dependencies" in config:
        config_deps = config["dependencies"]
        for dependency in DEPENDENCIES:
            if dependency in config_deps:
                dependencies[dependency] = os.path.abspath(config_deps[dependency]).replace("\\", "/")

    if ALL_DEPENDENCIES_REQUIRED:
        for dependency in DEPENDENCIES:
            if dependency not in dependencies:
                raise common.ConfigError("Failed to find dependency:", dependency)
    def get_values_if_exists(dict, *keys):
        return [dict[key] for key in filter(lambda key: key in dict, keys)]
    installed_paths = get_values_if_exists(dependencies, "cgal_dir")
    include_paths = get_values_if_exists(dependencies, "gmp_inc", "mpfr_inc", "boost_inc")
    library_paths = get_values_if_exists(dependencies, "gmp_lib", "mpfr_lib", "boost_lib")
    _write_dependencies_cmake(installed_paths, include_paths, library_paths)


def main():
    parser = argparse.ArgumentParser(description="FDML configuration")
    parser.add_argument("--config", dest="configfile", type=str,
                        help="Configuration file, see doc/config_example.json")
    args = parser.parse_args()

    try:
        configure(args.configfile)
    except common.ConfigError as e:
        print("Error was encountered:", str(e))
    except common.SilentExit:
        pass


if __name__ == "__main__":
    main()
