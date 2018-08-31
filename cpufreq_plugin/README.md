# Score-P CPU_Freq Tuning Plugin

## Compilation and Installation

### Prerequisites

To compile this plugin, you need:

* C11 compiler
* Readex Runtime Library (RRL)

### Building and installation

```
mkdir BUILD && cd BUILD
cmake ..
make
make install
```

### CMake settings

The following settings are important:

* SCOREP_CONFIG                   path to the scorep-config tool including the file name
* `RRL_INC`                       path to the RRL include folder
* One of the following interfaces is required to be able to do the CPU frequency tuning:
    * `x86_adapt`, see https://github.com/tud-zih-energy/x86_adapt 
    * `msr-safe`, see https://github.com/LLNL/msr-safe 
    * `LIKWID`, see https://github.com/RRZE-HPC/likwid
    * `sysfs` 
* CMAKE_INSTALL_PREFIX            directory where the resulting plugin will be installed (lib/ suffix will be added)


> *Note:*
> If you have `scorep-config` in your `PATH`, it should be found by CMake.
> Make sure to add the subfolder `lib` to your `LD_LIBRARY_PATH`.

## Usage

To add the tuning plugin you have to add `cpu_freq_plugin` to the environment
variable `SCOREP_TUNING_PLUGINS`.


### Environment variables

* `SCOREP_TUNING_CPU_FREQ_PLUGIN_VERBOSE` 
    Controls the output verbosity of the plugin. Possible values are:
    `VERBOSE`, `WARN` (default), `INFO`, `DEBUG`
    If set to any other value, WARN is used. Case in-sensitive.

### If anything fails:

1. Check whether the plugin library can be loaded from the `LD_LIBRARY_PATH`.

3. Write a mail to the author.

## Authors

* Andreas Gocht (andreas.gocht at tu-dresden dot de)
* Umbreen Sabir Mian (umbreen.mian at tu-dresden dot de)
