
/*
 * Copyright (c) Calin Crisan
 * This file is part of espQToggle.
 *
 * espQToggle is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <mem.h>
#include <gpio.h>
#include <user_interface.h>

#include "espgoodies/common.h"
#include "espgoodies/utils.h"

#include "ports.h"
#include "extra/rgb.h"


#ifdef HAS_RGB


#define RGB_MIN                     RGB_BLACK
#define RGB_MAX                     RGB_WHITE

#define GPIO_R_CONFIG_OFFS          0x01    /* 1 byte */
#define GPIO_G_CONFIG_OFFS          0x02    /* 1 byte */
#define GPIO_B_CONFIG_OFFS          0x03    /* 1 byte */

#define GPIO_CHOICES_LEN            8

enum {

    RGB_BLACK,                      /* 0 */
    RGB_RED,                        /* 1 */
    RGB_GREEN,                      /* 2 */
    RGB_YELLOW,                     /* 3 */
    RGB_BLUE,                       /* 4 */
    RGB_MAGENTA,                    /* 5 */
    RGB_CYAN,                       /* 6 */
    RGB_WHITE                       /* 7 */

};


typedef struct {

    uint8                           gpio_r;
    uint8                           gpio_g;
    uint8                           gpio_b;

    uint8                           rgb_value;

} extra_info_t;


#define get_gpio_r(p)               (((extra_info_t *) (p)->extra_info)->gpio_r)
#define set_gpio_r(p, v)            {((extra_info_t *) (p)->extra_info)->gpio_r = v;}

#define get_gpio_g(p)               (((extra_info_t *) (p)->extra_info)->gpio_g)
#define set_gpio_g(p, v)            {((extra_info_t *) (p)->extra_info)->gpio_g = v;}

#define get_gpio_b(p)               (((extra_info_t *) (p)->extra_info)->gpio_b)
#define set_gpio_b(p, v)            {((extra_info_t *) (p)->extra_info)->gpio_b = v;}

#define get_rgb_value(p)            (((extra_info_t *) (p)->extra_info)->rgb_value)
#define set_rgb_value(p, v)         {((extra_info_t *) (p)->extra_info)->rgb_value = v;}

ICACHE_FLASH_ATTR static double     read_value(port_t *port);
ICACHE_FLASH_ATTR static bool       write_value(port_t *port, double value);
ICACHE_FLASH_ATTR static void       configure(port_t *port);

ICACHE_FLASH_ATTR static int        attr_get_gpio_r(port_t *port);
ICACHE_FLASH_ATTR static void       attr_set_gpio_r(port_t *port, int index);

ICACHE_FLASH_ATTR static int        attr_get_gpio_g(port_t *port);
ICACHE_FLASH_ATTR static void       attr_set_gpio_g(port_t *port, int index);

ICACHE_FLASH_ATTR static int        attr_get_gpio_b(port_t *port);
ICACHE_FLASH_ATTR static void       attr_set_gpio_b(port_t *port, int index);


static uint8                        gpio_mapping[] = {0, 2, 4, 5, 12, 13, 14, 15};
static char                       * gpio_choices[] = {"gpio 0", "gpio 2", "gpio 4", "gpio 5", "gpio 12", "gpio 13",
                                                      "gpio 14", "gpio 15", NULL};

#if defined(_DEBUG) && defined(_DEBUG_RGB)

static char *COLOR_STR[] = {

    "BLACK",
    "RED",
    "GREEN",
    "YELLOW",
    "BLUE",
    "MAGENTA",
    "CYAN",
    "WHITE"

};

#endif

static attrdef_t gpio_r_attrdef = {

    .name = "gpio_r",
    .description = "The red signal GPIO.",
    .type = ATTR_TYPE_STRING,
    .choices = gpio_choices,
    .modifiable = TRUE,
    .set = attr_set_gpio_r,
    .get = attr_get_gpio_r

};

static attrdef_t gpio_g_attrdef = {

    .name = "gpio_g",
    .description = "The green signal GPIO.",
    .type = ATTR_TYPE_STRING,
    .choices = gpio_choices,
    .modifiable = TRUE,
    .set = attr_set_gpio_g,
    .get = attr_get_gpio_g

};

