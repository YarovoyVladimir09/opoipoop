cmake_minimum_required(VERSION 3.23)
project(TransportCatalogue)

set(CMAKE_CXX_STANDARD 17)

add_executable(TransportCatalogue main.cpp domain.cpp geo.cpp json.cpp json_reader.cpp json_builder.cpp map_renderer.cpp request_handler.cpp
        svg.cpp transport_catalogue.cpp transport_router.cpp)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")