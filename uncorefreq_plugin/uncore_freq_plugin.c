/**
 * @file sample_plugin.c
 *
 * @brief Description of a sample plugin
 */

#define _GNU_SOURCE
#include "uthash.h"
#include <errno.h>
#include <freqgen.h>
#include <sched.h>
#include <scorep/rrl_tuning_plugins.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

static freq_gen_interface_t *interface;
static int available_devices;
static int *devices;

static char *env_string;

static int cores_per_die;
static int *socket_owned;
static int check_fully_occupied = 0;

static long available_cores;
static cpu_set_t *responsible_cpus;
static size_t responsible_cpus_size;

static long long int *default_min_freq;
static long long int *default_max_freq;

#define PLUGIN_NAME "UNCORE_FREQ_TP"
#define MAX_SETTINGS 97

struct settings
{
    long long int freq;
    freq_gen_setting_t val;
    UT_hash_handle hh;
};
static struct settings *frequency_information_hashmap;

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
        level_str = getenv("SCOREP_TUNING_UNCORE_FREQ_PLUGIN_VERBOSE");
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
 *  Adds a new setting to hashtable
 *  @param[IN] setting  New setting to add to the hashmap.
 */
void hash_add(struct settings *setting)
{
    HASH_ADD(hh, frequency_information_hashmap, freq, sizeof(long long int), setting);
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

    env_string = getenv("CHECK_IF_NODE_FULLY_OCCUPIED");
    if (env_string == NULL)
        check_fully_occupied = 1;
    else
    {
        check_fully_occupied = atoi(env_string);
        if (check_fully_occupied == 1)
            llog(LOG_INFO,
                "CHECK_IF_NODE_FULLY_OCCUPIED is enabled. Uncore frequency only set "
                "when all the cores on the node are occupied");
        else if (check_fully_occupied != 0)
        {
            llog(LOG_WARN,
                "Could not parse the value provided. CHECK_IF_NODE_FULLY_OCCUPIED "
                "should be set to 0 or 1");
            check_fully_occupied = 0;
        }
    }
    int init_device_done = 0;
    while (init_device_done != 1)
    {
        interface = freq_gen_init(FREQ_GEN_DEVICE_UNCORE_FREQ);
        if (interface != NULL)
        {
            llog(LOG_DEBUG, "Got the interface %s", interface->name);
            available_devices = interface->get_num_devices();
            devices = calloc(available_devices, sizeof(int));
            if (!devices && available_devices != 0)
            {
                llog(LOG_WARN, "memory failure %s \n", strerror(errno));
                return -errno;
            }
            int node = 0;
            for (node = 0; node < available_devices; node++)
            {
                devices[node] = interface->init_device(node);
                if (devices[node] < 0)
                {
                    llog(LOG_WARN, "init device %d failed:", node);
                    llog(LOG_WARN,
                        "Got error no: %d %s",
                        devices[node],
                        strerror(abs(devices[node])));
                    interface->finalize();
                    break;
                }
                else
                {
                    llog(LOG_DEBUG, "init device %d successful:", node);
                    if (node == available_devices - 1)
                    {
                        init_device_done = 1;
                    }
                }
            }
        }
        else
        {
            llog(LOG_WARN, "No interface for UNCORE FREQ found");
            return -1;
        }
    }

    available_cores = sysconf(_SC_NPROCESSORS_ONLN);

    llog(LOG_DEBUG, "got %u devices", available_devices);
    llog(LOG_DEBUG, "got %u cpus", available_cores);

    cores_per_die = available_cores / available_devices;
    llog(LOG_INFO, "assuming %d cores per device (processor die)", cores_per_die);

    // get inital responsible CPUS

    responsible_cpus = CPU_ALLOC(available_cores);
    responsible_cpus_size = CPU_ALLOC_SIZE(available_cores);

    if (responsible_cpus == NULL)
    {
        llog(LOG_WARN, "error CPU_ALLOC");
        return -1;
    }

    CPU_ZERO_S(responsible_cpus_size, responsible_cpus);

    pid_t tid;
    tid = syscall(SYS_gettid);
    sched_getaffinity(tid, responsible_cpus_size, responsible_cpus);

    socket_owned = calloc(available_devices, sizeof(int));
    if (!socket_owned && available_devices != 0)
    {
        llog(LOG_WARN, "memory failure %s \n", strerror(errno));
        return -errno;
    }

    /** get default freqs
     *
     */
    long long int unc_freq = 0;

    default_max_freq = calloc(available_devices, sizeof(long long int));
    if (!default_max_freq && available_devices != 0)
    {
        llog(LOG_WARN, "memory failure %s \n", strerror(errno));
        return -errno;
    }
    default_min_freq = calloc(available_devices, sizeof(long long int));
    if (!default_min_freq && available_devices != 0)
    {
        llog(LOG_WARN, "memory failure %s \n", strerror(errno));
        return -errno;
    }
    for (int node = 0; node < available_devices; node++)
    {
        unc_freq = interface->get_frequency(devices[node]);
        if (unc_freq < 0)
        {
            llog(LOG_WARN, "Could not get default max uncore frequency for node %d\n", node);
            llog(LOG_WARN, "Got the error no: %lli %s \n", unc_freq, strerror(abs(unc_freq)));
            return unc_freq;
        }
        else
        {
            llog(LOG_DEBUG, "Default max uncore frequency for node %d  is %lli\n", node, unc_freq);
            default_max_freq[node] = unc_freq;
            freq_gen_setting_t generated_setting;
            struct settings *s = NULL;
            HASH_FIND_LLI(frequency_information_hashmap, &default_max_freq[node], s);
            if (s == NULL)
            {
                generated_setting = interface->prepare_set_frequency(default_max_freq[node], 0);
                if (generated_setting != NULL)
                {
                    s = (struct settings *) malloc(sizeof(struct settings));
                    if (s == NULL)
                    {
                        llog(LOG_WARN, "memory failure %s \n", strerror(errno));
                        return -errno;
                    }
                    s->freq = default_max_freq[node];
                    s->val = generated_setting;
                    hash_add(s);
                }
                else
                {
                    llog(LOG_WARN,
                        "Could not prepare default max uncore frequency for node %d\n",
                        node);
                }
            }
            else
            {
                llog(LOG_DEBUG, "Default Setting found  for %lli\n", default_max_freq[node]);
            }
        }
        if (interface->get_min_frequency != NULL)
        {
            unc_freq = interface->get_min_frequency(devices[node]);
            if (unc_freq < 0)
            {
                llog(LOG_WARN, "Could not get min. uncore frequency for node %d\n", node);
                llog(LOG_WARN, "Got the error no: %lli %s \n", unc_freq, strerror(abs(unc_freq)));
                return unc_freq;
            }
            else
            {
                llog(LOG_DEBUG,
                    "Default min. uncore frequency for node %d  is %lli\n",
                    node,
                    unc_freq);
                default_min_freq[node] = unc_freq;
                freq_gen_setting_t generated_setting;
                struct settings *s = NULL;
                HASH_FIND_LLI(frequency_information_hashmap, &default_min_freq[node], s);
                //                if ((generated_setting = lookup(frequency_information_hashmap,
                //                default_min_freq[node])) != NULL)
                //                if (!setting_found)
                if (s == NULL)
                {
                    generated_setting = interface->prepare_set_frequency(default_min_freq[node], 0);
                    if (generated_setting != NULL)
                    {
                        //                        insert(frequency_information_hashmap,
                        //                        default_min_freq[node],
                        //                        generated_setting);
                        s = (struct settings *) malloc(sizeof(struct settings));
                        if (s == NULL)
                        {
                            llog(LOG_WARN, "memory failure %s \n", strerror(errno));
                            return -errno;
                        }
                        s->freq = default_min_freq[node];
                        s->val = generated_setting;
                        hash_add(s);
                    }
                    else
                    {
                        llog(LOG_WARN,
                            "Could not prepare default min uncore frequency for node %d\n",
                            node);
                    }
                }
                else
                {
                    llog(LOG_DEBUG, "Default Setting found  for %lli\n", default_min_freq[node]);
                }
            }
        }
        else
        {
            llog(LOG_WARN,
                "The interface to get minimum uncore frequency is either not there \n"
                "or not implemented and the frequency is not considered to be a range");
        }
    }

    return 0;
}

