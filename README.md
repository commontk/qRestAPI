# OVERVIEW

qRestAPI is a cross-platform [Qt-based](https://www.qt.io/) library 
allowing to easily query any [RESTful](https://en.wikipedia.org/wiki/Representational_state_transfer) web services. 

It also provides a convenience interface to communicate with a [Midas](http://midas.kitware.com) server RESTful API.

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
    cmake -DQt5_DIR:PATH=/path/to/Qt5.9.1/5.9.1/gcc_64/lib/cmake/Qt5 ../qRestAPI
    make -j4

## Testing

To run the test `qMidasAPITest` checking that synchronous query can successfully be executed against http://slicer.kitware.com/midas3 server.

    cd qRestAPI-build
    ctest

## Contribute
Fork + pull.
