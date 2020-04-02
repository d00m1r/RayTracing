cmake_minimum_required (VERSION 3.8)

set (PROJECT rt)
set (SOURCE_EXE main.cpp)
set (HEADERS 
            BMP/EasyBMP.h
            BMP/EasyBMP.cpp
            window.hpp
            ray.hpp
            effects.hpp
            material.hpp
            object.hpp
                    )

include_directories(
	glm-0.9.7.1/
    BMP/
    argparse/
)
project (${PROJECT})

# get rid of annoying MSVC warnings.
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

include(CheckCXXCompilerFlag)

function(enable_cxx_compiler_flag_if_supported flag)
    string(FIND "${CMAKE_CXX_FLAGS}" "${flag}" flag_already_set)
    if(flag_already_set EQUAL -1)
        check_cxx_compiler_flag("${flag}" flag_supported)
        if(flag_supported)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
        endif()
        unset(flag_supported CACHE)
    endif()
endfunction()

     
enable_cxx_compiler_flag_if_supported("-Wall")
enable_cxx_compiler_flag_if_supported("-Wextra")
enable_cxx_compiler_flag_if_supported("-pedantic")
enable_cxx_compiler_flag_if_supported("-std=c++11")
enable_cxx_compiler_flag_if_supported("-O3")
enable_cxx_compiler_flag_if_supported("-fopenmp")

add_executable(${PROJECT_NAME} ${SOURCE_EXE} ${HEADERS})

