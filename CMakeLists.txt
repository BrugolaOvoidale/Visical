cmake_minimum_required(VERSION 3.21)

# Optional GenICam feature
option(WITH_GENICAM "Enable GenICam camera support via Aravis" OFF)

if(WITH_GENICAM)
    list(APPEND VCPKG_MANIFEST_FEATURES "genicam")
endif()

project(Visical LANGUAGES C CXX)


# Standards

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

find_package(OpenCV CONFIG REQUIRED)
find_package(wxWidgets CONFIG REQUIRED COMPONENTS core base)
find_package(nlohmann_json CONFIG REQUIRED)
if(WITH_GENICAM)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(aravis REQUIRED IMPORTED_TARGET aravis-0.8)
endif()


# Sources

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    src/*.cpp
)

file(GLOB_RECURSE HDR_FILES CONFIGURE_DEPENDS
    src/*.h
    src/*.hpp
    src/*.inl
)


# Executable

if(WIN32)
    add_executable(Visical WIN32 ${SRC_FILES} ${HDR_FILES} platform/windows/Resource.rc)
else()
    add_executable(Visical ${SRC_FILES} ${HDR_FILES})

    set_target_properties(Visical PROPERTIES
        INSTALL_RPATH ""
        BUILD_WITH_INSTALL_RPATH FALSE
        SKIP_BUILD_RPATH TRUE
    )

endif()


# Includes

target_include_directories(Visical PRIVATE
    src
)


# Compile definitions

target_compile_definitions(Visical PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

if(WIN32)
    target_compile_definitions(Visical PRIVATE
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
    )

    target_compile_options(Visical PRIVATE
        /permissive-
    )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        set(CMAKE_CXX23_STANDARD_COMPILE_OPTION "/std:c++latest")
        set(CMAKE_CXX23_EXTENSION_COMPILE_OPTION "/std:c++latest")
    else()
        set(CMAKE_CXX23_STANDARD_COMPILE_OPTION "-std=c++23")
        set(CMAKE_CXX23_EXTENSION_COMPILE_OPTION "-std=gnu++23")
    endif()
endif()

if(WITH_GENICAM)
    target_compile_definitions(Visical PRIVATE WITH_GENICAM)
endif()


# Linking

target_link_libraries(Visical PRIVATE
    opencv_core
    opencv_imgproc
    opencv_highgui
    opencv_calib3d
    opencv_features2d
	
    wx::core
    wx::base
	
	nlohmann_json::nlohmann_json
)

if(WITH_GENICAM)
	target_link_libraries(Visical PRIVATE PkgConfig::aravis)
endif()
