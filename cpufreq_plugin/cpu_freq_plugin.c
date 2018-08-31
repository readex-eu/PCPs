
/**
 * @file cpu_freq_plugin.c
 *
 * @brief Tuning Plugin for CPU frequency tuning
 */
#define _GNU_SOURCE
#include <errno.h>
#include <freqgen.h>
#include <sched.h>
#include <scorep/rrl_tuning_plugins.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include "uthash.h"

#define MAX_SETTINGS 97
#define PLUGIN_NAME "cpu_freq"

static freq_gen_interface_t *interface;

static cpu_set_t *responsible_cpus;
static size_t responsible_cpus_size;
static int *devices;

static int available_cores;
static long long int default_freq;
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
 * Prints messages depending on the entry in
 *SCOREP_TUNING_CPU_FREQ_PLUGIN_VERBOSE.
 *
 * Currently implemented log levels are:
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
        level_str = getenv("SCOREP_TUNING_CPU_FREQ_PLUGIN_VERBOSE");
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
 * Initialize the CPU cores
 *
 * initializes the cpus using init_device
 *
 * @return 1 at success, 0 at failure
 */

int init_responsible_cpus()
{
    llog(LOG_DEBUG, "init CPUs");

    for (int cpu = 0; cpu < available_cores; cpu++)
    {
        if (CPU_ISSET_S(cpu, responsible_cpus_size, responsible_cpus))
        {
            llog(LOG_DEBUG, "init cpu %d", cpu);
            devices[cpu] = interface->init_device(cpu);
            if (devices[cpu] < 0)
            {
                llog(LOG_WARN, "init cpu %d failed:", cpu);
                llog(LOG_WARN, "%s %s", strerror(abs(devices[cpu])),
                        freq_gen_error_string());
                CPU_CLR_S(cpu, responsible_cpus_size, responsible_cpus);
                return devices[cpu];
            }
            else
            {
                llog(LOG_DEBUG, "init device %d successful:", cpu);
            }
        }
    }
    return 1;
}

/**
 * Initialize the plugin
 *
 * Initializes the freqgen interface.
 * counts CPUs and calls get_cpus to get the for this program active ones
 * Checks the interface by initializing cpus.
 * If cpu initialization fails, finalizes the interface and tries to get a new interface
 * If no interface is found, returns with -1.
 *
 *
 * @return 0 at success, -1 at failure
 */
int32_t init()
{
    llog(LOG_DEBUG, "GIT revision: %s", GIT_REV);
    llog(LOG_VERBOSE, "CPU_FREQ tuning plugin: initializing");

    int init_device_done = 0;
    while (init_device_done != 1)
    {
        interface = freq_gen_init(FREQ_GEN_DEVICE_CORE_FREQ);
        if (interface == NULL)
        {
            llog(LOG_WARN, "No interface for CORE FREQ found. Last error: %s",
                 freq_gen_error_string() );
            return -1;
        }
        llog(LOG_INFO, "Got the interface %s", interface->name);
        available_cores = interface->get_num_devices();
        llog(LOG_INFO, "got %u cpus", available_cores);
        if (available_cores == 0)
        {
            return -errno;
        }

        devices = calloc(available_cores, sizeof(int));
        if (!devices && available_cores != 0)
        {
            llog(LOG_WARN, "memory failure %s \n", strerror(errno));
            return -errno;
        }

        // get inital responsible CPUS
//        available_cores=152;
        responsible_cpus = CPU_ALLOC(available_cores);
        responsible_cpus_size = CPU_ALLOC_SIZE(available_cores);

        if (responsible_cpus == NULL)
        {
            llog(LOG_WARN, "error CPU_ALLOC");
            return -1;
        }

        CPU_ZERO_S(responsible_cpus_size, responsible_cpus);
        int err = sched_getaffinity(getpid(), responsible_cpus_size, responsible_cpus);
        if (err == -1)
        {
            llog(LOG_WARN, "sched_getaffinity failed: %s (%d)", strerror(errno), errno);

            switch (errno)
            {
                case EFAULT:
                    llog(LOG_WARN, "EFAULT: A supplied memory address was invalid.");
                    break;
                case EINVAL:
                    llog(LOG_WARN,
                        "EFAULT: cpusetsize is smaller than the size of the affinity mask used by "
                        "the kernel.");
                    break;
                case ESRCH:
                    llog(LOG_WARN, "ESRCH: The thread whose ID is pid could not be found.");
                    break;
                default:
                    llog(LOG_WARN, "Errorcode %d unkown in this context.", errno);
            }
            return -1;
        }

        init_device_done = init_responsible_cpus();
        if (init_device_done != 1)
        {
            interface->finalize();
            CPU_FREE(responsible_cpus);
            responsible_cpus = 0;
            responsible_cpus_size = (size_t) 0;
            free(devices);
        }
    }

    /** get default freqs
                *
                */
    long long int freq = -1;
    for (int cpu = 0; cpu < available_cores; cpu++)
    {
        if (CPU_ISSET_S(cpu, responsible_cpus_size, responsible_cpus))
        {
            long long int cpu_freq = interface->get_frequency(devices[cpu]);
            if (cpu_freq < 0)
            {
                llog(LOG_WARN, "Error getting cpu freq for cpu %d", cpu);
                llog(LOG_WARN, "Got error no: %lli %s %s\n", cpu_freq, strerror(abs(cpu_freq)),
                        freq_gen_error_string());
            }
            else
            {
                llog(LOG_DEBUG, "Got default freq =  %lli for cpu %d", cpu_freq, cpu);
                if (freq < cpu_freq)
                {
                    freq = cpu_freq;
                }
            }
        }
    }
    default_freq = freq;
    freq_gen_setting_t generated_setting;
    struct settings *s = NULL;
    HASH_FIND(hh, frequency_information_hashmap, &default_freq, sizeof(long long int), s);
    //    if (!setting_found)
    if (s == NULL)
    {
        generated_setting = interface->prepare_set_frequency(default_freq, 0);

        if (generated_setting != NULL)
        {
            s = (struct settings *) malloc(sizeof(struct settings));
            if (s == NULL)
            {
                llog(LOG_WARN, "memory failure %s \n", strerror(errno));
                return -errno;
            }
            s->freq = default_freq;
            s->val = generated_setting;
            hash_add(s);
        }
        else
        {
            llog(LOG_WARN, "Could not prepare default core frequency to %lli %s\n", default_freq,
                    freq_gen_error_string());
        }
    }
    else
    {
        llog(LOG_DEBUG, "Default Setting found  for %lli\n", default_freq);
    }

    return 0;
}

