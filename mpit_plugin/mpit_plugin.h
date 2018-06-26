/*
 * mpit_plugin,
 *
 * a c++ MPI_T tuning plugin for Score-P.
 * Copyright (C) 2015 TU Dresden, ZIH
 *
 * Author: Andreas Gocht
 * Email: andreas.gocht [at] tu-dresden.de
 *
 */

#ifndef MPIT_PLUGIN_H_
#define MPIT_PLUGIN_H_

#include <scorep/rrl_tuning_plugins.h>

#include "mpit_interface.h"

int init();
rrl_tuning_action_info *get_tuning_info();
void create_location(RRL_LocationType location_type, uint32_t location_id);
void delete_location(RRL_LocationType location_type, uint32_t location_id);
void fini();

static mpit_interface::mpit_values *mpi_value_;

static int change_MPIR_CVAR_REDUCE_SHORT_MSG_SIZE(int new_settings);

extern "C" {

/**
 * Macro to setup the plugin
 */
RRL_TUNING_PLUGIN_ENTRY(mpit_plugin)
{
    /* Initialize info data (with zero) */
    rrl_tuning_plugin_info info;
    memset(&info, 0, sizeof(rrl_tuning_plugin_info));

    /* Set up */
    info.plugin_version = RRL_TUNING_PLUGIN_VERSION;
    info.initialize = init;
    info.get_tuning_info = get_tuning_info;
    info.finalize = fini;
    return info;
}
}

#endif /* MPIT_PLUGIN_H_ */
