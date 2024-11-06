cmake_minimum_required(VERSION 3.5)

project(opengl_learn)

include_directories(./include ./common ./task ./old_common)

link_directories(./lib)

aux_source_directory(./common COMMON_LIST)

# add_executable(homework_1 src/homework_1.cpp src/glad.c)
# target_link_libraries(homework_1 glfw3)

add_executable(homework_2 src/homework_2.cpp src/glad.c task/sphere.cpp ${COMMON_LIST})
target_link_libraries(homework_2 glfw3 libassimpd)

# add_executable(class_1 learn/class1_window.cpp src/glad.c)
# target_link_libraries(class_1 glfw3)

# add_executable(class_2 learn/class2_triangle.cpp src/glad.c)
# target_link_libraries(class_2 glfw3)

# add_executable(class_3 learn/class3_matrices.cpp src/glad.c)
# target_link_libraries(class_3 glfw3)

# add_executable(class_4 learn/class4_colored_cube.cpp src/glad.c)
# target_link_libraries(class_4 glfw3)

# add_executable(class_5 learn/class5_textured_cube.cpp src/glad.c)
# target_link_libraries(class_5 glfw3)

# add_executable(class_6 learn/class6_keyboard_mouse.cpp src/glad.c ${COMMON_LIST})
# target_link_libraries(class_6 glfw3)

# add_executable(class_7 learn/class7_model_loading.cpp src/glad.c ${COMMON_LIST})
# target_link_libraries(class_7 glfw3)

# add_executable(class_8 learn/class8_shading.cpp src/glad.c task/sphere.cpp ${COMMON_LIST})
# target_link_libraries(class_8 glfw3 libassimpd)

# add_executable(class_9 learn/class9_material.cpp src/glad.c task/sphere.cpp ${COMMON_LIST})
# target_link_libraries(class_9 glfw3 libassimpd)

# add_executable(class_10 learn/class10_multilight.cpp src/glad.c task/sphere.cpp ${COMMON_LIST})
# target_link_libraries(class_10 glfw3 libassimpd)

# add_executable(class_11 learn/class11_model.cpp src/glad.c task/sphere.cpp ${COMMON_LIST})
# target_link_libraries(class_11 glfw3 libassimpd)

# add_executable(class_12 learn/class12_stencil_testing.cpp src/glad.c task/sphere.cpp ${COMMON_LIST})
# target_link_libraries(class_12 glfw3 libassimpd)

# add_executable(class_13 learn/class13_blending.cpp src/glad.c task/sphere.cpp ${COMMON_LIST})
# target_link_libraries(class_13 glfw3 libassimpd)

# add_executable(class_14 learn/class14_skybox.cpp src/glad.c task/sphere.cpp ${COMMON_LIST})
# target_link_libraries(class_14 glfw3 libassimpd)

add_executable(class_15 learn/class15_PBR.cpp src/glad.c task/sphere.cpp ${COMMON_LIST})
target_link_libraries(class_15 glfw3 libassimpd)

add_executable(class_16 learn/class16_PBR_IBL.cpp src/glad.c task/sphere.cpp ${COMMON_LIST})
target_link_libraries(class_16 glfw3 libassimpd)

add_custom_target(copy_assimp_dll ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/bin/libassimp-5d.dll"
    "${PROJECT_SOURCE_DIR}/build"
    COMMENT "Copying libassimp-5d.dll to build directory"
)
