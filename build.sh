#!/bin/bash


project_root_dir=$(pwd)

git submodule init
git submodule update

build_dir=$project_root_dir/build
if [ ! -d $build_dir ]; then
    mkdir $build_dir
fi



libnabo_build_dir=external/libnabo/build
if [ ! -d $libnabo_build_dir ]; then
    mkdir $libnabo_build_dir
fi

cd $libnabo_build_dir
cmake ..
cmake --build . --config Release


cd $build_dir
cmake ..
cmake --build . 

