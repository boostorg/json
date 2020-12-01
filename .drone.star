# Use, modification, and distribution are
# subject to the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt)
#
# Copyright Rene Rivera 2020.

# Configuration for https://cloud.drone.io/.

# For Drone CI we use the Starlark scripting language to reduce duplication.
# As the yaml syntax for Drone CI is rather limited.

def main(ctx):
  addon_clang_38 = { "apt": { "packages": [ "clang-3.8"] } }
  addon_clang_39 = { "apt": { "packages": [ "clang-3.9"] } }
  addon_clang_4 = { "apt": { "packages": [ "clang-4.0", "libstdc++-6-dev" ] } }
  addon_clang_5 = { "apt": { "packages": [ "clang-5.0", "libstdc++-7-dev" ] } }
  addon_clang_6 = { "apt": { "packages": [ "clang-6.0", "libc6-dbg", "libstdc++-8-dev" ] } }
  addon_clang_7 = { "apt": { "packages": [ "clang-7", "libstdc++-8-dev" ] } }
  addon_clang_8 = { "apt": { "packages": [ "clang-8", "libstdc++-8-dev" ] } }
  addon_clang_9 = { "apt": { "packages": [ "clang-9", "libstdc++-9-dev" ] } }
  addon_clang_10 = { "apt": { "packages": [ "clang-10", "libstdc++-9-dev" ] } }
  addon_clang_11 = { "apt": { "packages": [ "clang-11", "libstdc++-9-dev" ] } }

  addon_gcc_44 = { "apt": { "packages": [ "g++-4.4" ] } }
  addon_gcc_46 = { "apt": { "packages": [ "g++-4.6" ] } }
  addon_gcc_47 = { "apt": { "packages": [ "g++-4.7" ] } }
  addon_gcc_48 = { "apt": { "packages": [ "g++-4.8" ] } }
  addon_gcc_49 = { "apt": { "packages": [ "g++-4.9" ] } }
  addon_gcc_5 =  { "apt": { "packages": [ "g++-5"   ] } }
  addon_gcc_6 =  { "apt": { "packages": [ "g++-6"   ] } }
  addon_gcc_7 =  { "apt": { "packages": [ "g++-7"   ] } }
  addon_gcc_8 =  { "apt": { "packages": [ "g++-8"   ] } }
  addon_gcc_9 =  { "apt": { "packages": [ "g++-9"   ] } }
  addon_gcc_10 = { "apt": { "packages": [ "g++-10"  ] } }

  return [
    linux_cxx("Clang 3.8", "clang++-3.8", packages=" ".join(addon_clang_38["apt"]["packages"]), llvm_os="trusty", llvm_ver="3.8", image="ubuntu:14.04", buildtype="boost", environment={"B2_TOOLSET": "clang-3.8", "B2_CXXSTD": "11"}),
    linux_cxx("Clang 4.0", "clang++-4.0", packages=" ".join(addon_clang_4["apt"]["packages"]), llvm_os="xenial", llvm_ver="4.0", buildtype="boost", environment={"B2_TOOLSET": "clang-4.0", "B2_CXXSTD": "11,14"}),
    linux_cxx("Clang 5.0", "clang++-5.0", packages=" ".join(addon_clang_5["apt"]["packages"]), llvm_os="bionic", llvm_ver="5.0", buildtype="boost", environment={"B2_TOOLSET": "clang-5.0", "B2_CXXSTD": "11,14"}),
    linux_cxx("Clang 6.0", "clang++-6.0", packages=" ".join(addon_clang_6["apt"]["packages"]), llvm_os="bionic", llvm_ver="6.0", buildtype="boost", environment={"B2_TOOLSET": "clang-6.0", "B2_CXXSTD": "14,17" }),
    linux_cxx("Clang 7", "clang++-7", packages=" ".join(addon_clang_7["apt"]["packages"]), llvm_os="bionic", llvm_ver="7", buildtype="boost", environment={"B2_TOOLSET": "clang-7", "B2_CXXSTD": "17,2a"}),
    linux_cxx("Clang 8", "clang++-8", packages=" ".join(addon_clang_8["apt"]["packages"]), llvm_os="bionic", llvm_ver="8", buildtype="boost", environment={"B2_TOOLSET": "clang-8", "B2_CXXSTD" :"17,2a"}),
    linux_cxx("Clang 9 standalone", "clang++-9", packages=" ".join(addon_clang_9["apt"]["packages"]), llvm_os="bionic", llvm_ver="9", buildtype="standalone", environment={"COMMENT": "standalone", "CXX": "clang++-9"}),
    linux_cxx("Clang 9", "clang++-9", packages=" ".join(addon_clang_9["apt"]["packages"]), llvm_os="bionic", llvm_ver="9", buildtype="boost", environment={"B2_TOOLSET": "clang-9", "B2_CXXSTD": "17,2a"}),
    linux_cxx("Clang 10 standalone", "clang++-10", packages=" ".join(addon_clang_10["apt"]["packages"]), llvm_os="bionic", llvm_ver="10", buildtype="standalone", environment={"COMMENT": "standalone", "CXX": "clang++-10"}),
    linux_cxx("Clang 10", "clang++-10", packages=" ".join(addon_clang_10["apt"]["packages"]), llvm_os="bionic", llvm_ver="10", buildtype="boost", environment={"B2_TOOLSET": "clang-10", "B2_CXXSTD": "17,2a"}),
    linux_cxx("Clang 11 standalone", "clang++-11", packages=" ".join(addon_clang_11["apt"]["packages"]), llvm_os="bionic", llvm_ver="11", buildtype="standalone", environment={"COMMENT": "standalone", "CXX":"clang++-11"}),
    linux_cxx("Clang 11", "clang++-11", packages=" ".join(addon_clang_11["apt"]["packages"]), llvm_os="bionic", llvm_ver="11", buildtype="boost", environment={"B2_TOOLSET": "clang-11", "B2_CXXSTD":"17,2a"}),
    linux_cxx("gcc 4.8", "g++-4.8", packages=" ".join(addon_gcc_48["apt"]["packages"]), image="ubuntu:14.04", buildtype="boost", environment={"B2_TOOLSET": "gcc-4.8", "B2_CXXSTD": "11"}),
    linux_cxx("gcc 4.9", "g++-4.9", packages=" ".join(addon_gcc_49["apt"]["packages"]), image="ubuntu:14.04", buildtype="boost", environment={"B2_TOOLSET": "gcc-4.9", "B2_CXXSTD": "11"}),
    linux_cxx("gcc 5", "g++-5", packages=" ".join(addon_gcc_5["apt"]["packages"]), image="ubuntu:16.04", buildtype="boost", environment={"B2_TOOLSET": "gcc-5", "B2_CXXSTD": "11"}),
    linux_cxx("gcc 6", "g++-6", packages=" ".join(addon_gcc_6["apt"]["packages"]), image="ubuntu:16.04", buildtype="boost", environment={"B2_TOOLSET": "gcc-6", "B2_CXXSTD": "11,14"}),
    linux_cxx("gcc 7", "g++-7", packages=" ".join(addon_gcc_7["apt"]["packages"]), image="ubuntu:16.04", buildtype="boost", environment={"B2_TOOLSET": "gcc-7", "B2_CXXSTD": "14,17"}),
    linux_cxx("gcc 8", "g++-8", packages=" ".join(addon_gcc_8["apt"]["packages"]), image="ubuntu:16.04", buildtype="boost", environment={"B2_TOOLSET": "gcc-8", "B2_CXXSTD": "17,2a"}),
    linux_cxx("gcc 9", "g++-9", packages=" ".join(addon_gcc_9["apt"]["packages"]), image="ubuntu:18.04", buildtype="boost", environment={"B2_TOOLSET": "gcc-9", "B2_CXXSTD": "17,2a"}),
    linux_cxx("gcc 9 standalone", "g++-9", packages=" ".join(addon_gcc_9["apt"]["packages"]), image="ubuntu:18.04", buildtype="standalone", environment={"COMMENT": "standalone", "CXX": "g++-9"}),
    linux_cxx("gcc 10", "g++-10", packages=" ".join(addon_gcc_10["apt"]["packages"]), image="ubuntu:18.04", buildtype="boost", environment={"B2_TOOLSET": "gcc-10", "B2_CXXSTD": "17,2a"}),
    linux_cxx("gcc 10 standalone", "g++-10", packages=" ".join(addon_gcc_10["apt"]["packages"]), image="ubuntu:18.04", buildtype="standalone", environment={"COMMENT": "standalone", "CXX": "g++-10"}),
    linux_cxx("coverity", "", packages="", image="ubuntu:18.04", buildtype="coverity", environment={}),
    linux_cxx("docs", "", packages="docbook docbook-xml docbook-xsl xsltproc libsaxonhe-java default-jre-headless flex libfl-dev bison unzip", image="ubuntu:16.04", buildtype="docs", environment={"COMMENT": "docs"}),
    linux_cxx("codecov", "", packages=" ".join(addon_gcc_8["apt"]["packages"]), image="ubuntu:16.04", buildtype="codecov", environment={"COMMENT": "codecov.io","LCOV_BRANCH_COVERAGE": 0,"B2_CXXSTD": 11,"B2_TOOLSET": "gcc-8", "B2_DEFINES": "BOOST_NO_STRESS_TEST=1"}, stepenvironment={"CODECOV_TOKEN": {"from_secret": "codecov_token"} }),
    linux_cxx("valgrind", "", packages=" ".join(addon_clang_6["apt"]["packages"]) + " autotools-dev automake", image="ubuntu:16.04", buildtype="valgrind", environment={"COMMENT": "valgrind","B2_TOOLSET": "clang-6.0", "B2_CXXSTD": "11,14", "B2_DEFINES": "BOOST_NO_STRESS_TEST=1", "B2_VARIANT": "debug", "B2_TESTFLAGS": "testing.launcher=valgrind","VALGRIND_OPTS": "--error-exitcode=1" }),
    linux_cxx("asan", "clang++-11", packages=" ".join(addon_clang_11["apt"]["packages"]), image="ubuntu:16.04", llvm_os="xenial", llvm_ver="11", buildtype="boost", environment={"COMMENT": "asan", "B2_VARIANT": "debug", "B2_TOOLSET": "clang-11", "B2_CXXSTD":"17", "B2_ASAN": "1", "B2_DEFINES": "BOOST_NO_STRESS_TEST=1"}, privileged=True),
    linux_cxx("ubsan", "clang++-11", packages=" ".join(addon_clang_11["apt"]["packages"]), llvm_os="bionic", llvm_ver="11", buildtype="boost", environment={"COMMENT": "asan", "B2_VARIANT": "debug", "B2_TOOLSET": "clang-11", "B2_CXXSTD":"17", "B2_UBSAN": "1", "B2_DEFINES": "BOOST_NO_STRESS_TEST=1" }),
    # linux_cxx("Intel", "", packages="g++-7 cmake build-essential pkg-config", buildtype="intel", environment={"COMMENT": "Intel oneAPI Toolkit", "B2_TOOLSET": "intel-linux", "B2_CXXSTD": "11,14,17", "B2_FLAGS": "warnings=on warnings-as-errors=off" }),
    windows_cxx("msvc-14.1", "g++", image="cppalliance/dronevs2017", buildtype="boost", environment={"B2_TOOLSET": "msvc-14.1", "B2_CXXSTD": "11,14,17"}),
    windows_cxx("msvc-14.1 standalone", "msvc-14.1", image="cppalliance/dronevs2017", buildtype="standalone-windows", environment={"COMMENT": "standalone","CXX": "msvc-14.1"}),
    windows_cxx("msvc-14.2", "g++", image="cppalliance/dronevs2019", buildtype="boost", environment={"B2_TOOLSET": "msvc-14.2", "B2_CXXSTD": "17,latest"}),
    windows_cxx("msvc-14.2 standalone", "msvc-14.2", image="cppalliance/dronevs2019", buildtype="standalone-windows", environment={"COMMENT": "standalone","CXX": "msvc-14.2"})
    ]

