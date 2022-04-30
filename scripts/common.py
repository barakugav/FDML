#!/usr/bin/env python3


import platform
import os
import sys
import zipfile
import tarfile
import tqdm
import urllib.request


SCRIPTS_TOP = os.path.dirname(os.path.realpath(__file__))
FDML_TOP = os.path.abspath(os.path.join(SCRIPTS_TOP, ".."))
LIBS_TOP = os.path.join(FDML_TOP, "libs")


class ConfigError(Exception):
    pass


class SilentExit(Exception):
    pass


def is_linux():
    return sys.platform == "linux" or sys.platform == "linux2"


def is_64bit():
    return platform.architecture()[0] == "64bit"


def download_with_progressbar(url, out_filename):
    def reporthook(block_index, block_size, total_size):
        if reporthook.progressbar is None:
            reporthook.progressbar = iter(
                tqdm.tqdm(range(total_size), unit='B', unit_scale=True, unit_divisor=1024, desc="Downloading"))
        bytes_num = block_index * block_size
        for i in reporthook.progressbar:
            if i >= bytes_num:
                break
    reporthook.progressbar = None

    urllib.request.urlretrieve(
        url, filename=out_filename, reporthook=reporthook)

    # exhaust the progressbar
    if reporthook.progressbar is not None:
        for _ in reporthook.progressbar:
            pass


def tar_extract_with_progressbar(tar_filename, output_dir):
    with tarfile.open(tar_filename, 'r') as tar_file:
        for member in tqdm.tqdm(tar_file.getmembers(), desc="Extracting"):
            tar_file.extract(member, path=output_dir)


def zip_extract_with_progressbar(zip_filename, output_dir):
    with zipfile.ZipFile(zip_filename) as zip_file:
        for member in tqdm.tqdm(zip_file.infolist(), desc="Extracting"):
            zip_file.extract(member, output_dir)
