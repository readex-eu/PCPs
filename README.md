# Parameter Control Plugins (PCPs)

The Parameter Control Plugins are written as part of the READEX project
([www.readex.eu](http://www.readex.eu))

## Compilation and Installation

### Prerequisites

To compile the Parameter Control Plugins, you need:

* C compiler
* Readex Runtime Library (RRL)
* One of the following interfaces is required to be able to do the CPU frequency and Uncore Frequency tuning:
    * `x86_adapt`, see https://github.com/tud-zih-energy/x86_adapt 
    * `msr-safe`, see https://github.com/LLNL/msr-safe 
    * `LIKWID`, see https://github.com/RRZE-HPC/likwid
    * `sysfs` Note: It only support CPU Frequency Scaling

### Required Environment variables

* `RRL_INC` path to the RRL include folder

### Building and installation

```
./build.sh <install directory. This is the directory set for CMAKE_INSTALL_PREFIX for each plugin>
```

## Usage

For the usage of PCPs, please refer to the `README.md` of individual plugins available in each *_plugin directory.

### If anything fails:

Please check whether the plugin libraries can be loaded from the `LD_LIBRARY_PATH`.
If you have some logs please contact the Author and attach the logs.

## Authors

* Umbreen Sabir Mian (umbreen.mian at tu-dresden dot de)
