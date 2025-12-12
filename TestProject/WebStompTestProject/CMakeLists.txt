cmake_minimum_required(VERSION 3.16)

project(WebStompTestProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable automatic MOC, UIC, and RCC processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Use the same Qt version as the parent project
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

# Source files
set(SOURCES
    main.cpp
)

# Create executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
)

# Link with Qt libraries and QTWebStompClient library
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    QTWebStompClient
)

# Include directories - point to the root directory where headers are located
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}
)