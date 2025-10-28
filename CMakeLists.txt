cmake_minimum_required(VERSION 3.10)


project(Game_Engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
file(GLOB_RECURSE SOURCE "./src/*.cpp")
file(GLOB GLAD_SOURCE "./src/*.c")

set(INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/include/)
set(LIBPATH ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/lib/)


add_executable(Game_Engine ${SOURCE} ${GLAD_SOURCE})


target_include_directories(Game_Engine PRIVATE "${INCLUDE}")
target_include_directories(Game_Engine PRIVATE "${INCLUDE}entt/")
target_include_directories(Game_Engine PRIVATE "${INCLUDE}imgui/")
target_include_directories(Game_Engine PRIVATE "${INCLUDE}imgui/backends/")
target_include_directories(Game_Engine PRIVATE "${INCLUDE}imguizmo/")
target_include_directories(Game_Engine PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/")





set(IMGUI_DIR ${INCLUDE}/imgui)

add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    
    # --- ADD YOUR BACKENDS HERE! ---
    # You MUST include the .cpp files for your renderer and platform.
    # This example uses GLFW and OpenGL3.
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PRIVATE "${INCLUDE}imgui/")
target_include_directories(imgui PRIVATE "${INCLUDE}/")


set(IMGUIZMO_DIR ${INCLUDE}/imguizmo)

add_library(imguizmo STATIC
    ${IMGUIZMO_DIR}/ImGuizmo.cpp
)

target_include_directories(imguizmo PRIVATE ${IMGUIZMO_DIR})
target_include_directories(imguizmo PRIVATE ${IMGUI_DIR})

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


set_target_properties(fmod PROPERTIES IMPORTED_LOCATION "${LIBPATH}fmod.dll" IMPORTED_IMPLIB "${LIBPATH}fmod_vc.lib")

set_target_properties(box2d PROPERTIES IMPORTED_LOCATION "${LIBPATH}box2d.lib")

set_target_properties(yaml PROPERTIES IMPORTED_LOCATION "${LIBPATH}yaml-cppd.lib")

set_target_properties(freetype PROPERTIES IMPORTED_LOCATION "${LIBPATH}freetype.lib")

set_target_properties(glfw PROPERTIES IMPORTED_LOCATION "${LIBPATH}glfw3.lib")

set_target_properties(opengl PROPERTIES IMPORTED_LOCATION "opengl32.lib")

set_target_properties(assimp PROPERTIES IMPORTED_LOCATION "${LIBPATH}assimp-vc143-mt.lib")

set_target_properties(BulletDynamics PROPERTIES IMPORTED_LOCATION "${LIBPATH}BulletDynamics_Debug.lib")

set_target_properties(BulletCollision PROPERTIES IMPORTED_LOCATION "${LIBPATH}BulletCollision_Debug.lib")

set_target_properties(LinearMath PROPERTIES IMPORTED_LOCATION "${LIBPATH}LinearMath_Debug.lib")

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
target_link_libraries(Game_Engine PRIVATE imgui)
target_link_libraries(Game_Engine PRIVATE imguizmo)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Game_Engine)


set(ASSIMP_DLL ${LIBPATH}assimp-vc143-mt.dll)
set(FMOD_DLL ${LIBPATH}fmod.dll)

add_custom_command (
    TARGET Game_Engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ASSIMP_DLL}"
        "$<TARGET_FILE_DIR:Game_Engine>"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${FMOD_DLL}"
        "$<TARGET_FILE_DIR:Game_Engine>"

    COMMENT "Copying assimp and fmod dll files..."
)

set_property(TARGET Game_Engine PROPERTY DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")