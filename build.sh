#!/bin/bash

set -eu

function build_clean()
{
    rm -rf build
}

function build_test()
{
    mkdir -p build
    cd build
    cmake .. 
    make -j8 
    make test
}

function usage()
{
    echo "./build.sh [clean] [-h|--help]"
}

opt=""
if [ $# -ge 1 ];then
    opt=$1
fi

case "${opt}" in
    clean)
        build_clean
        ;;
    -h|--help)
        usage
        ;;
    *)
        build_test
        ;;
esac
