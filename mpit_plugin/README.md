#Score-P MPIT-CVar Tuning Plugin

##Compilation and Installation

###Prerequisites

To compile this plugin, you need:

* C++11 compiler

* Score-P with SCOREP_TUNING_PLUGIN_VERSION 0
    Currently this is just available in the Munich online access branch
    
###Building and installation

1. Invoke CMake

        mkdir BUILD && cd BUILD
        cmake ..

The following settings are important:

* SCOREP_CONFIG                   path to the scorep-config tool including the file name
* CMAKE_INSTALL_PREFIX            directory where the resulting plugin will be installed (lib/ suffix will be added)

2. Invoke make

        make

> *Note:*

> If you have `scorep-config` in your `PATH`, it should be found by CMake.

3. Invoke make

        make install

> *Note:*

> Make sure to add the subfolder `lib` to your `LD_LIBRARY_PATH`.

##Usage

To add the tuning plugin, you have to add `mpit_plugin` to the environment
variable `SCOREP_TUNING_PLUGINS`.

Currently the following MPIT-CVars are supported:

* for MPICH 3.2:
    * `MPIR_CVAR_REDUCE_SHORT_MSG_SIZE`
    
Further CVars might be added soon.


###Environment variables

* `SCOREP_TUNING_MPIT_PLUGIN_VERBOSE` 

    Controls the output verbosity of the plugin. Possible values are:
    `VERBOSE`, `WARN` (default), `INFO`, `DEBUG`
    If set to any other value, WARN is used. Case in-sensitive.

###If anything fails:

1. Check whether the plugin library can be loaded from the `LD_LIBRARY_PATH`.

2. Check whether your MPI supports the requested CVar operation.

3. Write a mail to the author, pleas add the MPI and Score-P version you used.

##Authors

* Andreas Gocht (andreas.gocht at tu-dresden dot de)