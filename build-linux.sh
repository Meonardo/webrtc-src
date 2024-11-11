export PATH="$PATH:/home/suma/project/depot_tools"

is_debug=false
target_lib_dir="out/lib/Release"

# check if debug build
if [ "$1" == "debug" ]; then
  BUILD_TYPE="debug"
  is_debug=true
  target_lib_dir="out/lib/Debug"
else
  BUILD_TYPE="release"
fi

# check if the out directory exists
if [ ! -d "$target_lib_dir" ]; then
  mkdir -p "$target_lib_dir"
fi

# check if the out-$BUILD_TYPE directory exists
if [ ! -d "out-$BUILD_TYPE/Linux-x64" ]; then
    mkdir "out-$BUILD_TYPE/Linux-x64"
fi

# generate build files
gn gen out-$BUILD_TYPE/Linux-x64 --args="target_os=\"linux\" target_cpu=\"x64\" is_debug=$is_debug rtc_include_tests=false rtc_use_h264=true ffmpeg_branding=\"Chrome\" is_component_build=false use_rtti=true use_custom_libcxx=false rtc_enable_protobuf=false"

# build the project
ninja -C out-$BUILD_TYPE/Linux-x64 libwebrtc

# copy the generated library to the out directory
cp out-$BUILD_TYPE/Linux-x64/libwebrtc.so $target_lib_dir/libwebrtc.so
cp -r libwebrtc/include $target_lib_dir/../..