#====- setup.py ----------------------------------------------*- Python -*-===//
#
#  Copyright (C) 2020 GrammaTech, Inc.
#
#  This code is licensed under the MIT license. See the LICENSE file in the
#  project root for license terms.
#
# This material is based on research sponsored by the Department of Homeland
# Security (DHS) Office of Procurement Operations, S&T acquisition Division via
# contract number 70RSAT19C00000056. The views and conclusions contained herein
# are those of the authors and should not be interpreted as necessarily
# representing the official policies or endorsements, either expressed or
# implied, of the Department of Homeland Security.
#
#====----------------------------------------------------------------------===//
from distutils.core import setup, Extension

setup(name="swappedargs", version="0.1",
      ext_modules=[
          Extension("swappedargs", ["SwappedArgsExt.cpp"],
                    extra_compile_args=['-std=c++17'],
                    libraries=["SwappedArgChecker"])
      ])