# Generate pipeline for Linux platform compilers.
def linux_cxx(name, cxx, cxxflags="", packages="", llvm_os="", llvm_ver="", arch="amd64", image="ubuntu:18.04", buildtype="boost", environment={}, stepenvironment={}, privileged=False):
  environment_global = {
      "CXX": cxx,
      "CXXFLAGS": cxxflags,
      "PACKAGES": packages,
      "LLVM_OS": llvm_os,
      "LLVM_VER": llvm_ver,
      "B2_CI_VERSION": 1,
      # see: http://www.boost.org/build/doc/html/bbv2/overview/invocation.html#bbv2.overview.invocation.properties
      # - B2_ADDRESS_MODEL=64,32
      # - B2_LINK=shared,static
      # - B2_THREADING=threading=multi,single
      "B2_VARIANT" : "release",
      "B2_FLAGS" : "warnings=extra warnings-as-errors=on"
    }
  environment_current=environment_global
  environment_current.update(environment)

  return {
    "name": "Linux %s" % name,
    "kind": "pipeline",
    "type": "docker",
    "trigger": { "branch": [ "master","develop", "drone*", "bugfix/*", "feature/*", "fix/*", "pr/*" ] },
    "platform": {
      "os": "linux",
      "arch": arch
    },
    # Create env vars per generation arguments.
    "environment": environment_current,
    "clone": { "depth": 5 },
    "steps": [
      {
        "name": "Everything",
        "image": image,
        "privileged" : privileged,
        "environment": stepenvironment,
        "commands": [

          "echo '==================================> SETUP'",
          "uname -a",
          "apt-get -o Acquire::Retries=3 update && DEBIAN_FRONTEND=noninteractive apt-get -y install tzdata && apt-get -o Acquire::Retries=3 install -y sudo software-properties-common wget curl apt-transport-https git cmake apt-file sudo mercurial && rm -rf /var/lib/apt/lists/*",

          "echo '==================================> PACKAGES'",
          "./.drone/linux-cxx-install.sh",

          "echo '==================================> INSTALL AND COMPILE'",
          "./.drone/%s-script.sh" % buildtype,
        ]
      }
    ]
  }