void create_location(RRL_LocationType location_type, uint32_t location_id)
{
    llog(LOG_DEBUG, "create_location for location %u with typ %u ", location_id, location_type);
    if (location_type == RRL_LOCATION_TYPE_CPU_THREAD)
    {
        cpu_set_t *set;
        pid_t tid;
        size_t set_size = CPU_ALLOC_SIZE(available_cores);

        tid = syscall(SYS_gettid);
        set = CPU_ALLOC(available_cores);

        if (set == NULL)
        {
            llog(LOG_WARN, "error during create_location: CPU_ALLOC");
            return;
        }

        CPU_ZERO_S(set_size, set);
        sched_getaffinity(tid, set_size, set);
        CPU_OR_S(responsible_cpus_size, responsible_cpus, set, responsible_cpus);

        CPU_FREE(set);
        llog(LOG_DEBUG, "added new CPU");
        int cpu = 0;
        for (cpu = 0; cpu < available_cores; cpu++)
        {
            if (CPU_ISSET_S(cpu, responsible_cpus_size, responsible_cpus))
            {
                llog(LOG_DEBUG, "cpu in set: %d ", cpu);
            }
        }

        /* check which cpus we own on which die:
         *
         */
        int node;
        for (node = 0; node < available_devices; node++)
        {
            int owned_cpus_on_socet = 0;

            for (cpu = node * cores_per_die; cpu < cores_per_die * (node + 1); cpu++)
            {
                if (CPU_ISSET_S(cpu, responsible_cpus_size, responsible_cpus))
                {
                    owned_cpus_on_socet++;
                }
            }
            if (owned_cpus_on_socet < cores_per_die)
            {
                socket_owned[node] = 0;
                llog(LOG_DEBUG,
                    "got %d out of %d cores on die %d. Won't start tuning the uncore",
                    owned_cpus_on_socet,
                    cores_per_die,
                    node);
            }
            else if (owned_cpus_on_socet == cores_per_die)
            {
                socket_owned[node] = 1;
                llog(LOG_DEBUG,
                    "got %d out of %d cores on die %d. Start tuning the uncore",
                    owned_cpus_on_socet,
                    cores_per_die,
                    node);
            }
            else if (owned_cpus_on_socet > cores_per_die)
            {
                socket_owned[node] = 0;
                llog(LOG_WARN,
                    "got %d out of %d cores on die %d. Won't start tuning the uncore. \n"
                    "YOU GOT MORE CORES ASSOCIATED TO YOUR DIE, THAN THERE ARE CORES ON YOUR DIE\n"
                    "YOU ARE EITHER A MAGE OR SOMETHING WENT REALLY WRONG :-)",
                    owned_cpus_on_socet,
                    cores_per_die,
                    node);
            }
            else
            {
                llog(LOG_WARN,
                    "This should never happen XD",
                    owned_cpus_on_socet,
                    cores_per_die,
                    node);
            }
        }
    }
}

