#!/usr/bin/env python3

import sys
import os
import shutil
import subprocess
import platform
import multiprocessing
import re
import argparse
import click

FDML_TOP = os.path.abspath(os.path.join(
    os.path.dirname(os.path.realpath(__file__)), ".."))
if FDML_TOP not in sys.path:
    sys.path.insert(0, FDML_TOP)
from scripts import get_python
from scripts import common


VERSION_REQUIRED = (1, 78, 0)


def get_download_url(version):
    return "https://sourceforge.net/projects/boost/files/boost/%d.%d.%d/boost_%d_%d_%d.tar.gz/download" % (*version, *version)


def download_sources(version, output_dir):
    if os.path.exists(output_dir):
        print("Output directory exists:", output_dir)
        if not click.confirm("Do you want to delete it?", default=False):
            raise common.SilentExit()
        shutil.rmtree(output_dir)
    os.makedirs(output_dir, exist_ok=True)

    boost_ver_format = "boost_%d_%d_%d" % version
    zip_filename = os.path.join(output_dir, "%s.tar.gz" % boost_ver_format)

    url = get_download_url(version)
    print("Downloading into \"%s\" from:" % zip_filename, url)
    common.download_with_progressbar(url, zip_filename)

    print("Extracting boost sources into:", output_dir)
    common.tar_extract_with_progressbar(zip_filename, output_dir)
    os.remove(zip_filename)

    # boost contains another directory within the zip, move all files one dir up
    inner_dir = os.path.join(output_dir, boost_ver_format)
    if os.path.exists(inner_dir):
        inner_dir_new = os.path.join(
            output_dir, "%s_toremove" % os.path.basename(inner_dir))
        shutil.move(inner_dir, inner_dir_new)
        for inner_file in os.listdir(inner_dir_new):
            shutil.move(os.path.join(
                inner_dir_new, inner_file), output_dir)
        os.rmdir(inner_dir_new)


def _user_config_str(python_exe, python_version):
    if " " in python_exe:
        raise common.ConfigError(
            "Boost can handle only python interperter with no spaces in their path.", python_exe)
    return """import toolset ;

using python : %d.%d : %s ;

""" % (python_version[0], python_version[1], python_exe)


def build(source_dir, build_dir, bin_dir, python_exe=None, numpy_enable=False):
    if not os.path.exists(source_dir):
        raise common.ConfigError("provided sources dir doesn't exists", source_dir)
    if python_exe is None and numpy_enable:
        raise common.ConfigError("numpy can be enable only if python is provided")
    if numpy_enable and not get_python.is_numpy_enable(python_exe):
        raise common.ConfigError("numpy was not found in python:", python_exe)

    if python_exe is not None:
        python_version = get_python.get_version(python_exe)
        user_config_str = _user_config_str(python_exe, python_version)
    else:
        user_config_str = ""
    user_config_filename = os.path.join(source_dir, "user-config.jam")
    with open(user_config_filename, "w") as user_config:
        user_config.write(user_config_str)

    bootstap_file = os.path.join(source_dir, "bootstrap.%s" % (
        "sh" if common.is_linux() else "bat"))
    if python_exe is not None:
        subprocess.run([bootstap_file, "--with-python=%s" % python_exe, "--with-python-version=%d.%d" %
                        (python_version[0], python_version[1])], cwd=source_dir).check_returncode()
    else:
        subprocess.run([bootstap_file], cwd=source_dir).check_returncode()

    x86_machine_names = ["AMD64", "x86_64", "i386", "x86", "i686"]
    arm_machine_names = ["arm", "aarch64_be", "aarch64",
                         "armv8b", "armv8l", "armv7l", "arm", ]
    machine_name = platform.machine()
    if machine_name in x86_machine_names:
        arch = "x86"
    elif machine_name in arm_machine_names:
        arch = "arm"
    else:
        raise common.ConfigError("could not determine architecture:", machine_name)

    b2_file = os.path.join(source_dir, "b2")
    args = [b2_file, "--user-config=%s" % user_config.name,
            "--build-dir=%s" % build_dir, "--stagedir=%s" % bin_dir,
            "architecture=%s" % arch, "address-model=%d" % (
                64 if common.is_64bit() else 32),
            "-j%d" % multiprocessing.cpu_count(), "link=static,shared",
            "--variant=debug,release", "--debug-configuration"]
    if not common.is_linux():  # windows
        args += ["runtime-link=static,shared"]
    subprocess.run(args, cwd=source_dir).check_returncode()


