cmake_minimum_required(VERSION 3.10)


project(Game_Engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
file(GLOB_RECURSE SOURCE "./src/*.cpp")
file(GLOB GLAD_SOURCE "./src/*.c")


add_executable(Game_Engine ${SOURCE} ${GLAD_SOURCE})


target_include_directories(Game_Engine PRIVATE "./Dependencies/include/")
target_include_directories(Game_Engine PRIVATE "./Dependencies/include/entt/")
target_include_directories(Game_Engine PRIVATE "./Dependencies/include/imgui/")
target_include_directories(Game_Engine PRIVATE "./src/")

add_library(freetype STATIC IMPORTED)

add_library(yaml STATIC IMPORTED)

add_library(box2d STATIC IMPORTED)

add_library(fmod SHARED IMPORTED)

add_library(glfw STATIC IMPORTED)

add_library(opengl STATIC IMPORTED)

add_library(assimp STATIC IMPORTED)

add_library(BulletDynamics STATIC IMPORTED)

add_library(BulletCollision STATIC IMPORTED)

add_library(LinearMath STATIC IMPORTED)


set_target_properties(fmod PROPERTIES IMPORTED_LOCATION "./Dependencies/lib/fmod.dll" IMPORTED_IMPLIB "./Dependencies/lib/fmod_vc.lib")

set_target_properties(box2d PROPERTIES IMPORTED_LOCATION "./Dependencies/lib/box2d.lib")

set_target_properties(yaml PROPERTIES IMPORTED_LOCATION "./Dependencies/lib/yamld.lib")

set_target_properties(freetype PROPERTIES IMPORTED_LOCATION "./Dependencies/lib/freetype.lib")

set_target_properties(glfw PROPERTIES IMPORTED_LOCATION "./Dependencies/lib/glfw3.lib")

set_target_properties(opengl PROPERTIES IMPORTED_LOCATION "./Dependencies/lib/opengl32.lib")

set_target_properties(assimp PROPERTIES IMPORTED_LOCATION "./Dependencies/lib/assimp-vc143-mt.lib")

set_target_properties(BulletDynamics PROPERTIES IMPORTED_LOCATION "./Dependencies/lib/BulletDynamics_Debug.lib")

set_target_properties(BulletCollision PROPERTIES IMPORTED_LOCATION "./Dependencies/lib/BulletCollision_Debug.lib")

set_target_properties(LinearMath PROPERTIES IMPORTED_LOCATION "./Dependencies/lib/LinearMath_Debug.lib")

target_link_libraries(Game_Engine PRIVATE opengl)
target_link_libraries(Game_Engine PRIVATE glfw)
target_link_libraries(Game_Engine PRIVATE freetype)
target_link_libraries(Game_Engine PRIVATE yaml)
target_link_libraries(Game_Engine PRIVATE fmod)
target_link_libraries(Game_Engine PRIVATE box2d)
target_link_libraries(Game_Engine PRIVATE assimp)
target_link_libraries(Game_Engine PRIVATE BulletDynamics)
target_link_libraries(Game_Engine PRIVATE BulletCollision)
target_link_libraries(Game_Engine PRIVATE LinearMath)



set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Game_Engine)

