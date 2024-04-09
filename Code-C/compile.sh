# !/bin/bash

if [ "$1" = "all" ]; then
    cd ClientServer
    make all -j32
    cd ../Robot/
    make all -j32
    cd ..
elif [ "$1" = "clean" ]; then
    cd ClientServer
    make clean
    cd ../Robot/
    make clean
    cd ..
else
    echo "Invalid option."
    exit 1
fi
