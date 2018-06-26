/**
 * @file sample_plugin.c
 *
 * @brief Description of a sample plugin
 */

#include <scorep/rrl_tuning_plugins.h>
#include <stdio.h>
#include <string.h>

/**
 * Initialize the plugin
 *
 * looks for all CPU's that have a userspace governor.
 *
 * @retrun 0 at sucess
 */
int32_t init()
{
    // some initialisation
    printf("some initialisation\n");
    return 0;
}

/**
 * finalising the plugin
 */
void fini()
{
    // some finalisation
    printf("some finalisation\n");
}

static int set_param1(int new_setting)
{
    printf("Set param 1 to new parameter %d\n", new_setting);
    // set_some_new_values(new_setting);
    return 0;
}

static int get_param1()
{
    int param1 = 0;
    // param1 = get_some_current_values()
    return param1;
}

static int set_param2(int new_setting)
{
    printf("Set param 2 to new parameter %d\n", new_setting);
    // set_some_new_values(new_setting);
    return 0;
}

static int get_param2()
{
    int param2 = 0;
    // param2 = get_some_current_values()
    return param2;
}

/**
 * ScoreP array for plugin definitions
 */
static rrl_tuning_action_info return_values[] = {{
                                                     .name = "PARAM1",
                                                     .current_config = &get_param1,
                                                     .enter_region_set_config = &set_param1,
                                                     .exit_region_set_config = &set_param1,
                                                 },
    {
        .name = "PARAM2",
        .current_config = &get_param2,
        .enter_region_set_config = &set_param2,
        .exit_region_set_config = &set_param2,
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
RRL_TUNING_PLUGIN_ENTRY(example)
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