def main():
    parser = argparse.ArgumentParser(description="Get boost")
    source_dir_default = "{boost_top}/boost_{ver_maj}_{ver_min}_{ver_patch}"
    build_dir_default = "{boost_top}/boost_{ver_maj}_{ver_min}_{ver_patch}_build"
    bin_dir_default = "{boost_top}/boost_{ver_maj}_{ver_min}_{ver_patch}_bin"
    parser.add_argument(
        "--cmd", choices=["download", "build", "all"], required=True, help="command type")
    parser.add_argument("--version", type=str, default="1.79.0",
                        help="Version of boost to download, for example 1.78.0 (necessary for download)")
    parser.add_argument("--boost-top", type=str,
                        help="top directory for sources, build and binaries dirs. For example './libs/boost'"
                        "can be used instead of other arguments")
    parser.add_argument("--source-dir", type=str,
                        default=source_dir_default, help="directory for boost sources")
    parser.add_argument("--build-dir", type=str,
                        default=build_dir_default, help="directory for boost build. (only for build command)")
    parser.add_argument("--bin-dir", type=str,
                        default=bin_dir_default,  help="directory for boost binaries. (only for build command)")
    parser.add_argument(
        '--python', type=str, help="Path to python interpeter. "
        "Boost python and boost numpy will be installed along with standard boost libraries. "
        "NumPy is required with the python interpeter. (only for build command)")
    args = parser.parse_args()

    download_en = args.cmd == "download" or args.cmd == "all"
    build_en = args.cmd == "build" or args.cmd == "all"

    ver_re = re.findall("([0-9]+)\.([0-9]+)\.([0-9]+)", args.version)
    if len(ver_re) != 1:
        raise common.ConfigError("Invalid version", args.version)
    ver = tuple([int(x) for x in ver_re[0]])
    if ver < VERSION_REQUIRED:
        raise common.ConfigError("Minimum version is:", VERSION_REQUIRED)

    if args.source_dir == source_dir_default and args.boost_top is None:
        raise common.ConfigError("Source dir or boost top dir is required")
    else:
        source_dir = args.source_dir if args.source_dir != source_dir_default else os.path.join(
            args.boost_top, "boost_%d_%d_%d" % ver)
    source_dir = os.path.abspath(source_dir)

    if build_en and args.build_dir == build_dir_default and args.boost_top is None:
        raise common.ConfigError("Build dir or boost top dir is required")
    else:
        build_dir = args.build_dir if args.build_dir != build_dir_default else os.path.join(
            args.boost_top, "boost_%d_%d_%d_build" % ver)
    build_dir = os.path.abspath(build_dir)

    if build_en and args.bin_dir == bin_dir_default and args.boost_top is None:
        raise common.ConfigError("Bin dir or boost top dir is required")
    else:
        bin_dir = args.bin_dir if args.bin_dir != bin_dir_default else os.path.join(
            args.boost_top, "boost_%d_%d_%d_bin" % ver)
    bin_dir = os.path.abspath(bin_dir)

    python_en = args.python is not None

    try:
        if download_en:
            download_sources(ver, source_dir)
            print("Sources downloaded successfully at:", source_dir)
        if build_en:
            if python_en:
                build(source_dir, build_dir, bin_dir,
                      python_exe=args.python, numpy_enable=True)
            else:
                build(source_dir, build_dir, bin_dir)
            print("Boost built successfully. Binaries at:", bin_dir)
    except common.ConfigError as e:
        print("Error was encountered:", str(e))
    except common.SilentExit:
        pass

if __name__ == "__main__":
    main()

