cd ..
mkdir build
cd build
cmake ..
cmake --build . --config=Release
.\modules\console\Release\rmvm.exe rmvm.exe > base.json
code base.json