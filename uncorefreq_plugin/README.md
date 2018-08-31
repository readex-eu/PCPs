# Score-P Tuning Plugin example

## Compilation and Installation

### Prerequisites

To compile this plugin, you need:

* C11 compiler
* Readex Runtime Library (RRL)
* One of the following interfaces is required to be able to do the Uncore Frequency tuning:
    * `x86_adapt`, see https://github.com/tud-zih-energy/x86_adapt 
    * `msr-safe`, see https://github.com/LLNL/msr-safe 
    * `LIKWID`, see https://github.com/RRZE-HPC/likwid

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


## Usage

To add the tuing plugin you have to add `uncore_freq_plugin` to the environment
variable `SCOREP_RRL_PLUGINS`.

## Environment variables:

* `CHECK_IF_NODE_FULLY_OCCUPIED` enables the check if the node (alias processor die) is fully occupied by the process. Defualt is 1 which enables the bahviour. To disable please set 0. Please be aware that if `CHECK_IF_NODE_FULLY_OCCUPIED` is enabled and a process just uses a part of the node, no uncor tuning will happen.
* `SCOREP_TUNING_UNCORE_FREQ_PLUGIN_VERBOSE` sets the plugin print mode. Possible values are `DEBUG`, `INFO`, `WARN`, `VERBOSE`


### If anything fails:

Please check whether the plugin library can be loaded from the `LD_LIBRARY_PATH`.
If you have some logs please contact the Authors and attach the logs.

## Authors

* Andreas Gocht (andreas.gocht at tu-dresden dot de)
* Umbreen Sabir Mian (umbreen.mian at tu-dresden dot de)
