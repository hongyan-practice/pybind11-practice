#!/usr/bin/env python
#  -*- coding: utf-8 -*-
__date__ = '2021/6/10'

from glob import glob
from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext

__version__ = "0.0.1"

ext_modules = [
    Pybind11Extension("demo",
                      sorted(glob("**/*.cpp", recursive = True)),
                      define_macros=[('VERSION_INFO', __version__)]
                      ),
]

setup(
    name="demo",
    version=__version__,
    author="HongYan",
    author_email="524425141@qq.com",
    description="some tiny demo for pybind11 practice",
    long_description="",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
)
