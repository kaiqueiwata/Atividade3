project(ataqueATerra)

add_executable(${PROJECT_NAME} main.cpp openglwindow.cpp enemies.cpp
                               bullets.cpp ship.cpp starlayers.cpp)

enable_abcg(${PROJECT_NAME})
