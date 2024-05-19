# How to Run
This text describes how to run the project after it has been built.

When the project is successfully built, the following executables will be present in the build directory.
```
application

evaluator_test
layerstyle_test
render_test
tileloader_test
vectortile_test

tile_parsing_benchmark
tileloader_threaded_benchmark

merlin_rebuild_baseline
merlin_rendering_output_tests
```

# Running the Main Application
The application can be started by using the `application` executable.  A prerequisite to running this executable is to supply it with a MapTiler API key. This key is mandatory for the application to run (unless a previous offline cache has been established). This API key can be supplied in one of the following ways:
 - **Environment variable** - Set an environment variable with key `MAPTILER_KEY` with value set to the MapTiler API key. Check your OS instructions on how to set environment variables. The application will read this key automatically if found by the application. Example: `MAPTILER_KEY=abcdefgh`
- **File** - Create a file `key.txt` and place it in the same directory as the executable file. This file will contain the raw MapTiler API key only. Example: `abcdefgh`

If the application has been successfully run previously, the application is able to reuse the cache if no networking connection can be established.

# Running the Tests
The executables postfixed with `_test` perform unit tests on the corresponding subsystem.

Note: On headless systems, these tests will need an environment variable `QT_QPA_PLATFORM=offscreen` in order to run correctly. This is not necessary on systems with a display server supported by Qt (such as regular desktop Windows and Ubuntu).
 
While these can be run individually, it is recommended to run all of them sequentially. This can be done using the command `ctest` provided by CMake. Example for Ubuntu Server:
```
export QT_QPA_PLATFORM=offscreen
ctest --rerun-failed --output-on-failure
```

This will also run the relevant Merlin tests.

# Running the Benchmarks
The executables postfixed with `_benchmark` perform our benchmarks. These can be run standalone with no prerequisites. Keep in mind some of the benchmarks take a long time (>20 minutes) to complete.

# Running Merlin Tests
The executables prefixed with `merlin_` are related to the Merlin subsystem for performing graphical output tests. Specific instructions on how to operate the Merlin subsystem are found in the dedicated file [tests/merlin/README.md](tests/merlin/README.md).
