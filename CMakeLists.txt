cmake_minimum_required(VERSION 3.11)

project(transport_catalogue CXX)
set(CMAKE_CXX_STANDARD 20)


set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(DOMAIN_DIR "src/domain")
set(JSON_DIR "src/json")
set(SVG_DIR "src/svg")

#---------------------------------------------------------
#                      PROGRAM
#---------------------------------------------------------
add_executable(transport_catalogue
        src/main.cpp
        src/transport_catalogue.h
        src/transport_catalogue.cpp
        src/transport_router.h
        src/transport_router.cpp
        ${DOMAIN_DIR}/domain.h
        ${DOMAIN_DIR}/geo.h
        ${DOMAIN_DIR}/geo.cpp
        ${DOMAIN_DIR}/ranges.h
        ${JSON_DIR}/json.h
        ${JSON_DIR}/json.cpp
        ${JSON_DIR}/json_builder.h
        ${JSON_DIR}/json_builder.cpp
        ${JSON_DIR}/json_reader.h
        ${JSON_DIR}/json_reader.cpp
        ${SVG_DIR}/svg.h
        ${SVG_DIR}/svg.cpp
        ${SVG_DIR}/map_renderer.h
        ${SVG_DIR}/map_renderer.cpp
)

target_include_directories(transport_catalogue PRIVATE src)
