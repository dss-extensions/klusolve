# Copyright (C) 2012-2019 Paulo Meira
# This augmented KLUSolve CMakeLists.txt is based on a personal script to
# build SuiteSparse on Windows

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(klusolvex)
SET(CMAKE_DEBUG_POSTFIX "")
SET(CMAKE_RELEASE_PREFIX "")

SET(USE_SYSTEM_SUITESPARSE ON CACHE BOOL "Use system SuiteSparse.")
SET(USE_SYSTEM_EIGEN3 ON CACHE BOOL "Use system Eigen3.")

if ("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    SET(KLUSOLVE_OUT_DIR_SUFFIX "x64")
else ()
    SET(KLUSOLVE_OUT_DIR_SUFFIX "x86")
endif ()

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    SET(KLUSOLVE_OUT_DIR_PREFIX "win")
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(CMAKE_CXX_STANDARD 11)
    SET(KLUSOLVE_OUT_DIR_PREFIX "darwin")
elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    SET(KLUSOLVE_OUT_DIR_PREFIX "linux")
else ()
    MESSAGE(FATAL_ERROR "Unsupported system? ${CMAKE_SYSTEM_NAME}" )
endif ()

SET(KLUSOLVE_OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/${KLUSOLVE_OUT_DIR_PREFIX}_${KLUSOLVE_OUT_DIR_SUFFIX}")
add_definitions("-Dcsi=int")

SET(KLUSOLVE_SRC
    src/KLUSolveX.cpp
    src/KLUSystemX.cpp
    src/mvmult.cpp
    src/klusolve_metis.c
)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.4)
        set (CMAKE_CXX_STANDARD 11)
    endif()
endif()

# Best results for GCC and MSVC, used only on the Release build
if (NOT MSVC)
    # set_property(GLOBAL PROPERTY COMPILE_DEFINITIONS_RELEASE "-DNDEBUG -O2 -ftree-vectorize -funroll-loops -march=corei7")
    add_definitions("-DNDEBUG -O2 -ftree-vectorize -funroll-loops -march=corei7")
else()
    # set_property(GLOBAL PROPERTY COMPILE_DEFINITIONS_RELEASE "/DNDEBUG /O2 /Ot /GS- /openmp")
    add_definitions("/DNDEBUG /Ot /GS- /openmp") # removed /O2 since it's the default
endif()

# Handle Eigen3
# Eigen3 is a header-only library, no building necessary
if (USE_SYSTEM_EIGEN)
    find_package(Eigen3 REQUIRED)
    include_directories(${EIGEN3_INCLUDE_DIR})
else ()
    IF (EXISTS "$ENV{EIGEN3_SRC}/Eigen/Eigen")
        SET(EIGEN3_DIR $ENV{EIGEN3_SRC})
        if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
            STRING(REGEX REPLACE "\\\\" "/" EIGEN3_DIR ${EIGEN3_DIR})
        endif()
    ELSE()
        IF (NOT EXISTS "${CMAKE_BINARY_DIR}/eigen-eigen-323c052e1731/Eigen/Eigen")
            message(STATUS "Downloading Eigen3 source code...")
            file(DOWNLOAD "http://bitbucket.org/eigen/eigen/get/3.3.7.tar.gz" "${CMAKE_BINARY_DIR}/eigen3.tar.gz")
            message(STATUS "Unpacking Eigen3...")
            execute_process(COMMAND ${CMAKE_COMMAND} -E tar zxf "eigen3.tar.gz" WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
        ELSE()
            message(STATUS "Eigen3 already exists, skipping download.")
        ENDIF()
        SET(EIGEN3_DIR "${CMAKE_BINARY_DIR}/eigen-eigen-323c052e1731")
    ENDIF()
    include_directories(${EIGEN3_DIR})
endif ()


if (USE_SYSTEM_SUITESPARSE)
    find_path(SUITESPARSE_INCLUDE_DIR NAMES klu.h HINTS /usr/include /usr/include/suitesparse /usr/local/include /usr/local/include/suitesparse)

    if (NOT SUITESPARSE_INCLUDE_DIR)
        message(FATAL_ERROR "KLU header not found. Please ensure SuiteSparse development package is properly installed. You can also set USE_SYSTEM_SUITESPARSE=OFF to download the official source code automatically.")
    endif ()

    find_library(SUITESPARSE_KLU_LIB NAMES klu)
    find_library(SUITESPARSE_AMD_LIB NAMES amd)
    find_library(SUITESPARSE_BTF_LIB NAMES btf)
    find_library(SUITESPARSE_METIS_LIB NAMES metis)
    find_library(SUITESPARSE_COLAMD_LIB NAMES colamd)
    set(KLU_LIBRARIES ${SUITESPARSE_KLU_LIB} ${SUITESPARSE_AMD_LIB} ${SUITESPARSE_BTF_LIB} ${SUITESPARSE_COLAMD_LIB} ${SUITESPARSE_METIS_LIB})

    if (NOT SUITESPARSE_KLU_LIB)
        message(FATAL_ERROR "KLU header not found. Please ensure SuiteSparse development package is properly installed.")
    endif ()


    if (NOT MSVC)
        if (NOT MINGW)
            SET(FPIC "-fPIC")
        endif ()
        if (APPLE)
            # MacOS doesn't support linking statically with system libraries
            set(CMAKE_SHARED_LINKER_FLAGS "-fPIC")
        else()
            set(CMAKE_SHARED_LINKER_FLAGS "-static-libgcc -static-libstdc++ ${FPIC} -Wl,--version-script,\"${CMAKE_CURRENT_SOURCE_DIR}/src/klusolvex.map\"")
        endif()
    endif()

    add_library(klusolve SHARED ${KLUSOLVE_SRC} src/klusolvex.def)
    target_link_libraries(klusolve ${KLU_LIBRARIES})
    include_directories(${SUITESPARSE_INCLUDE_DIR})
else ()
    IF (EXISTS "$ENV{SUITESPARSE_SRC}/SuiteSparse_config/SuiteSparse_config.h")
        SET(SUITESPARSE_DIR $ENV{SUITESPARSE_SRC})
        if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
            STRING(REGEX REPLACE "\\\\" "/" SUITESPARSE_DIR ${SUITESPARSE_DIR})
        endif()
    ELSE()
        IF(NOT EXISTS "${CMAKE_BINARY_DIR}/SuiteSparse/SuiteSparse_config/SuiteSparse_config.h")
            message(STATUS "Downloading SuiteSparse source code...")
            file(DOWNLOAD "https://github.com/DrTimothyAldenDavis/SuiteSparse/archive/v5.6.0.tar.gz" "${CMAKE_BINARY_DIR}/suitesparse.tar.gz")
            message(STATUS "Unpacking SuiteSparse...")
            execute_process(COMMAND ${CMAKE_COMMAND} -E tar zxf "suitesparse.tar.gz" WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
            file(RENAME "${CMAKE_BINARY_DIR}/SuiteSparse-5.6.0" "${CMAKE_BINARY_DIR}/SuiteSparse")
        ELSE()
            message(STATUS "SuiteSparse already exists, skipping download.")
        ENDIF()
        SET(SUITESPARSE_DIR "${CMAKE_BINARY_DIR}/SuiteSparse")
    ENDIF()
    if (MINGW)
        # Ideally the patch should be merged upstream, either in SuiteSparse's
        # patched code, or in the original METIS (no updates since 2013)
        find_package(Patch)
        if(NOT Patch_FOUND)
            message(FATAL_ERROR "Patch executable is required for METIS")
        endif()
        message(STATUS "${Patch_EXECUTABLE} --input=${CMAKE_CURRENT_SOURCE_DIR}/src/metis.patch WORKING_DIRECTORY ${SUITESPARSE_DIR}")
        execute_process(COMMAND ${Patch_EXECUTABLE} "--input=${CMAKE_CURRENT_SOURCE_DIR}/src/metis.patch" "-p0" WORKING_DIRECTORY ${SUITESPARSE_DIR})
    endif()

    # METIS
    set(GKLIB_PATH "${SUITESPARSE_DIR}/metis-5.1.0/GKlib" CACHE PATH "Forced GKLIB path fix" FORCE)
    include(CheckIncludeFile)
    check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)

    add_subdirectory("${SUITESPARSE_DIR}/metis-5.1.0/" "METIS")
    include_directories(
        "${SUITESPARSE_DIR}/AMD/Include/"
        "${SUITESPARSE_DIR}/COLAMD/Include/"
        "${SUITESPARSE_DIR}/BTF/Include/"
        "${SUITESPARSE_DIR}/KLU/Include/"
        "${SUITESPARSE_DIR}/SuiteSparse_config/"
        "${SUITESPARSE_DIR}/metis-5.1.0/include/"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/"
        # "${SUITESPARSE_DIR}/CHOLMOD/Include/"
    )

    #SET(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    add_definitions("-DSUITESPARSE_DLLEXPORT=__declspec(dllexport)")
    set(CMAKE_DEBUG_POSTFIX "d")

    file(GLOB AMD_TEMPLATE_SRC "${SUITESPARSE_DIR}/AMD/Source/*.c")
    file(GLOB COLAMD_TEMPLATE_SRC "${SUITESPARSE_DIR}/COLAMD/Source/*.c")
    file(GLOB BTF_TEMPLATE_SRC "${SUITESPARSE_DIR}/BTF/Source/*.c")
    list(FILTER AMD_TEMPLATE_SRC EXCLUDE REGEX .*amd_global.*)
    set(AMD_GLOBAL "${SUITESPARSE_DIR}/AMD/Source/amd_global.c")
    
    add_library(AMD_I OBJECT ${AMD_TEMPLATE_SRC})
    add_library(COLAMD_I OBJECT ${COLAMD_TEMPLATE_SRC})
    add_library(BTF_I OBJECT ${BTF_TEMPLATE_SRC})
    add_library(AMD_L OBJECT ${AMD_TEMPLATE_SRC})
    add_library(COLAMD_L OBJECT ${COLAMD_TEMPLATE_SRC})
    add_library(BTF_L OBJECT ${BTF_TEMPLATE_SRC})
    target_compile_options(AMD_L PRIVATE "-DDLONG")
    target_compile_options(COLAMD_L PRIVATE "-DDLONG")
    target_compile_options(BTF_L PRIVATE "-DDLONG")

    #set_source_files_properties(${BTF_I} PROPERTIES COMPILE_FLAGS -DDINT)
    set_source_files_properties(${BTF_L} PROPERTIES COMPILE_FLAGS -DDLONG)

    set(
        SUITESPARSE_OBJS 
        $<TARGET_OBJECTS:AMD_I>
        $<TARGET_OBJECTS:COLAMD_I>
        $<TARGET_OBJECTS:BTF_I>
        $<TARGET_OBJECTS:AMD_L>
        $<TARGET_OBJECTS:COLAMD_L>
        $<TARGET_OBJECTS:BTF_L>
    )


    #KLU
    set(KLU_SRC "${SUITESPARSE_DIR}/KLU/Source")

    set(KLU_TEMPLATE_SRC
        "${KLU_SRC}/klu.c"
        "${KLU_SRC}/klu_diagnostics.c"
        "${KLU_SRC}/klu_dump.c"
        "${KLU_SRC}/klu_extract.c"
        "${KLU_SRC}/klu_factor.c"
        "${KLU_SRC}/klu_kernel.c"
        "${KLU_SRC}/klu_refactor.c"
        "${KLU_SRC}/klu_scale.c"
        "${KLU_SRC}/klu_solve.c"
        "${KLU_SRC}/klu_sort.c"
        "${KLU_SRC}/klu_tsolve.c"
        "${KLU_SRC}/klu_free_numeric.c")

    set(KLU_COMMON_SRC
        "${KLU_SRC}/klu_analyze.c"
        "${KLU_SRC}/klu_analyze_given.c"
        "${KLU_SRC}/klu_defaults.c"
        "${KLU_SRC}/klu_free_symbolic.c"
        "${KLU_SRC}/klu_memory.c"
    )

    # add_library(KLU_D OBJECT ${KLU_TEMPLATE_SRC})
    add_library(KLU_Z OBJECT ${KLU_TEMPLATE_SRC})
    # add_library(KLU_ZL OBJECT ${KLU_TEMPLATE_SRC})
    # add_library(KLU_DL OBJECT ${KLU_TEMPLATE_SRC})
    add_library(KLU_I OBJECT ${KLU_COMMON_SRC})
    add_library(KLU_L OBJECT ${KLU_COMMON_SRC})
    
    # target_compile_options(KLU_D PRIVATE -DDINT)
    target_compile_options(KLU_Z PRIVATE -DCOMPLEX -DDINT)
    # target_compile_options(KLU_ZL PRIVATE -DCOMPLEX -DDLONG)
    # target_compile_options(KLU_DL PRIVATE -DDLONG)
    target_compile_options(KLU_L PRIVATE -DDLONG)

    set(
        KLU_OBJS 
        $<TARGET_OBJECTS:KLU_Z>
        $<TARGET_OBJECTS:KLU_I>
        $<TARGET_OBJECTS:KLU_L>
    )

    if (NOT MSVC)
        if (APPLE)
            # MacOS doesn't support linking statically with system libraries
            set(CMAKE_SHARED_LINKER_FLAGS "-fPIC")
        else()
            set(CMAKE_SHARED_LINKER_FLAGS "-static-libgcc -static-libstdc++ -fPIC -Wl,--version-script,\"${CMAKE_CURRENT_SOURCE_DIR}/src/klusolvex.map\"")
        endif()
        add_definitions(-DNCOMPLEX -fPIC)
    else()
        add_definitions(-DNCOMPLEX)
    endif()

    SET(SUITESPARSE_SRC "${SUITESPARSE_DIR}/SuiteSparse_config/SuiteSparse_config.c" ${COLAMD_GLOBAL} ${AMD_GLOBAL})
    add_library(klusolve SHARED ${KLU_OBJS} ${SUITESPARSE_OBJS} ${SUITESPARSE_SRC} ${KLUSOLVE_SRC} src/klusolvex.def)

    target_link_libraries(klusolve PUBLIC metis)
    include_directories(
        "${SUITESPARSE_DIR}/AMD/Include/"
        "${SUITESPARSE_DIR}/COLAMD/Include/"
        "${SUITESPARSE_DIR}/BTF/Include/"
        "${SUITESPARSE_DIR}/KLU/Include/"
        "${SUITESPARSE_DIR}/SuiteSparse_config/"
        "${SUITESPARSE_DIR}/SuiteSparse_config/Include"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    )
endif (USE_SYSTEM_SUITESPARSE)

if (MSVC)
    set_target_properties(klusolve PROPERTIES OUTPUT_NAME libklusolvex)
else ()
    set_target_properties(klusolve PROPERTIES OUTPUT_NAME klusolvex)
endif ()

set_target_properties(klusolve PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${KLUSOLVE_OUT_DIR})
set_target_properties(klusolve PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${KLUSOLVE_OUT_DIR})
set_target_properties(klusolve PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${KLUSOLVE_OUT_DIR})

if (MSVC)
    # Force the Release output to be in the expected folder
    set_target_properties(klusolve PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE ${KLUSOLVE_OUT_DIR})
    set_target_properties(klusolve PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${KLUSOLVE_OUT_DIR})
    set_target_properties(klusolve PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${KLUSOLVE_OUT_DIR})
endif ()
