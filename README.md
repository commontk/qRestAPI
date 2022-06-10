# OVERVIEW

qRestAPI is a cross-platform [Qt-based](https://www.qt.io/) library 
allowing to easily query any [RESTful](https://en.wikipedia.org/wiki/Representational_state_transfer) web services. 

It provides the following interfaces:

| Interface    | RESTful API      |
|--------------| -----------------|
| `qRestAPI`   | _any_            |
| `qGirderAPI` | [Girder][girder] |
| `qMidasAPI`  | [Midas][midas]   |

[girder]: https://github.com/girder/girder
[midas]: https://github.com/midasplatform/midas

## Prerequisites

 * [Qt](https://www.qt.io/) 4.x or 5.x
 * [CMake](http://www.cmake.org)

## How to build using Qt 4.x

    git clone git://github.com/commontk/qRestAPI.git
    mkdir qRestAPI-build
    cd qRestAPI-build
    cmake -DQT_QMAKE_EXECUTABLE:FILEPATH=/path/to/qmake ../qRestAPI
    make -j4

## How to build using Qt 5.x

    git clone git://github.com/commontk/qRestAPI.git
    mkdir qRestAPI-build
    cd qRestAPI-build
    cmake -DQt5_DIR:PATH=/path/to/QtX.Y.Z/X.Y.Z/gcc_64/lib/cmake/Qt5 ../qRestAPI
    make -j4

## Testing

To run tests checking that queries can successfully be executed.

    cd qRestAPI-build
    ctest

List of tests expecting servers to be reachable:

| Test             | Server                            |
|------------------|-----------------------------------|
| `qGirderAPITest` | https://data.kitware.com/api/v1   |
| `qMidasAPITest`  | https://slicer.kitware.com/midas3 |


## Contribute

Contributions are welcome, and they are greatly appreciated! Every little bit helps, and credit will always be given.

See [CONTRIBUTING.md][contributing] for more details.

[contributing]: https://github.com/commontk/qRestAPI/blob/master/CONTRIBUTING.md
