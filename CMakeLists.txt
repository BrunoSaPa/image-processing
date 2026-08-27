cmake_minimum_required(VERSION 3.20)

project(ComputerVisionCourse LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_EXERCISES "Build the exercises under exercises/" ON)

if(BUILD_EXERCISES)
    find_package(OpenCV REQUIRED)

    function(add_cv_exercise target source)
        add_executable(${target} ${source})

        target_include_directories(
            ${target}
            PRIVATE
            ${OpenCV_INCLUDE_DIRS}
        )

        target_link_libraries(
            ${target}
            PRIVATE
            ${OpenCV_LIBS}
        )
    endfunction()

    # New exercises are picked up automatically when they follow this layout:
    #
    #   exercises/<name>/main.cpp
    #
    # CONFIGURE_DEPENDS asks CMake to re-run configuration when a new matching
    # source file is added, so adding an exercise does not require editing this
    # file.
    file(
        GLOB exercise_sources
        CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/exercises/*/main.cpp"
    )
    list(SORT exercise_sources)

    if(NOT exercise_sources)
        message(FATAL_ERROR
            "No exercises found. Add exercises/<name>/main.cpp and configure again."
        )
    endif()

    set(exercise_targets)

    foreach(exercise_source IN LISTS exercise_sources)
        get_filename_component(exercise_directory "${exercise_source}" DIRECTORY)
        get_filename_component(exercise_name "${exercise_directory}" NAME)
        string(MAKE_C_IDENTIFIER "${exercise_name}" exercise_target)

        if(TARGET "${exercise_target}")
            message(FATAL_ERROR
                "Exercise target '${exercise_target}' is defined more than once."
            )
        endif()

        add_cv_exercise("${exercise_target}" "${exercise_source}")
        list(APPEND exercise_targets "${exercise_target}")
    endforeach()

    string(JOIN ", " exercise_target_list ${exercise_targets})
    message(STATUS "Configured exercises: ${exercise_target_list}")
endif()
