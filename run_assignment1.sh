git checkout main

# Delete old build file if it exists
if [ -d "build" ]; then
  rm -r build
fi

# Create a new build folder and go into it
mkdir build
cd build

# Generating make file
cmake ..

# Compiling
make

# Go inside the folder with all the executables
cd ../bin

# Running the project by running the master process
./master
