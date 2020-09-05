#!/bin/sh
#
# builds the fuzzers, runs old crashes etc
#
# Optional: set environment variable CLANG, otherwise clang is auto detected.
#
# By Paul Dreik 2019-2020 for the boost json project
# License: Boost 1.0

set -e

fuzzdir=$(dirname $0)
me=$(basename $0)

cd $fuzzdir

if [ -z $CLANG ] ; then
    #see if we can find clang
    for clangver in -10 -9 -8 -7 -6 -6.0 "" ;   do
	CLANG=clang++$clangver
	if which $CLANG >/dev/null; then
	    break
	fi
    done
fi

if ! which $CLANG >/dev/null; then
    if ! -x $CLANG; then
	echo $me: sorry, could not find clang $CLANG
	exit 1
    fi
fi
echo "$me: will use this compiler: $CLANG"

variants="basic_parser parse parser"

for variant in $variants; do

srcfile=fuzz_$variant.cpp
fuzzer=./fuzzer_$variant

if [ ! -e $fuzzer -o $srcfile -nt $fuzzer ] ; then
    
    $CLANG \
	-std=c++17 \
	-O3 \
	-g \
	-fsanitize=fuzzer,address,undefined \
	-fno-sanitize-recover=undefined \
	-DBOOST_JSON_STANDALONE \
	-DBOOST_JSON_HEADER_ONLY \
	-I../include \
	-o $fuzzer \
	$srcfile
fi

# make sure ubsan stops in case anything is found
export UBSAN_OPTIONS="halt_on_error=1"

# make sure the old crashes pass without problems
if [ -d old_crashes/$variant ]; then
  find old_crashes/$variant -type f -print0 |xargs -0 --no-run-if-empty $fuzzer
fi

# make an initial corpus from the test data already in the repo
seedcorpus=seedcorpus/$variant
if [ ! -d $seedcorpus ] ; then
    mkdir -p $seedcorpus
    find ../test -name "*.json" -type f -print0 |xargs -0 --no-run-if-empty cp -f -t $seedcorpus/
fi

# if an old corpus exists, use it
# get it with curl -O --location -J https://bintray.com/pauldreik/boost.json/download_file?file_path=corpus%2Fcorpus.tar
if [ -e corpus.tar ] ; then
  mkdir -p oldcorpus
  tar xf corpus.tar -C oldcorpus || echo "corpus.tar was broken! ignoring it"
  OLDCORPUS=oldcorpus/cmin/$variant
  # in case the old corpus did not have this variant (when adding/renaming a new fuzzer)
  mkdir -p $OLDCORPUS
else
  OLDCORPUS=
fi


# run the fuzzer for a short while
mkdir -p out/$variant
$fuzzer out/$variant $OLDCORPUS $seedcorpus/ -max_total_time=30

# minimize the corpus
mkdir -p cmin/$variant
$fuzzer cmin/$variant $OLDCORPUS out/$variant $seedcorpus/ -merge=1

done