/**
 * Set the frequency
 *
 * Sets the frequency to new_settigns for all CPUs that have a userspace
 * governor.
 * If new_settings is smaller or larger than the minimal or maximal CPU
 * frequency,
 * the frequency is set to the minimal or maximal CPU frequency.
 * The new_setting should be in MHz.
 *
 * @param[in] new_settings new frequency settings for CPUs
 * @return 0 on success or <0 on failure
 */

static int scorep_set_cpu_freq(int new_settings)
{
    long long int new_settings_ = ((long long int) new_settings) * 1000000;
    freq_gen_setting_t generated_setting;
    if (new_settings != -1)
    {
        struct settings *s = NULL;
        llog(LOG_DEBUG, "setting freq to %lli", new_settings_);
        HASH_FIND_LLI(frequency_information_hashmap, &new_settings_, s);
        if (s == NULL)
        {
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
                llog(
                    LOG_WARN, "Could not prepare the new frequency setting to %lli %s", new_settings_,
                    freq_gen_error_string());
                return -1;
            }
        }
        else
        {
            llog(LOG_DEBUG, "Setting found  for %lli\n", new_settings_);
            generated_setting = s->val;
        }
    }
    else
    {
        llog(LOG_WARN,
            "Invalid value for frequency = %lli received \n"
            "Setting to default frequency = %lli \n",
            new_settings_,
            default_freq);
        struct settings *s = NULL;
        HASH_FIND_LLI(frequency_information_hashmap, &default_freq, s);
        if (s == NULL)
        {
            llog(LOG_WARN,
                "Could not find default core frequency in prepared frequencies \n"
                "This should never happen");
            return -1;
        }
        else
        {
            llog(LOG_DEBUG, "Default Setting found  for %lli\n", default_freq);
            generated_setting = s->val;
        }
    }

    int rt = 0;

    for (int cpu = 0; cpu < available_cores; cpu++)
    {
        if (CPU_ISSET_S(cpu, responsible_cpus_size, responsible_cpus))
        {
            llog(LOG_DEBUG, "set cpu %d to %lu", cpu, new_settings_);
            rt = interface->set_frequency(devices[cpu], generated_setting);

            if (rt < 0)
            {
                switch (rt)
                {
                    case -1:
                        llog(LOG_WARN,
                            "getting error setting frequency: not initialized (number: %d) %s",
                            rt,
                            freq_gen_error_string());
                        break;
                    case -2:
                        llog(LOG_WARN,
                            "getting error setting frequency: invalid cpu selected "
                            "(number: %d) %s",
                            rt,
                            freq_gen_error_string());
                        break;
                    default:
                        llog(LOG_WARN,
                            "getting error setting frequency: unknown error (number: %d) %s",
                            rt,
                            freq_gen_error_string());
                        break;
                }
            }
        }
    }
    return rt;
}

