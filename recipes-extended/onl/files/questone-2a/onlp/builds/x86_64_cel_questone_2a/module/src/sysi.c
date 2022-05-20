#include <unistd.h>
#include <fcntl.h>

#include <onlplib/file.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/sysi.h>
#include <x86_64_cel_questone_2a/x86_64_cel_questone_2a_config.h>

#include "x86_64_cel_questone_2a_int.h"
#include "x86_64_cel_questone_2a_log.h"
#include "platform.h"
//Below include add for support Cache system
#include <sys/stat.h>
#include <time.h>
#include <sys/mman.h>
#include <semaphore.h>

static char arr_cplddev_name[NUM_OF_CPLD][10] =
{
        "version"
};

const char *onlp_sysi_platform_get(void)
{
    return "x86-64-cel-questone-2a-r0";
}

int onlp_sysi_init(void)
{
    return ONLP_STATUS_OK;
}

int onlp_sysi_platform_info_get(onlp_platform_info_t *pi)
{
    int i, v[NUM_OF_CPLD] = {0};
    char r_data[10] = {0};
    char fullpath[PREFIX_PATH_LEN] = {0};

    for (i = 0; i < NUM_OF_CPLD; i++)
    {
        memset(fullpath, 0, PREFIX_PATH_LEN);
        sprintf(fullpath, "%s%s", SYS_CPLD_PATH, arr_cplddev_name[i]);
        if (read_device_node_string(fullpath, r_data, sizeof(r_data), 0) != 0)
        {
            DEBUG_PRINT("%s(%d): read %s error\n", __FUNCTION__, __LINE__, fullpath);
            return ONLP_STATUS_E_INTERNAL;
        }
        v[i] = strtol(r_data, NULL, 0);
    }
    pi->cpld_versions = aim_fstrdup("CPLD_B=0x%02x", v[0]);
    return 0;
}

void onlp_sysi_platform_info_free(onlp_platform_info_t *pi)
{
    aim_free(pi->cpld_versions);
}

int onlp_sysi_onie_data_get(uint8_t **data, int *size)
{
    uint8_t *rdata = aim_zmalloc(256);

    if (onlp_file_read(rdata, 256, size, PREFIX_PATH_ON_SYS_EEPROM) == ONLP_STATUS_OK)
    {
        if (*size == 256)
        {
            *data = rdata;
            return ONLP_STATUS_OK;
        }
    }
    aim_free(rdata);
    rdata = NULL;
    *size = 0;
    DEBUG_PRINT("[Debug][%s][%d][Can't get onie data]\n", __FUNCTION__, __LINE__);
    return ONLP_STATUS_E_INTERNAL;
}

void onlp_sysi_onie_data_free(uint8_t *data)
{
    aim_free(data);
}

int onlp_sysi_platform_manage_init(void)
{
    if (is_cache_exist() < 1)
    {
        create_cache();
    }
    return ONLP_STATUS_OK;
}

int onlp_sysi_platform_manage_fans(void)
{
    if (is_cache_exist() < 1)
    {
        create_cache();
    }
    return ONLP_STATUS_OK;
}

int onlp_sysi_platform_manage_leds(void)
{
    if (is_cache_exist() < 1)
    {
        create_cache();
    }
    return ONLP_STATUS_OK;
}

int onlp_sysi_oids_get(onlp_oid_t *table, int max)
{
    int i;
    onlp_oid_t *e = table;

    memset(table, 0, max * sizeof(onlp_oid_t));

    /* 2 PSUs */
    *e++ = ONLP_PSU_ID_CREATE(1);
    *e++ = ONLP_PSU_ID_CREATE(2);

    // // /* LEDs Item */
    for (i = 1; i <= LED_COUNT; i++)
        *e++ = ONLP_LED_ID_CREATE(i);

    // // /* THERMALs Item */
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++)
        *e++ = ONLP_THERMAL_ID_CREATE(i);

    // /* Fans Item */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++)
        *e++ = ONLP_FAN_ID_CREATE(i);

    return ONLP_STATUS_OK;
}
