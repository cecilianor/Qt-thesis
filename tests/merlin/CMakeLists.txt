add_library(merlin_lib
	include/Bach/Merlin/Merlin.h
        Merlin.cpp)
target_link_libraries(merlin_lib PUBLIC maplib)
target_include_directories(merlin_lib PUBLIC include)
set(RENDER_OUTPUT_TESTER_BASELINE_RELATIVE_PATH "unitTestResources/RenderOutputTesterBaseline")
target_compile_definitions(
	merlin_lib
	PUBLIC
        BACH_RENDEROUTPUT_BASELINE_DIR="${CMAKE_SOURCE_DIR}/${RENDER_OUTPUT_TESTER_BASELINE_RELATIVE_PATH}")
qt_add_executable(
	merlin_rebuild_baseline
	rebuild_baseline.cpp)
target_link_libraries(merlin_rebuild_baseline PUBLIC merlin_lib maplib)


# There's a problem when using ImageMagick on Ubuntu and Windows.
# At the time of writing, the commonly available ImagMagick version on Windows is
# v7. Ubuntu 22.04 only ships with v6. These two versions differ slightly on how the
# comparison command is used and this code attempts to detect the difference, and
# pass appropriate data to the C++ code.
find_program(image_magick NAMES magick)
if (NOT image_magick)
	find_program(image_magick NAMES compare REQUIRED)
	# We found the "compare" executable
	set(USE_IMAGEMAGICK_V7 false)
else()
	# Image magick program "magick" found
	set(USE_IMAGEMAGICK_V7 true)
endif()
qt_add_executable(
	merlin_rendering_output_tests
	rendering_output_tests.cpp)
target_link_libraries(merlin_rendering_output_tests PUBLIC merlin_lib maplib Qt6::Test)
# Make sure our variable is set to either 'true' or 'false' since it
# has to be passed into our C++ code as boolean.
if (NOT USE_IMAGEMAGICK_V7 STREQUAL true AND NOT USE_IMAGEMAGICK_V7 STREQUAL false)
	message(FATAL_ERROR "Variable 'USE_IMAGEMAGICK_V7' was not set correctly.")
endif()
target_compile_definitions(
	merlin_rendering_output_tests
	PUBLIC
	BACH_USE_IMAGEMAGICK_V7=${USE_IMAGEMAGICK_V7})

add_test(
	NAME "Merlin: Rendering output test"
	COMMAND merlin_rendering_output_tests)