/** gets the freq for !all! cpus, and return the highest.
 *
 *  @return CPU frequency in MHz on success or 0 on failure
 */
static int scorep_get_cpu_freq()
{
    long long int freq = -1;

    for (int cpu = 0; cpu < available_cores; cpu++)
    {
        if (CPU_ISSET_S(cpu, responsible_cpus_size, responsible_cpus))
        {
            long long int cpu_freq = interface->get_frequency(devices[cpu]);
            if (cpu_freq < 0)
            {
                llog(LOG_DEBUG, "Error getting cpu freq for cpu %d", cpu);
                llog(LOG_WARN, "Got error no: %lli %s %s\n", cpu_freq, strerror(abs(cpu_freq)),
                        freq_gen_error_string());
            }
            else
            {
                llog(LOG_DEBUG, "Got freq =  %lli for cpu %d", cpu_freq, cpu);
                if (freq < cpu_freq)
                {
                    freq = cpu_freq;
                }
            }
        }
    }
    freq /= 1000000;
    return (int) freq;
}

/**
 * Gets the cpu affinity of the CPU Thread and adds it to the responsible cpus list
 * If the init_device fails for the requested CPU thread, this CPU will not be tuned.
 */

void create_location(RRL_LocationType location_type, uint32_t location_id)
{
    llog(LOG_DEBUG, "create_location for location %u with typ %u ", location_id, location_type);
    if (location_type == RRL_LOCATION_TYPE_CPU_THREAD)
    {
        cpu_set_t *set;
        pid_t tid;
        tid = syscall(SYS_gettid);
        set = CPU_ALLOC(available_cores);
        size_t set_size = CPU_ALLOC_SIZE(available_cores);

        if (set == NULL)
        {
            llog(LOG_WARN, "error during create_location: CPU_ALLOC");
            return;
        }
        CPU_ZERO_S(set_size, set);
        int err = sched_getaffinity(tid, set_size, set);
        if (err == -1)
        {
            llog(LOG_WARN, "sched_getaffinity failed: %s (%d)", strerror(errno), errno);

            switch (errno)
            {
                case EFAULT:
                    llog(LOG_WARN, "EFAULT: A supplied memory address was invalid.");
                    break;
                case EINVAL:
                    llog(LOG_WARN,
                        "EFAULT: cpusetsize is smaller than the size of the affinity mask used by "
                        "the kernel.");
                    break;
                case ESRCH:
                    llog(LOG_WARN, "ESRCH: The thread whose ID is pid could not be found.");
                    break;
                default:
                    llog(LOG_WARN, "Errorcode %d unkown in this context.", errno);
            }
        }
        else
        {
            CPU_OR_S(responsible_cpus_size, responsible_cpus, set, responsible_cpus);
            llog(LOG_DEBUG, "adding new CPU");
        }

        CPU_FREE(set);

        int rt = init_responsible_cpus();
        if (rt == 0)
        {
            llog(LOG_WARN, "error during cpu initializing, new cpu not added \n");
            return;
        }
        else
        {
            for (int cpu = 0; cpu < available_cores; cpu++)
            {
                if (CPU_ISSET_S(cpu, responsible_cpus_size, responsible_cpus))
                {
                    llog(LOG_DEBUG, "cpu in set: %d ", cpu);
                }
            }
        }
    }
}

void delete_location(RRL_LocationType location_type, uint32_t location_id)
{
    llog(LOG_DEBUG, "delete_location for location %u with typ %u ", location_id, location_type);
}

/**
 * finalising the plugin
 */
void fini()
{
    llog(LOG_INFO, "CPU_FREQU tuning plugin: finalising");
    struct settings *s = NULL, *temp = NULL;
    HASH_ITER(hh, frequency_information_hashmap, s, temp)
    {
        interface->unprepare_set_frequency(s->val);
        HASH_DEL(frequency_information_hashmap, s);
        free(s);
    }

    for (int cpu = 0; cpu < available_cores; cpu++)
    {
        interface->close_device(cpu, devices[cpu]);
    }
    interface->finalize();
    CPU_FREE(responsible_cpus);
    responsible_cpus = 0;
}

/**
 * ScoreP array for plugin definitions
 */
static rrl_tuning_action_info return_values[] = {
    {
        .name = "CPU_FREQ",                              /**< Name of the Plugin*/
        .current_config = &scorep_get_cpu_freq,          /**< get current configuration*/
        .enter_region_set_config = &scorep_set_cpu_freq, /**< function to call on enter event*/
        .exit_region_set_config = &scorep_set_cpu_freq   /**< function to call on exit event*/
    },
    {.name = NULL,
        .current_config = NULL,
        .enter_region_set_config = NULL,
        .exit_region_set_config = NULL}};

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
RRL_TUNING_PLUGIN_ENTRY(cpu_freq_plugin)
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
