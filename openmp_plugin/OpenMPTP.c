/*
 * This file is part of the Score-P software (http://www.score-p.org)
 *
 * Copyright (c) 2015, Technische Universität München, Germany
 *
 * This software may be modified and distributed under the terms of
 * a BSD-style license.  See the COPYING file in the package base
 * directory for details.
 *
 *
 * INFO:
 *
 * This tuning plugin for Score-P allows the the number of threads in
 * an OpenMP parallel region to be adjusted during run-time.
 *
 */

#include <errno.h>
#include <omp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "scorep/rrl_tuning_plugins.h"

#define PLUGIN_NAME "OpenMPTP"

typedef enum { LOG_VERBOSE, LOG_WARN, LOG_INFO, LOG_DEBUG, LOG_INVALID } log_level;

/**
 * log function.
 *
 * Prints messages depending on the entry in
 * SCOREP_TUNING_OPENMPTP_PLUGIN_VERBOSE.
 *
 * Currently implemented loge levels are are:
 *
 *  * LOG_VERBOSE
 *  * LOG_WARN
 *  * LOG_INFO
 *  * LOG_DEBUG
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
        level_str = getenv("SCOREP_TUNING_OPENMPTP_PLUGIN_VERBOSE");
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
        char *output_fmt = (char *) malloc(strlen(message_fmt) + strlen(PLUGIN_NAME) + 5);
        strcpy(output_fmt, "[");
        strcpy(output_fmt + 1, PLUGIN_NAME);
        strcpy(output_fmt + 1 + strlen(PLUGIN_NAME), "]");
        strcpy(output_fmt + 2 + strlen(PLUGIN_NAME), message_fmt);
        strcpy(output_fmt + 2 + strlen(PLUGIN_NAME) + strlen(message_fmt), "\n");
        va_list args;
        va_start(args, message_fmt);
        vprintf(output_fmt, args);
        va_end(args);
    }
}

static int scorep_omp_set_num_threads(int new_setting)
{
    llog(LOG_DEBUG, "[NUMTHREADS]: Set new setting");
    omp_set_num_threads(new_setting);
    llog(LOG_DEBUG, "[NUMTHREADS]: New Setting = %d", new_setting);
    return 0;
}

static int scorep_omp_get_num_threads()
{
    int max_threads = omp_get_max_threads();

    return max_threads;
}

// static void scorep_kmp_set_blocktime(int new_setting, int* old_setting) {
//    (*old_setting) = kmp_get_blocktime();
//    kmp_set_blocktime(new_setting);
//}

static int scorep_omp_set_schedule_type(int new_setting)
{
    llog(LOG_DEBUG, "[SCHEDULE_TYPE]: setting scheduling type");

    if ((new_setting < 1) || (new_setting > 4))
    {
        /*
         *      see documentetion:
         *      omp_sched_static = 1,
         *      omp_sched_dynamic = 2,
         *      omp_sched_guided = 3,
         *      omp_sched_auto = 4
         */
        llog(LOG_WARN,
            "[SCHEDULE_TYPE]: Invalid scheduler type. %d is not allowed,"
            "type has to be between 1 and 4\n",
            new_setting);

        return -1;
    }

    omp_sched_t kind = 4; // set to auto;
    int chunk_size;

    omp_get_schedule(&kind, &chunk_size); // get existing chunk_size

    omp_set_schedule((omp_sched_t) new_setting, chunk_size);
    llog(LOG_INFO, "[SCHEDULE_TYPE]: New kind = %d New chunk size = %d", new_setting, chunk_size);
    return 0;
}

static int scorep_omp_get_schedule_type()
{
    omp_sched_t kind;
    int chunk_size;

    omp_get_schedule(&kind, &chunk_size);

    return (int) kind;
}

static int scorep_omp_set_chunk_size(int new_setting)
{
    omp_sched_t kind = 1;

    llog(LOG_DEBUG, "[SCHEDULE_CHUNK_SIZE]: setting scheduling chunk size");

    omp_set_schedule(kind, new_setting);
    llog(LOG_DEBUG, "[SCHEDULE_CHUNK_SIZE]: New kind = %d New chunk size = %d", kind, new_setting);
    return 0;
}

static int scorep_omp_get_chunk_size()
{
    int chunk_size;
    omp_sched_t kind;

    omp_get_schedule(&kind, &chunk_size);

    return chunk_size;
}

static rrl_tuning_action_info return_values[] = {
    {
        .name = "NUMTHREADS",
        .current_config = &scorep_omp_get_num_threads,
        .enter_region_set_config = &scorep_omp_set_num_threads,
        .exit_region_set_config = &scorep_omp_set_num_threads,
    },
    /*
        {
            .name = "KMPBLOCKTIME",
            .enter_region_set_config = &scorep_kmp_set_blocktime,
            .exit_region_set_config = &scorep_kmp_set_blocktime,
            .current_config = &scorep_kmp_get_blocktime,
        },
    */
    {
        .name = "SCHEDULE_TYPE",
        .current_config = &scorep_omp_get_schedule_type,
        .enter_region_set_config = &scorep_omp_set_schedule_type,
        .exit_region_set_config = &scorep_omp_set_schedule_type,
    },
    {
        .name = "SCHEDULE_CHUNK_SIZE",
        .current_config = &scorep_omp_get_chunk_size,
        .enter_region_set_config = &scorep_omp_set_chunk_size,
        .exit_region_set_config = &scorep_omp_set_chunk_size,
    },
    {
        .name = NULL,
        .current_config = NULL,
        .enter_region_set_config = NULL,
        .exit_region_set_config = NULL,
    }};

int32_t init()
{
    llog(LOG_DEBUG, "GIT revision: %s", GIT_REV);
    llog(LOG_INFO, " Initializing");
    return 0;
}

void create_location(RRL_LocationType location_type, uint32_t location_id)
{
}

void delete_location(RRL_LocationType location_type, uint32_t location_id)
{
}

rrl_tuning_action_info *get_tuning_info()
{
    return return_values;
}

void fini()
{
    llog(LOG_INFO, " Finalizing");
}

RRL_TUNING_PLUGIN_ENTRY(OpenMPTP)
{
    /* Initialize info data (with zero) */
    rrl_tuning_plugin_info info;
    memset(&info, 0, sizeof(rrl_tuning_plugin_info));

    /* Set up */
    info.plugin_version = RRL_TUNING_PLUGIN_VERSION;
    info.initialize = init;
    info.get_tuning_info = get_tuning_info;
    info.finalize = fini;
    info.create_location = create_location;
    info.delete_location = delete_location;
    return info;
}
#ifdef __cplusplus
}

#endif
