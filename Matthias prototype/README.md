# Qt Bachelor Thesis Repository
## About this Repository
The following repository is dedicated to the bachelor thesis work by Eimen Oueslati, Nils Petter Skålerud, and Cecilia Norevik Bratlie. The developed software is for the Qt Group.

The core goal is to develop a native and open-source parser and renderer for the MapBox vector format using the Qt framework. The binary data is retrieved from a server, parsed, and rendered on the screen. The Qt framework offers cross-platform C++ APIs for all essential tasks, including networking, binary file parsing, vector data rendering, and creating user interface elements.


## Building Qt From Source and Rendering a Tile

This is just a small test application to see if we can render MapBox maps with Qt.

### Getting started

Build qt from source: https://code.qt.io/cgit/qt/qt5.git/ (Needed because of a recent bugfix in qtgrpc).

Use the following submodlues:

```./init-repository --module-subset="qtbase,qtgrpc,qtrepotools" -f```

(Repotools is only required for gerrit to work)

Make sure you got all dependencies for qtgrpc installed (maybe not all are needed):

* libprotobuf-dev
* protobuf-compiler
* protobuf-compiler-grpc
* libgrpc++-dev
* libgrpc-dev

Also see the introduction here: https://doc.qt.io/qt-6/qtprotobuf-index.html

Compile with (e.g. in a new folder qt5-build, next to your qt5 folder)

```./qt5/configure -developer-build -nomake examples -make tests```

If you build Qt sucessfully, make sure that QtCreator knows where your Qt build is.
Then you can just open the CMakeList.txt file of this project in QtCreator.

### Test Data

The testdata contains a style sheet (tiles.json) from maptiler.com as well as three tiles (e.g. z12x2170y1190.mvt), also downloaded from maptiler.com.

You can get more test data if you make a maptiler account: https://www.maptiler.com/

You can also use my key(aTD2rOqjSNUxK8sO2oEz), but please don't do anything that gets me into trouble:

So here is a stylesheet: https://api.maptiler.com/maps/basic-v2/style.json?key=aTD2rOqjSNUxK8sO2oEz

And this is the url to get the tiles:
https://api.maptiler.com/tiles/v3/tiles.json?key=aTD2rOqjSNUxK8sO2oEz

that forwards to:
```https://api.maptiler.com/tiles/v3/{z}/{x}/{y}.pbf?key=aTD2rOqjSNUxK8sO2oEz```
(You have to fill out x,y,z yourself)


### Status

* 28.12.23: The json style and the mvt tiles are parsed (incompletely) and some fill rendering is happening.
* 29.12.23: Improved rendering a bit. Colors seem correct in the example and added lines.


### References:

#### GRPC intro
https://grpc.io/docs/what-is-grpc/introduction/

#### Vector tile standard (the GRPC part)
https://docs.mapbox.com/data/tilesets/guides/vector-tiles-standards/

#### Style spec (the json part)
https://docs.mapbox.com/style-spec/guides/

#### Webmercator toy (to find the correct tile)
https://www.maptiler.com/google-maps-coordinates-tile-bounds-projection
