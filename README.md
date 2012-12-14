# OVERVIEW

qRestAPI is a cross-platform [Qt-based](http://doc.qt.nokia.com/4.7/qt4-7-intro.html) library 
allowing to easily query a [Midas](http://midas.kitware.com) or [XNAT](http://xnat.org) server.

## Prerequisites

 * [Qt 4.6.2](http://qt.nokia.com/downloads)
 * [CMake 2.8.2](http://www.cmake.org)

## How to build

    git clone git://github.com/commontk/qRestAPI.git
    mkdir qRestAPI-build
    cd qRestAPI-build
    cmake -DQT_QMAKE_EXECUTABLE:FILEPATH=/path/to/cmake ../qRestAPI
    make -j4
    ctest

## Contribute
Fork + pull.
