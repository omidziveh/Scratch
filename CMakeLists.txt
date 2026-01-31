cmake_minimum_required(VERSION 3.10)
project(Scratch)

set(CMAKE_CXX_STANDARD 17)


if(WIN32)
    set(SDL2_PATH "C:/SDL2/SDL2-2.32.10/x86_64-w64-mingw32")
    set(SDL2_TTF_PATH "C:/SDL2/SDL2_ttf-2.22.0/x86_64-w64-mingw32")
    set(SDL2_IMAGE_PATH "C:/SDL2/SDL2_image-2.8.2/x86_64-w64-mingw32")
    set(CMAKE_PREFIX_PATH ${SDL2_PATH} ${SDL2_TTF_PATH} ${SDL2_IMAGE_PATH})
else()
endif()

# 2. Find official libraries
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_image REQUIRED)

# 3. Collect all source files
# This includes your main.cpp and the SDL2_gfx .c files you added
file(GLOB SOURCES "src/*.cpp" "src/gfx/*.c")
add_executable(${PROJECT_NAME} ${SOURCES})

# 4. Include Headers
target_include_directories(${PROJECT_NAME} PRIVATE "src/gfx")

target_include_directories(${PROJECT_NAME} PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    "${SDL2_PATH}/include/SDL2"  # Fixes legacy #include "SDL.h" inside gfx files
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE mingw32 SDL2::SDL2main SDL2::SDL2 SDL2_ttf::SDL2_ttf SDL2_image::SDL2_image)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-mwindows")
else()
    # Ubuntu Linker (much simpler)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2 SDL2_ttf SDL2_image SDL2_gfx)
endif()