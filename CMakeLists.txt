cmake_minimum_required(VERSION 3.21)
cmake_policy(SET CMP0135 NEW)

# --- CCACHE ---
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

# --- OPTIONS ---
option(WITH_GENICAM "Enable GenICam camera support via Aravis" OFF)

# --- ABILITAZIONE LINGUAGGI ---
# Su macOS dobbiamo abilitare OBJCXX altrimenti la property LANGUAGE viene ignorata
if(APPLE)
    project(Visical LANGUAGES C CXX OBJCXX)
else()
    project(Visical LANGUAGES C CXX)
endif()

# --- STANDARDS ---
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Garantisce lo standard puro (niente gnu++2b)
set(CMAKE_C_STANDARD 11)

# --- DEPENDENCIES ---
find_package(OpenCV CONFIG REQUIRED COMPONENTS core aruco videoio imgcodecs calib3d)
find_package(wxWidgets REQUIRED COMPONENTS core base)
include(FetchContent)
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

if(WITH_GENICAM)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(aravis REQUIRED IMPORTED_TARGET aravis-0.8)
endif()

# --- SORGENTI ---
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE HDR_FILES CONFIGURE_DEPENDS src/*.h src/*.hpp src/*.inl)

# --- FIX AVFOUNDATION MAC (Senza rinominare il file) ---
if(APPLE)
    # Diciamo a CMake di compilare il TUO wrapper .cpp come Objective-C++
    set_source_files_properties(
        "${CMAKE_CURRENT_SOURCE_DIR}/src/camera/interfaces/openCV/apple/AVFoundation.cpp" 
        PROPERTIES LANGUAGE OBJCXX
    )
endif()

# --- ESEGUIBILE ---
if(WIN32)
    add_executable(Visical WIN32 ${SRC_FILES} ${HDR_FILES} platform/windows/Resource.rc)
else()
    add_executable(Visical ${SRC_FILES} ${HDR_FILES}
        src/thread/Thread.inl)
    
    set_target_properties(Visical PROPERTIES
        INSTALL_RPATH ""
        BUILD_WITH_INSTALL_RPATH FALSE
        SKIP_BUILD_RPATH TRUE
    )
endif()

# --- PRECOMPILED HEADERS  ---
target_precompile_headers(Visical PRIVATE
    <wx/wx.h>
    <opencv2/opencv.hpp>
)

# --- INCLUDES ---
target_include_directories(Visical PRIVATE 
    src
    ${wxWidgets_INCLUDE_DIRS}
)

target_compile_definitions(Visical PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    ${wxWidgets_DEFINITIONS}
)

if(NOT MSVC)
    target_compile_options(Visical PRIVATE
        $<$<CONFIG:Release>:-O3>
    )
endif()

if(WIN32)
    target_compile_definitions(Visical PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
    target_compile_options(Visical PRIVATE /permissive-)
endif()

if(WITH_GENICAM)
    target_compile_definitions(Visical PRIVATE WITH_GENICAM)
endif()

# --- LINKING ---
target_link_libraries(Visical PRIVATE
    ${OpenCV_LIBS}
    ${wxWidgets_LIBRARIES}
    nlohmann_json::nlohmann_json
)

if(APPLE)
    target_link_libraries(Visical PRIVATE
        "-framework AVFoundation"
        "-framework CoreMedia"
        "-framework CoreVideo"
        "-framework Foundation"
    )
endif()

if(WITH_GENICAM)
    target_link_libraries(Visical PRIVATE PkgConfig::aravis)
endif()