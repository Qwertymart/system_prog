cmake_minimum_required(VERSION 3.26)
project(system_prog C)

set(CMAKE_C_STANDARD 11)

add_executable(system_prog lab_1/task_1/main.c
        lab_1/task_7/main.c
        lab_1/task_6/server.c
        lab_1/task_6/client.c
        lab_1/task_6/client.c
        lab_1/task_5/main.c
        lab_1/task_3/main.c
        lab_1/task_2/main.c
)