static attrdef_t gpio_b_attrdef = {

    .name = "gpio_b",
    .description = "The blue signal GPIO.",
    .type = ATTR_TYPE_STRING,
    .choices = gpio_choices,
    .modifiable = TRUE,
    .set = attr_set_gpio_b,
    .get = attr_get_gpio_b

};

static attrdef_t *attrdefs[] = {

    &gpio_r_attrdef,
    &gpio_g_attrdef,
    &gpio_b_attrdef,
    NULL

};

#ifdef HAS_RGB0
static extra_info_t rgb0_extra_info;

static port_t _rgb0 = {

    .slot = PORT_SLOT_EXTRA0,

    .id = "rgb0",
    .type = PORT_TYPE_NUMBER,
    .min = RGB_MIN,
    .max = RGB_MAX,
    .step = UNDEFINED,
    .integer = TRUE,

    .flags = PORT_FLAG_OUTPUT,

    .extra_info = &rgb0_extra_info,

    .read_value = read_value,
    .write_value = write_value,
    .configure = configure,

    .attrdefs = attrdefs

};

port_t *rgb0 = &_rgb0;
#endif

#ifdef HAS_RGB1
static extra_info_t rgb1_extra_info;

static port_t _rgb1 = {

    .slot = PORT_SLOT_EXTRA2,

    .id = "rgb1",
    .type = PORT_TYPE_NUMBER,
    .min = RGB_MIN,
    .max = RGB_MAX,
    .step = UNDEFINED,
    .integer = TRUE,

    .flags = PORT_FLAG_OUTPUT,
    .extra_info = &rgb1_extra_info,

    .read_value = read_value,
    .write_value = write_value,
    .configure = configure,

    .attrdefs = attrdefs

};

port_t *rgb1 = &_rgb1;
#endif

#ifdef HAS_RGB2
static extra_info_t rgb2_extra_info;

static port_t _rgb2 = {

    .slot = PORT_SLOT_EXTRA2,

    .id = "rgb2",
    .type = PORT_TYPE_NUMBER,
    .min = RGB_MIN,
    .max = RGB_MAX,
    .step = UNDEFINED,
    .integer = TRUE,

    .flags = PORT_FLAG_OUTPUT,
    .extra_info = &rgb2_extra_info,

    .read_value = read_value,
    .write_value = write_value,
    .configure = configure,

    .attrdefs = attrdefs

};

port_t *rgb2 = &_rgb2;
#endif

#ifdef HAS_RGB3
static extra_info_t rgb3_extra_info;

static port_t _rgb3 = {

    .slot = PORT_SLOT_EXTRA3,

    .id = "rgb3",
    .type = PORT_TYPE_NUMBER,
    .min = RGB_MIN,
    .max = RGB_MAX,
    .step = UNDEFINED,
    .integer = TRUE,

    .flags = PORT_FLAG_OUTPUT,
    .extra_info = &rgb3_extra_info,

    .read_value = read_value,
    .write_value = write_value,
    .configure = configure,

    .attrdefs = attrdefs

};

port_t *rgb3 = &_rgb3;
#endif

#ifdef HAS_RGB4
static extra_info_t rgb4_extra_info;

static port_t _rgb4 = {

    .slot = PORT_SLOT_EXTRA4,

    .id = "rgb4",
    .type = PORT_TYPE_NUMBER,
    .min = RGB_MIN,
    .max = RGB_MAX,
    .step = UNDEFINED,
    .integer = TRUE,

    .flags = PORT_FLAG_OUTPUT,
    .extra_info = &rgb4_extra_info,

    .read_value = read_value,
    .write_value = write_value,
    .configure = configure,

    .attrdefs = attrdefs

};

port_t *rgb4 = &_rgb4;
#endif

#ifdef HAS_RGB5
static extra_info_t rgb5_extra_info;

