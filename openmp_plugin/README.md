#Score-P OpenMP tuning plugin

##Compilation and Installation

###Prerequisites

To compile this plugin, you need:

* C compiler
* libpthread
* Readex Runtime Library (RRL)

### Building and installation

```
mkdir BUILD && cd BUILD
cmake ../
make
make install
```

#### CMake settings

* `SCOREP_CONFIG` path to the scorep-config tool including the file name
* `RRL_INC` path to the RRL include folder
* `CMAKE_INSTALL_PREFIX` directory where the resulting plugin will be installed (lib/ suffix will be added)
 
> *Note:*
> If you have `scorep-config` in your `PATH`, it should be found by CMake.

> Make sure to add the subfolder `lib` to your `LD_LIBRARY_PATH`.


### Usage

To add the tuing plugin you have to add `OpenMPTP` to the environment
variable `SCOREP_RRL_PLUGINS`.

Important variables:

* `SCOREP_TUNING_OpenMPTP_PLUGIN_VERBOSE` sets the plugin print mode. Possible values are `DEBUG`, `INFO`, `WARN`, `VERBOSE

###If anything fails

Please check whether the plugin library can be loaded from the `LD_LIBRARY_PATH`.
If you have some logs please contact the Authors and attach the logs.

##Authors

Andreas Gocht (andreas.gocht at tu-dresden dot de)

Robert Mijaković (mijakovi at in dot tum dot de)
