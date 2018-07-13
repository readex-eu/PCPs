# Score-P Tuning Plugin example

## Compilation and Installation

### Prerequisites

To compile this plugin, you need:

* C11 compiler

* Readex Runtime Library (RRL)

### Building and installation

1. Invoke CMake

        mkdir BUILD && cd BUILD
        cmake ../

The following settings are important:

* SCOREP_CONFIG                   path to the scorep-config tool including the file name
* `RRL_INC`                       path to the RRL include folder
* CMAKE_INSTALL_PREFIX            directory where the resulting plugin will be installed (lib/ suffix will be added)

2. Invoke make

        make

> *Note:*

> If you have `scorep-config` in your `PATH`, it should be found by CMake.

3. Invoke make

        make install

> *Note:*

> Make sure to add the subfolder `lib` to your `LD_LIBRARY_PATH`.

## Usage

To add the tuing plugin you have to add `example` to the environment
variable `SCOREP_TUNING_PLUGINS`.


### Environment variables

### If anything fails:

1. Check whether the plugin library can be loaded from the `LD_LIBRARY_PATH`.

2. Check whether you are using a onlineaccess enhanced version of scorep

3. Write a mail to the author.

## Authors

* Andreas Gocht (andreas.gocht at tu-dresden dot de)
