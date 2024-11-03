cmake -B ./build  --toolchain arm-none-eabi.toolchain
cmake --build ./build
cmake --install ./build --prefix prebuilt