project(AssetProvider)

add_library(AssetProvider STATIC Source/ZIPLevelAssetProvider.cpp)

target_include_directories(AssetProvider PUBLIC Include)
target_link_libraries(AssetProvider PUBLIC GameLib)
target_link_libraries(AssetProvider PRIVATE zip zlib bz2 lzma zstd_static)