#!/usr/bin/env python3

import sys
import os
import shutil
import subprocess
import re
import argparse
import click

FDML_TOP = os.path.abspath(os.path.join(
    os.path.dirname(os.path.realpath(__file__)), ".."))
if FDML_TOP not in sys.path:
    sys.path.insert(0, FDML_TOP)
from scripts import common


VERSION_REQUIRED = (4, 1, 0)


def get_download_url(version):
    return "https://www.mpfr.org/mpfr-current/mpfr-%d.%d.%d.tar.gz" % version


def download_sources(version, output_dir):
    if os.path.exists(output_dir):
        print("Output directory exists:", output_dir)
        if not click.confirm("Do you want to delete it?", default=False):
            raise common.SilentExit()
        shutil.rmtree(output_dir)
    os.makedirs(output_dir)

    mpfr_ver_format = "mpfr-%d.%d.%d" % version
    zip_filename = os.path.join(output_dir, "%s.tar.gz" % mpfr_ver_format)

    url = get_download_url(version)
    print("Downloading into \"%s\" from:" % zip_filename, url)
    common.download_with_progressbar(url, zip_filename)

    print("Extracting MPFR sources into:", output_dir)
    common.tar_extract_with_progressbar(zip_filename, output_dir)
    os.remove(zip_filename)

    # MPFR contains another directory within the zip, move all files one dir up
    inner_dir = os.path.join(output_dir, mpfr_ver_format)
    if os.path.exists(inner_dir):
        inner_dir_new = os.path.join(
            output_dir, "%s_toremove" % os.path.basename(inner_dir))
        shutil.move(inner_dir, inner_dir_new)
        for inner_file in os.listdir(inner_dir_new):
            shutil.move(os.path.join(
                inner_dir_new, inner_file), output_dir)
        os.rmdir(inner_dir_new)


def build(source_dir, install_dir, gmp_dir):
    if not os.path.exists(source_dir):
        raise common.ConfigError(
            "provided sources dir doesn't exists", source_dir)

    if os.path.exists(install_dir):
        print("Install directory exists:", install_dir)
        if not click.confirm("Do you want to delete it?", default=False):
            raise common.SilentExit()
        shutil.rmtree(install_dir)

    print("Building MPFR")
    print("\tsource directory:", source_dir)
    print("\tinstall directory:", install_dir)

    configure_file = os.path.join(source_dir, "configure")
    subprocess.run([configure_file, "--prefix=%s" % install_dir,
                   "--with-gmp=%s" % gmp_dir], cwd=source_dir).check_returncode()
    subprocess.run(["make", "all"], cwd=source_dir).check_returncode()
    subprocess.run(["make", "check"], cwd=source_dir).check_returncode()
    subprocess.run(["make", "install"], cwd=source_dir).check_returncode()


def main():
    if not common.is_linux(): # windows
        print("On windows, MPFR is downloaded and installed along with GMP on windows. Use get_gmp.py")
        return

    parser = argparse.ArgumentParser(description="Get MPFR")
    source_dir_default = "{mpfr_top}/mpfr-{ver_maj}.{ver_min}.{ver_patch}"
    install_dir_default = "{mpfr_top}/mpfr-{ver_maj}.{ver_min}.{ver_patch}_installed"
    parser.add_argument(
        "--cmd", choices=["download", "build", "all"], required=True, help="command type")
    parser.add_argument("--version", type=str, default="4.1.0",
                        help="Version of MPFR to download, for example 4.1.0 (necessary for download)")
    parser.add_argument("--mpfr-top", type=str,
                        help="top directory for sources install dirs. "
                        "can be used instead of other arguments")
    parser.add_argument("--source-dir", type=str,
                        default=source_dir_default, help="directory for MPFR sources")
    parser.add_argument("--install-dir", type=str,
                        default=install_dir_default, help="directory for MPFR installation. (only for build command)")
    parser.add_argument("--gmp-dir", type=str,
                        help="directory of GMP, which is a dependency of MPFR (only for build command)")
    args = parser.parse_args()

    download_en = args.cmd == "download" or args.cmd == "all"
    build_en = args.cmd == "build" or args.cmd == "all"

    ver_re = re.findall("([0-9]+)\.([0-9]+)\.([0-9]+)", args.version)
    if len(ver_re) != 1:
        raise common.ConfigError("Invalid version", args.version)
    ver = tuple([int(x) for x in ver_re[0]])

    if args.source_dir == source_dir_default and args.mpfr_top is None:
        raise common.ConfigError("Source dir or MPFR top dir is required")
    else:
        source_dir = args.source_dir if args.source_dir != source_dir_default else os.path.join(
            args.mpfr_top, "mpfr-%d.%d.%d" % ver)
    source_dir = os.path.abspath(source_dir)

    if build_en and args.install_dir == install_dir_default and args.mpfr_top is None:
        raise common.ConfigError("Build dir or MPFR top dir is required")
    else:
        install_dir = args.install_dir if args.install_dir != install_dir_default else os.path.join(
            args.mpfr_top, "mpfr-%d.%d.%d_installed" % ver)
    install_dir = os.path.abspath(install_dir)

    if build_en and args.gmp_dir is None:
        raise common.ConfigError("GMP dependency is required for MPFR. Provide --gmp-dir")
    if build_en:
        gmp_dir = os.path.abspath(args.gmp_dir)

    try:
        if download_en:
            download_sources(ver, source_dir)
            print("Sources downloaded successfully at:", source_dir)
        if build_en:
            build(source_dir, install_dir, gmp_dir)
            print("MPFR built successfully at:", install_dir)
    except common.ConfigError as e:
        print("Error was encountered:", str(e))
    except common.SilentExit:
        pass

if __name__ == "__main__":
    main()