void delete_location(RRL_LocationType location_type, uint32_t location_id)
{
    llog(LOG_DEBUG, "delete_location for location %u with typ %u ", location_id, location_type);
}

/**
 * Finalising the plugin
 */
void fini()
{
    // some finalisation
    llog(LOG_INFO, "UNCORE_FREQ tuning plugin: finalizing\n");
    struct settings *s = NULL, *temp = NULL;
    HASH_ITER(hh, frequency_information_hashmap, s, temp)
    {
        interface->unprepare_set_frequency(s->val);
        HASH_DEL(frequency_information_hashmap, s);
        free(s);
    }
    int node = 0;
    for (node = 0; node < available_devices; node++)
    {
        interface->close_device(node, devices[node]);
    }
    interface->finalize();
}

/**
 * set the frequency on uncore
 * the new_setting is provided in MHz
 * @param new_settings  new frequency to set
 * @return 0 on success or an error defined in errno.h
 */

static int scorep_set_uncore_freq(int new_settings)
{
    int rt = 0;
    long long int new_settings_ = (long long int) new_settings * 1000000;
    llog(LOG_INFO, "setting freq to %lu", new_settings_);

    for (int node = 0; node < available_devices; node++)
    {
        /* Set the uncore freq
         *
         */
        if (!check_fully_occupied || socket_owned[node])
        {

            if (new_settings != -1)
            {
                freq_gen_setting_t generated_setting;
                struct settings *s = NULL;
                HASH_FIND_LLI(frequency_information_hashmap, &new_settings_, s);
                if (s == NULL)
                {
                    llog(LOG_DEBUG,
                        "preparing new setting for uncore frequency to %lli\n",
                        new_settings_);
                    generated_setting = interface->prepare_set_frequency(new_settings_, 0);
                    if (generated_setting != NULL)
                    {
                        s = (struct settings *) malloc(sizeof(struct settings));
                        if (s == NULL)
                        {
                            llog(LOG_WARN, "memory failure %s \n", strerror(errno));
                            return -errno;
                        }
                        s->freq = new_settings_;
                        s->val = generated_setting;
                        hash_add(s);
                    }
                    else
                    {
                        llog(LOG_WARN,
                            "Could not prepare the new frequency setting to %lu",
                            new_settings_);
                        return -1;
                    }
                }
                else
                {
                    llog(LOG_DEBUG, "Setting found  for %lli\n", new_settings_);
                    generated_setting = s->val;
                }
                if ((rt = interface->set_frequency(devices[node], generated_setting)) != 0)
                {
                    llog(LOG_WARN, "Could not set uncore frequency for node %d\n", node);
                    llog(LOG_WARN, "Got error no: %d %s\n", rt, strerror(rt));
                    return rt;
                }
                llog(LOG_INFO, "setting uncore frequency %lli (node: %d)\n", new_settings_, node);
            }
            else
            {
                /** reset default
                 *
                 */
                freq_gen_setting_t generated_setting;
                struct settings *s_max = NULL, *s_min = NULL;
                HASH_FIND_LLI(frequency_information_hashmap, &default_max_freq[node], s_max);
                if (s_max == NULL)
                {
                    llog(LOG_WARN,
                        "Could not find default max uncore frequency in prepared frequencies \n"
                        "This should never happen");
                    return -1;
                }
                else
                {
                    llog(LOG_DEBUG,
                        "Default max uncore frequency setting found  for %lli\n",
                        default_max_freq[node]);
                    generated_setting = s_max->val;
                }
                if ((rt = interface->set_frequency(devices[node], generated_setting)) != 0)
                {
                    llog(
                        LOG_WARN, "Could not set default max uncore frequency for node %d\n", node);
                    llog(LOG_WARN, "Got error no: %d %s\n", rt, strerror(rt));
                    return rt;
                }
                else
                {

                    llog(LOG_DEBUG,
                        "setting default max uncore frequency %lli (node: %d)\n",
                        default_max_freq[node],
                        node);
                }
                HASH_FIND_LLI(frequency_information_hashmap, &default_min_freq[node], s_min);
                if (s_min == NULL)
                {
                    llog(LOG_WARN,
                        "Could not find default min uncore frequency in prepared frequencies \n"
                        "This should never happen");
                    return -1;
                }
                else
                {
                    llog(LOG_DEBUG,
                        "Default min uncore frequency setting found  for %lli\n",
                        default_min_freq[node]);
                    generated_setting = s_min->val;
                }
                if (interface->set_min_frequency != NULL)
                {
                    if ((rt = interface->set_min_frequency(devices[node], generated_setting)) != 0)
                    {
                        llog(LOG_WARN,
                            "Could not set default min uncore frequency for node %d\n",
                            node);
                        llog(LOG_WARN, "Got error no: %d %s\n", rt, strerror(rt));
                        return rt;
                    }
                    else
                    {

                        llog(LOG_INFO,
                            "setting default min uncore frequency %lli (node: %d)\n",
                            default_min_freq[node],
                            node);
                    }
                }
                else
                {
                    llog(LOG_INFO,
                        "The interface to set minimum uncore frequency is either not there \n"
                        "or not implemented and the frequency is not considered to be a range");
                }
            }
        }
    }
    return rt;
}

