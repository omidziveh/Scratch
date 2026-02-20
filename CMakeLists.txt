cmake_minimum_required(VERSION 3.10)
project(Blocky)
include_directories(${CMAKE_SOURCE_DIR}/gfx)

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

file(GLOB SOURCES "src/*.cpp" "src/backend/*.cpp" "src/frontend/*.cpp" "src/common/*.cpp" "src/utils/*.cpp" "src/gfx/*.c")

if(WIN32)
    list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/app.rc")
endif()

message(STATUS "Found source files:")
foreach(SOURCE_FILE ${SOURCES})
    message(STATUS "  ${SOURCE_FILE}")
endforeach()

add_executable(${PROJECT_NAME} ${SOURCES})

# ✅ این خط باید بعد از add_executable باشد!
target_link_options(${PROJECT_NAME} PRIVATE -Wl,--allow-multiple-definition)

# 5. Include Headers
target_include_directories(${PROJECT_NAME} PRIVATE "src/gfx")

target_include_directories(${PROJECT_NAME} PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    "${SDL2_PATH}/include/SDL2"
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE mingw32 SDL2::SDL2main SDL2::SDL2 SDL2_ttf::SDL2_ttf SDL2_image::SDL2_image SDL2_mixer)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-mwindows")
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2 SDL2_ttf SDL2_image SDL2_gfx SDL2_mixer)
endif()
