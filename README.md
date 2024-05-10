# Qt Bachelor Thesis Repository

## About this Repository
This repository hosts the bachelor project authored by [Eimen Oueslati](https://github.com/EimenOueslati), [Nils Petter Skålerud](https://github.com/Didgy74), and [Cecilia Norevik Bratlie](https://github.com/cecilianor). The core goal of the project is to develop a vector-bases map application that uses the [MapBox vector format](https://github.com/mapbox/vector-tile-spec/blob/master/2.1/README.md). The application is developed in C++ with the Qt framework.

## Details
The project is part of a bachelor thesis at the Norwegian University of Science and Technology (NTNU). The solution is developed in C++17 and relies heavily on the Qt framework. Cmake is the project's build system. The application renders geographical data in both vector and raster format, and the data is supplied by [MapTiler](https://www.maptiler.com). The application first requests and parses styling data, and then requests, decodes, styles, and renders the geographical data on screen. The application is multithreaded and the current solution only supports the basic v2 map style. The requested map tiles are cached on disk for better performance.

## Technologies

| Tool | Version | Usage |
|---|---|---|
| Windows |  22H2 | Primary OS used for development |
| Ubuntu | 22.04 | Used for Dockerising the project (for GitHub CI/CD) |
| Qt | 6.7.0 | Rendering, networking, Protobuf parsing, containers, threading, helper utilities |
| GCC | 11.4.0 | C++ compiler for Ubuntu |
| MSVC | v143 | C++ compiler for Windows |
| Cmake | 3.22.1 | Generating build files |
| Ninja | 1.10.2 | Running compilation and linker step on build files |
| Protobuf | 3.21.12 | Dependency for QtGrpc module |
| Image Magick | 6.9.11 | Image comparison during rendering output tests |

## Features
**Rendering:** The application is able to render polygons, lines, and text including road names.  There is no support for icons in the current version of the application.  
**Styling:** The only supported style is the Basic v2 map style.  
**UI:** Moving the map (panning) and zooming can be done using either the mouse/mouse wheel or the keyboard. It's also possible to move to a specified location using longitude and latitude coordinates. For the sake of the thesis, there are two buttons to move to Gjøvik or Nydalen in Oslo.  

## Showcase
![zoom_leve_0](https://github.com/cecilianor/Qt-thesis/assets/73423072/1a31ca50-2212-45d0-a244-af0e2e37d5d8)

https://github.com/cecilianor/Qt-thesis/assets/73423072/b443ae4d-13ac-49cd-91fb-37ed658d0878

## How to Build the Project
Build instructions can be found in [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md)

## Merlin
Details on the rendering testing system can be found in [tests/merlin/README.md](tests/merlin/README.md)


