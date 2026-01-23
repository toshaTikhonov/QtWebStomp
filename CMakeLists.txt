cmake_minimum_required(VERSION 3.16)

project(QTWebStompClient VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable automatic MOC, UIC, and RCC processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt packages - try Qt6 first, fallback to Qt5
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core WebSockets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core WebSockets)

message(STATUS "Building with Qt${QT_VERSION_MAJOR}")

# Option to build as static or shared library
option(BUILD_SHARED_LIBS "Build as shared library" OFF)

# Source files
set(SOURCES
    QTWebStompClientDll.cpp
)

# Header files
set(HEADERS
    QTWebStompClientDll.h
    StompMessage.h
)

# Create library (shared or static based on option)
if(BUILD_SHARED_LIBS)
    add_library(${PROJECT_NAME} SHARED
        ${SOURCES}
        ${HEADERS}
    )
else()
    add_library(${PROJECT_NAME} STATIC
        ${SOURCES}
        ${HEADERS}
    )
endif()

# Add compile definitions based on library type
if(BUILD_SHARED_LIBS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        SHARED_LIBRARY
        QTWEBSTOMPCLIENTDLL_LIB
        QT_DEPRECATED_WARNINGS
    )
else()
    target_compile_definitions(${PROJECT_NAME} PUBLIC
        BUILD_STATIC
        QT_DEPRECATED_WARNINGS
    )
endif()

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::WebSockets
)

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Installation rules
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${HEADERS}
    DESTINATION include
)

# Option to build test project and examples
option(BUILD_TESTS "Build test project and examples" ON)

if(BUILD_TESTS)
    message(STATUS "Building test project and examples")
    add_subdirectory(TestProject/WebStompTestProject)
    add_subdirectory(examples/embedded)
endif()