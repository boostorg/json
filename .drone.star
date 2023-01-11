# Use, modification, and distribution are
# subject to the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt)
#
# Copyright Rene Rivera 2020.
# Copyright Alan de Freitas 2022.

# For Drone CI we use the Starlark scripting language to reduce duplication.
# As the yaml syntax for Drone CI is rather limited.
#
#

def main(ctx):
    return generate(
        # Compilers
        ['gcc >=5.0',
         'clang >=3.8',
         'msvc >=14.0',
         'arm64-gcc latest',
         's390x-gcc latest',
         'apple-clang *',
         'arm64-clang latest',
         's390x-clang latest',
         'x86-msvc latest'],
        # Standards
        '>=11')


# from https://github.com/boostorg/boost-ci
load("@boost_ci//ci/drone/:functions.star", "linux_cxx", "windows_cxx", "osx_cxx", "freebsd_cxx")
load("@url//:.drone.star", "generate")
