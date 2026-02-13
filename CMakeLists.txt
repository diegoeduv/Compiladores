cmake_minimum_required(VERSION 3.10)
project(compiler)

# Estándar de C
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Flags del compilador
if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -g)
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Encontrar bibliotecas
# find_package({NOMBRE DE LA BIBLIOTECA} REQUIRED)

# Buscar todos los archivos fuente automáticamente
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.c)   

# Ejecutable
add_executable(${PROJECT_NAME} ${SRC_FILES})
    
# Librerías
# Linkear las bibliotecas encontradas
# target_link_libraries(${PROJECT_NAME} 
# 
# )