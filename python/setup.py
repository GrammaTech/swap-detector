from distutils.core import setup, Extension

setup(name="swappedargs", version="0.1",
      ext_modules=[
          Extension("swappedargs", ["SwappedArgsExt.cpp"],
                    extra_compile_args=['-std=c++17'],
                    libraries=["SwappedArgChecker"])
      ])