/**
 * returns the average of the uncore frequency
 *
 * looks for all CPU's frequencies and makes their average
 *
 * @return average uncore frequency in MHz
 */
static int scorep_get_uncore_freq()
{
    long long int unc_freq = 0;
    long long int average = 0;
    int count = 0;
    int node = 0;

    for (node = 0; node < available_devices; node++)
    {
        unc_freq = interface->get_frequency(devices[node]);
        if (unc_freq < 0)
        {
            llog(LOG_WARN, "Could not get uncore frequency for node %d\n", node);
            llog(LOG_WARN, "Got error no: %lli %s\n", unc_freq, strerror(abs(unc_freq)));
        }
        else
        {
            llog(LOG_DEBUG, "Got uncore freq =  %lli for node %d", unc_freq, node);
            ++count;
            average += unc_freq;
        }
    }
    average /= 1000000;
    if (count > 1)
    {
        return (int) (average / count);
    }
    else
    {
        return (int) average;
    }
}

/**
 * ScoreP array for plugin definitions
 */
static rrl_tuning_action_info return_values[] = {
    {
        .name = "UNCORE_FREQ",
        .current_config = &scorep_get_uncore_freq,
        .enter_region_set_config = &scorep_set_uncore_freq,
        .exit_region_set_config = &scorep_set_uncore_freq,
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
RRL_TUNING_PLUGIN_ENTRY(uncore_freq_plugin)
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
