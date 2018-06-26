/*
 * mpit_plugin,
 *
 * a c++ MPI_T tuning plugin for Score-P.
 * Copyright (C) 2015 TU Dresden, ZIH
 *
 * Author: Andreas Gocht
 * Email: andreas.gocht [at] tu-dresden.de
 *
 * @brief This file allows the tuning interface to change MPI_T CVars.
 *
 */

#include "mpit_plugin.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef enum { LOG_VERBOSE, LOG_WARN, LOG_INFO, LOG_DEBUG, LOG_INVALID } log_level;

/**
 * log function.
 *
 * Prints messages depending on the entry in SCOREP_TUNING_CPU_FREQ_PLUGIN_VERBOSE.
 *
 * Currently implemented loge levels are are:
 *
 *	* LOG_VERBOSE
 *	* LOG_WARN
 *	* LOG_INFO
 *	* LOG_DEBUG
 *
 * @param[in] msg_level level of message
 * @param[in] message_fmt printf like message
 * @param[in] ... printf like parameters for the message
 */
void llog(log_level msg_level, const char *message_fmt, ...)
{
    static char *level_str = NULL;
    static log_level level = LOG_INVALID;
    if (level == LOG_INVALID)
    {
        level_str = getenv("SCOREP_TUNING_MPIT_PLUGIN_VERBOSE");
        if (level_str == NULL)
        {
            level = LOG_WARN;
        }
        else if (strcmp(level_str, "DEBUG") == 0)
        {
            level = LOG_DEBUG;
        }
        else if (strcmp(level_str, "INFO") == 0)
        {
            level = LOG_INFO;
        }
        else if (strcmp(level_str, "WARN") == 0)
        {
            level = LOG_WARN;
        }
        else if (strcmp(level_str, "VERBOSE") == 0)
        {
            level = LOG_VERBOSE;
        }
        else
        {
            level = LOG_WARN;
        }
    }
    if (msg_level <= level)
    {
        va_list args;
        va_start(args, message_fmt);
        vprintf(message_fmt, args);
        va_end(args);
    }
}

/**
 * Initialize the plugin
 *
 * loads mpi_value
 *
 * @return 0 at sucess
 */
int32_t init()
{
    llog(LOG_DEBUG, "MPIT tuning plugin: initializing\n");

    mpi_value_ = new mpit_interface::mpit_values();

    llog(LOG_DEBUG, "MPIT tuning plugin: initialised\n");
    return 0;
}

void create_location(RRL_LocationType location_type, uint32_t location_id)
{
}

void delete_location(RRL_LocationType location_type, uint32_t location_id)
{
}

/**
 * finalising the plugin
 */
void fini()
{
    llog(LOG_DEBUG, "MPIT tuning plugin: finalizing\n");
    delete mpi_value_;
    llog(LOG_DEBUG, "MPIT tuning plugin: finalised\n");
}

/**
 * Set the MPIT_value change_MPIR_CVAR_REDUCE_SHORT_MSG_SIZE
 *
 * Implemented for MPICH 3.2. Please see the MPI implementation.
 *
 * @param[in]   new_settings new CVar setting
 * @param[out]  old_setting old CVar setting
 * @return return 0 on success, -1 on failure
 */
static int change_MPIR_CVAR_REDUCE_SHORT_MSG_SIZE(int new_settings)
{

    int new_settings_ = new_settings;

    try
    {
        int cvar_id = mpi_value_->change_mpi_variable_by_name(
            std::string("MPIR_CVAR_REDUCE_SHORT_MSG_SIZE"), &new_settings);
        llog(LOG_DEBUG, "MPIT tuning plugin: CVar ID: %d \n", cvar_id);
    }
    catch (mpit_interface::cvar_not_found &e)
    {
        llog(LOG_WARN, "MPIT tuning plugin: %s", e.what());
        return -1;
    }

    llog(LOG_DEBUG,
        "MPIT tuning plugin: setting \"MPIR_CVAR_REDUCE_SHORT_MSG_SIZE\" from %d to %d \n",
        new_settings,
        new_settings_);

    return 0;
}

static int get_MPIR_CVAR_REDUCE_SHORT_MSG_SIZE()
{
    // int value = 0;
    // int cvar_id = mpi_value_->get_cvar_by_name("MPIR_CVAR_REDUCE_SHORT_MSG_SIZE");
    // mpi_value_->get_mpit_bind_no_object_int_values(cvar_id, &value);
    int value = mpi_value_->change_mpi_variable_by_name(
        std::string("MPIR_CVAR_REDUCE_SHORT_MSG_SIZE"), &value);

    return value;
}

/**
 * ScoreP function to get plugin definitions
 *
 * @param return return_values.
 */
#define MPIT_TUNING_FUNCTIONS 1
rrl_tuning_action_info return_values[MPIT_TUNING_FUNCTIONS + 1];
rrl_tuning_action_info *get_tuning_info()
{
    return_values[0].name = (char *) "MPIR_CVAR_REDUCE_SHORT_MSG_SIZE";
    return_values[0].current_config = &get_MPIR_CVAR_REDUCE_SHORT_MSG_SIZE;
    return_values[0].enter_region_set_config = &change_MPIR_CVAR_REDUCE_SHORT_MSG_SIZE;
    return_values[0].exit_region_set_config = &change_MPIR_CVAR_REDUCE_SHORT_MSG_SIZE;

    return_values[MPIT_TUNING_FUNCTIONS].name = NULL;
    return_values[MPIT_TUNING_FUNCTIONS].enter_region_set_config = NULL;
    return_values[MPIT_TUNING_FUNCTIONS].exit_region_set_config = NULL;

    return return_values;
}