def windows_cxx(name, cxx="g++", cxxflags="", packages="", llvm_os="", llvm_ver="", arch="amd64", image="ubuntu:18.04", buildtype="boost", environment={}):
  environment_global = {
      "CXX": cxx,
      "CXXFLAGS": cxxflags,
      "PACKAGES": packages,
      "LLVM_OS": llvm_os,
      "LLVM_VER": llvm_ver,
      "B2_CI_VERSION": 1,
      # see: http://www.boost.org/build/doc/html/bbv2/overview/invocation.html#bbv2.overview.invocation.properties
      # - B2_ADDRESS_MODEL=64,32
      # - B2_LINK=shared,static
      # - B2_THREADING=threading=multi,single
      "B2_VARIANT" : "release",
      "B2_FLAGS" : "warnings=extra warnings-as-errors=on"
    }
  environment_current=environment_global
  environment_current.update(environment)

  return {
    "name": "Windows %s" % name,
    "kind": "pipeline",
    "type": "docker",
    "trigger": { "branch": [ "master","develop", "drone", "bugfix/*", "feature/*", "fix/*", "pr/*" ] },
    "platform": {
      "os": "windows",
      "arch": arch
    },
    # Create env vars per generation arguments.
    "environment": environment_current,
    "clone": { "depth": 5 },
    "steps": [
      {
        "name": "Everything",
        "image": image,
        "commands": [
          "echo '==================================> SETUP'",
          "echo '==================================> PACKAGES'",
          "bash.exe ./.drone/windows-msvc-install.sh",

          "echo '==================================> INSTALL AND COMPILE'",
          "bash.exe ./.drone/%s-script.sh" % buildtype,
        ]
      }
    ]
  }