static port_t _rgb5 = {

    .slot = PORT_SLOT_EXTRA5,

    .id = "rgb5",
    .type = PORT_TYPE_NUMBER,
    .min = RGB_MIN,
    .max = RGB_MAX,
    .step = UNDEFINED,
    .integer = TRUE,

    .flags = PORT_FLAG_OUTPUT,
    .extra_info = &rgb5_extra_info,

    .read_value = read_value,
    .write_value = write_value,
    .configure = configure,

    .attrdefs = attrdefs

};

port_t *rgb5 = &_rgb5;
#endif


double read_value(port_t *port) {
    return get_rgb_value(port);
}

bool write_value(port_t *port, double value) {
    int rgb = (int) value;

    DEBUG_RGB(port, "setting color %s", COLOR_STR[rgb]);

    set_rgb_value(port, rgb);

    GPIO_OUTPUT_SET(get_gpio_r(port), !!(rgb & 1));
    GPIO_OUTPUT_SET(get_gpio_g(port), !!(rgb & 2));
    GPIO_OUTPUT_SET(get_gpio_b(port), !!(rgb & 4));

    return TRUE;
}

void configure(port_t *port) {
    uint8 gpio_r = get_gpio_r(port);
    uint8 gpio_g = get_gpio_g(port);
    uint8 gpio_b = get_gpio_b(port);

    gpio_select_func(gpio_r);
    gpio_select_func(gpio_g);
    gpio_select_func(gpio_b);

    gpio_set_pullup(gpio_r, false);
    gpio_set_pullup(gpio_g, false);
    gpio_set_pullup(gpio_b, false);
}

int attr_get_gpio_r(port_t *port) {
    int i;
    uint8 value;

    /* read from persisted data */
    memcpy(&value, port->extra_data + GPIO_R_CONFIG_OFFS, 1);

    /* update cached value */
    set_gpio_r(port, value);

    for (i = 0; i < GPIO_CHOICES_LEN; i++) {
        if (gpio_mapping[i] == value) {
            /* return choice index */
            return i;
        }
    }

    return 0;
}

void attr_set_gpio_r(port_t *port, int index) {
    uint8 value = gpio_mapping[index];

    /* update cached value */
    set_gpio_r(port, value);

    /* write to persisted data */
    memcpy(port->extra_data + GPIO_R_CONFIG_OFFS, &value, 1);
}

int attr_get_gpio_g(port_t *port) {
    int i;
    uint8 value;

    /* read from persisted data */
    memcpy(&value, port->extra_data + GPIO_G_CONFIG_OFFS, 1);

    /* update cached value */
    set_gpio_g(port, value);

    for (i = 0; i < GPIO_CHOICES_LEN; i++) {
        if (gpio_mapping[i] == value) {
            /* return choice index */
            return i;
        }
    }

    return 0;
}

void attr_set_gpio_g(port_t *port, int index) {
    uint8 value = gpio_mapping[index];

    /* update cached value */
    set_gpio_g(port, value);

    /* write to persisted data */
    memcpy(port->extra_data + GPIO_G_CONFIG_OFFS, &value, 1);
}

int attr_get_gpio_b(port_t *port) {
    int i;
    uint8 value;

    /* read from persisted data */
    memcpy(&value, port->extra_data + GPIO_B_CONFIG_OFFS, 1);

    /* update cached value */
    set_gpio_b(port, value);

    for (i = 0; i < GPIO_CHOICES_LEN; i++) {
        if (gpio_mapping[i] == value) {
            /* return choice index */
            return i;
        }
    }

    return 0;
}

void attr_set_gpio_b(port_t *port, int index) {
    uint8 value = gpio_mapping[index];

    /* update cached value */
    set_gpio_b(port, value);

    /* write to persisted data */
    memcpy(port->extra_data + GPIO_B_CONFIG_OFFS, &value, 1);
}


void rgb_init_ports() {
#ifdef HAS_RGB0
    port_register(rgb0);
#endif

#ifdef HAS_RGB1
    port_register(rgb1);
#endif

#ifdef HAS_RGB2
    port_register(rgb2);
#endif

#ifdef HAS_RGB3
    port_register(rgb3);
#endif

#ifdef HAS_RGB4
    port_register(rgb4);
#endif

#ifdef HAS_RGB5
    port_register(rgb5);
#endif
}


#endif  /* HAS_RGB */
