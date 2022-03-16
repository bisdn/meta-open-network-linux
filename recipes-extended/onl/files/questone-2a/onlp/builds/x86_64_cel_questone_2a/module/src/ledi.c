#include <onlp/platformi/ledi.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "platform.h"

enum onlp_led_id
{
    LED_RESERVED = 0,
    LED_SYSTEM,
    LED_ALARM,
    LED_PSU1,
    LED_PSU2,
    LED_FAN1,
    LED_FAN2,
    LED_FAN3,
    LED_FAN4
};

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t led_info[] =
{
    { },
    {
        { ONLP_LED_ID_CREATE(LED_SYSTEM), "System LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_YELLOW | ONLP_LED_CAPS_YELLOW_BLINKING | ONLP_LED_CAPS_GREEN | 
        ONLP_LED_CAPS_GREEN_BLINKING | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_ALARM), "Alert LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_YELLOW | ONLP_LED_CAPS_YELLOW_BLINKING | ONLP_LED_CAPS_GREEN | 
        ONLP_LED_CAPS_GREEN_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU1), "PSU-1 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_AUTO | ONLP_LED_CAPS_YELLOW | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU2), "PSU-2 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_AUTO | ONLP_LED_CAPS_YELLOW | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN1), "Chassis FAN(1) LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED |  ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN2), "Chassis FAN(2) LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED |  ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN3), "Chassis FAN(3) LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED |  ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN4), "Chassis FAN(4) LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED |  ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_AUTO,
    }
};

int
onlp_ledi_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info_p)
{
    int led_id;
    uint8_t led_color = 0;
    uint8_t alert_status,present_status =0;
    uint8_t blink_status = 0;
    uint8_t result = 0;

    led_id = ONLP_OID_ID_GET(id);
    *info_p = led_info[led_id];
 
    result = get_led_status(led_id);

    if(result != 0xFF)
        info_p->status |= ONLP_LED_STATUS_ON;

    switch(led_id){
        case LED_SYSTEM:
        case LED_ALARM:

            led_color = (result >> 4)&0x3;
            if(led_color == 0){
                if(led_id==LED_SYSTEM)
                    info_p->mode |= ONLP_LED_MODE_AUTO;
                if(led_id==LED_ALARM)
                    info_p->mode |= ONLP_LED_MODE_OFF;
            }
            if(led_color == 1){
                info_p->mode |= ONLP_LED_MODE_GREEN;
            }
            if(led_color == 2){
                info_p->mode |= ONLP_LED_MODE_YELLOW;
            }
            if(led_color == 3){
                info_p->mode |= ONLP_LED_MODE_OFF;
                break;
            }

            blink_status = result & 0x3;
            if(blink_status == 1 || blink_status == 2){
                int current_mode = info_p->mode;
                info_p->mode = current_mode+1;
            }

            break;
        case LED_FAN1:
        case LED_FAN2:
        case LED_FAN3:
        case LED_FAN4:
            led_color = result & 0x3;

            if(led_color == 0){
                info_p->mode |= ONLP_LED_MODE_OFF;
            }
            if(led_color == 1){
                info_p->mode |= ONLP_LED_MODE_GREEN;
            }
            if(led_color == 2){
                info_p->mode |= ONLP_LED_MODE_RED;
            }
            break;
        case LED_PSU1:
            alert_status = (result >> 7) & 0x1;
            present_status = (result >> 5) & 0x1;

            if(present_status == 0){
                if(alert_status == 1){
                    info_p->mode |= ONLP_LED_MODE_GREEN;
                }else{
                    info_p->mode |= ONLP_LED_MODE_YELLOW;
                }
            }else{
                info_p->mode |= ONLP_LED_MODE_YELLOW;
            }
            break;
        case LED_PSU2:
            alert_status = (result >> 6) & 0x1;
            present_status = (result >> 4) & 0x1;

            if(present_status == 0){
                if(alert_status == 1){
                    info_p->mode |= ONLP_LED_MODE_GREEN;
                }else{
                    info_p->mode |= ONLP_LED_MODE_YELLOW;
                }
            }else{
                info_p->mode |= ONLP_LED_MODE_YELLOW;
            }
            break;
    }
    return ONLP_STATUS_OK;
}
