# Merlin
Merlin is a testing utility written for the project. It serves as a standalone subsystem that is added to the test.

## Purpose
The purpose of Merlin is to detect changes to the graphical output of the rendering code. Merlin should help us catch regressions whenever the application changes in such a way that outputted map graphics. This can help us catch rendering artifacts, or other bugs introduced to the rendering code, before merging or deployment.

## How
Merlin works by establishing a "baseline" at a point in time when rendering is generally know to be correct (Where correctness is judged manually by the maintainers). This baseline should remain unchanged until the definition of correctness of the output changes.

Later in time we can then attempt to recreate the baseline with newer code, and compare it to the baseline. If any output differs, it could imply either a regression/bug, or it could imply that an intentional change was made to the rendering and the baseline needs to be rebuilt.

Merlin uses a set of predetermined  list test-case configurations. A test-case configuration will include specific details on how to recreate a specific image. This include viewport coordinates, zoom, the list of tiles available, the combination of elements to include during rendering.

## Tools
Merlin defines two tools to perform it's purpose.

The executable `merlin_rebuild_base` is the program that produces the initial baseline. It processes the list of test-case configurations and writes the output to disk. This tool should generally not be used often, and should mostly be used when making intentional changes to the behavior of the rendering functonality.

If a developer makes changes to the behavior of the rendering functionality *without* running this tool, they should expect the automated tests to fail.

The executable `merlin_rendering_output_tests` is the tool that performs the comparison to the baseline. This tool will attempt to recreate every test-case configuration from the baseline, but instead using the current code. If any of the outputs don't match the baseline (within an error margin), the process will return non-zero result code.

Additionally, if the output-tests fail, the test-cases that did fail will then be outputted to a local directory `rendering_failures`. This folder will contain 3 files per failed test-case: one for the baseline, one for the recently generated image, and one image that highlights the differences between the two former images.

## Required resources
Merlin requires 3 types of resources to function. They are as follows:
 - **List of test-case configurations.** Stored as JSON.
 - **Stylesheet**. The stylesheet to run the rendering. Stored as a JSON. Should follow the MapTiler specifications for stylesheets.
 - **MapBox Vector Tiles**. A set of MVT files containing the source data for map-tiles. 

## Configuration
Configuration happens by managing the collection of test-case configurations. These are stored in a JSON format, in the file `unitTestResources/merlin/testcases.json`. This JSON file will then contain a list of items that will be read and executed by Merlin.

## Limitations
Merlin is not able to perform rendering of text consistently across platforms (i.e Linux and Windows). This means in all our tests, text will not be tested to be correct.

## `testcases.json` schema
This section describes how to setup a valid JSON file of test-case configurations for Merlin to process.

The top level object in a test-case configurations file needs to be an array. Each element in this array describes an individual test-case configuration. A test-case configuration may have the following values:

type ViewportCoords = `[float, float]` where each element corresponds to longitude and latitude respectively.

type TileCoord = `[int, int, int]` where each element corresponds to zoom, x and y respectively.

- `name`. Value: `string`. Optional. An identifier that may be output during terminal. Mosty helpful for identifying individual
test-cases during execution. Default to an empty string.
- `coords`. Value: `Array[2] of floats`. Optional. The center-coordinates of the viewport in longitude and latitude, respectively. Values outside the world map are allowed. Defaults to `[0, 0]`.
- `vp-zoom`. Value: `float`. Optional. The zoom level of the viewport. Defaults to `0`.
- `map-zoom`. Value: `int`. Optional. The zoom level of the map. Defaults to `0`.
-  `tiles`. Value: `TileCoord[]`. A list of TileCoords. This defines the subset of tiles that are allowed to be used during rendering. If this value is not defined, the program will automatically calculate which tiles would be visible in viewports configuration.
- `draw-fill`. Value: `bool`. Optional. Controls whether any fill-elements should be rendered in this test-case. Defaults to `true`.
- `draw-lines`. Value: `bool`. Optional. Controls whether any line-elements should be rendered in this test-case. Defaults to `true`.

## `testcases.json` example
```json
[
	{
		"name": "Background only",
		"draw-fill": false,
		"draw-lines": false
	},
	{
		"name": "Top worldmap"
	},
	{
		"name": "Top worldmap, map zoom 1",
		"vp-zoom": 0.1,
	},
	{
		"name": "Top worldmap, viewport zoom 1",
		"vp-zoom": 1
	}
]
```
