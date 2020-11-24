#!/bin/sh

# Usage:
# LLVM_OS: LLVM OS release to obtain clang binaries. Only needed for clang install.
# LLVM_VER: The LLVM toolset version to point the repo at.
# PACKAGES: Compiler packages to install.

set -xe
echo ">>>>>"
echo ">>>>> APT: REPO.."
echo ">>>>>"
sudo -E apt-add-repository -y "ppa:ubuntu-toolchain-r/test"
if test -n "${LLVM_OS}" ; then
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
    if test -n "${LLVM_VER}" ; then
	sudo -E apt-add-repository "deb http://apt.llvm.org/${LLVM_OS}/ llvm-toolchain-${LLVM_OS}-${LLVM_VER} main"
    else
        # Snapshot (i.e. trunk) build of clang
	sudo -E apt-add-repository "deb http://apt.llvm.org/${LLVM_OS}/ llvm-toolchain-${LLVM_OS} main"
    fi
fi
echo ">>>>>"
echo ">>>>> APT: UPDATE.."
echo ">>>>>"
sudo -E apt-get -o Acquire::Retries=3 update
echo ">>>>>"
echo ">>>>> APT: INSTALL ${PACKAGES}.."
echo ">>>>>"
sudo -E apt-get -o Acquire::Retries=3 -yq --no-install-suggests --no-install-recommends install ${PACKAGES}

MAJOR_VERSION=$(lsb_release -r -s | cut -c 1-2)
if [ "$MAJOR_VERSION" -lt "20" ]; then
    sudo -E apt-get -o Acquire::Retries=3 -y install python python-pip
fi

if [ "$MAJOR_VERSION" -gt "18" ]; then
    sudo -E apt-get -o Acquire::Retries=3 -y install python3 python3-pip
    ln -s /usr/bin/python3 /usr/bin/python
    ln -s /usr/bin/pip3 /usr/bin/pip
fi
