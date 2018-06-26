/**
 * @file ebp_plugin.c
 *
 * @brief Intel Energy Performance Bias Plugin
 */

#include <scorep/rrl_tuning_plugins.h>

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "x86_adapt.h"

x86_adapt_device_type adapt_type;
int epb_num;

#define EPB "Intel_Energy_Perf_Bias"
#define PLUGIN_NAME "EPB_TP"

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
        level_str = getenv("SCOREP_TUNING_EPB_PLUGIN_VERBOSE");
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

/**
 * Initialize the plugin
 *
 * looks for all CPUs that have a userspace governor.
 *
 * @return 0 at sucess
 */
int32_t init()
{
    // some initialisation
    llog(LOG_VERBOSE, "UNCORE_FREQ tuning plugin: initializing\n");
    if (x86_adapt_init())
    {
        llog(LOG_WARN, "Could not initialize x86_adapt library");
    }

    adapt_type = X86_ADAPT_CPU;

    epb_num = x86_adapt_lookup_ci_name(adapt_type, EPB);
    if (epb_num < 0)
    {
        llog(LOG_WARN, "could not find item %s\n", EPB);
    }
    llog(LOG_DEBUG, "[init]found item %s with number %d", EPB, epb_num);

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
    x86_adapt_finalize();
    // some finalisation
    llog(LOG_INFO, ": finalizing\n");
}

static int scorep_set_epb(int new_settings)
{
    unsigned long new_settings_ = (unsigned long) new_settings;
    int cpu = 0;

    for (cpu = 0; cpu < x86_adapt_get_nr_avaible_devices(adapt_type); cpu++)
    {

        int fd = x86_adapt_get_device(adapt_type, cpu);
        unsigned long epb;
        int rt;

        if ((rt = x86_adapt_get_setting(fd, epb_num, &epb)) != 8)
        {
            llog(LOG_WARN, "Could not get epb for node %d\n", cpu);
            return rt;
        }

        int rt1;

        if ((rt1 = x86_adapt_set_setting(fd, epb_num, new_settings_)) != 8)
        {
            llog(LOG_WARN, "Could not set epb for node %d\n", cpu);
            return rt1;
        }

        llog(LOG_INFO, "setting epb from %d to %lu (node: %d)\n", epb, new_settings_, cpu);

        rt = x86_adapt_put_device(adapt_type, cpu);
        if (rt != 0)
        {
            llog(LOG_WARN, "Could not close x86a_adapt for node %d\n", cpu);
        }
    }
    return 0;
}

/**
 * returns the average of the epb
 *
 * looks for all CPU's epbs and makes their average
 *
 * @return average
 */
static int scorep_get_epb()
{

    int cpu = 0;
    unsigned long int epb = 0;
    unsigned long int average = 0;
    int count = 0;

    for (cpu = 0; cpu < x86_adapt_get_nr_avaible_devices(adapt_type); cpu++)
    {

        int rt;
        int fd = x86_adapt_get_device(adapt_type, cpu);

        if ((rt = x86_adapt_get_setting(fd, epb_num, &epb)) != 8)
        {
            llog(LOG_WARN, "Could not get epb for node %d\n", cpu);
            llog(LOG_WARN, "Got error: %d ", rt, strerror(abs(rt)));
        }
        else
        {
            ++count;
            average += epb;
        }
    }

    if (count > 1)
    {
        return (int) average / count;
    }
    else
    {
        return (int) average;
    }
}

/**
 * ScoreP array for plugin definitions
 */
static rrl_tuning_action_info return_values[] = {{
                                                     .name = "EPB",
                                                     .current_config = &scorep_get_epb,
                                                     .enter_region_set_config = &scorep_set_epb,
                                                     .exit_region_set_config = &scorep_set_epb,
                                                 },
    {
        .name = NULL,
        .current_config = NULL,
        .enter_region_set_config = NULL,
        .exit_region_set_config = NULL,

    }};

/**
 * ScoreP function to get plugin definitions
 *
 * @param return return_values.
 */
rrl_tuning_action_info *get_tuning_info()
{
    return return_values;
}

/**
 * Macro to setup the plugin
 */
RRL_TUNING_PLUGIN_ENTRY(epb_plugin)
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
