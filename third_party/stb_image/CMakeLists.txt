cmake_minimum_required(VERSION 3.25)

add_library(stb_image src/stb_image.c)

target_include_directories(stb_image PUBLIC include)