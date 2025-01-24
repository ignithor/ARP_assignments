if [ -d "build" ]; then
  rm -r build
fi

if [ -d "bin" ]; then
  rm -r bin
fi

mkdir build
cd build
# Generating make file
cmake ..
# Compiling
make
# Going inside the folder with the executables
cd ../bin
# Running the project
./master
