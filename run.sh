cd build
# Cleaning the previous compiled executables if present
make clean
# Generating make file
cmake ..
# Compiling
make
# Going inside the folder with the executables
cd ../bin
# Running the project
./master
