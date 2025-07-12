/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>
#include <zephyr/drivers/uart.h>
// user lib
#include "../lib/chosen_drinks.h"
#include "../lib/mood_states.h"
// lvgl
#include <lvgl.h>
// display
#include <zephyr/drivers/display.h>
// encoder
#include <lvgl_input_device.h>
// logging
#include <zephyr/logging/log.h>

#include <zephyr/input/input.h>

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
LOG_MODULE_REGISTER(app);

const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));
static const struct device *lvgl_encoder =
    DEVICE_DT_GET(DT_COMPAT_GET_ANY_STATUS_OKAY(zephyr_lvgl_encoder_input));

static uint8_t tx_buf[] = {"Software replacement for LCD\r\n"
                           "Please type 'lime' or 'vodka'\r\n"};

#define RECEIVE_BUFF_SIZE 10
static uint8_t rx_buf[RECEIVE_BUFF_SIZE] = {0};
#define RECEIVE_TIMEOUT 100
static bool confirmed = false;
enum states CURRENT_STATE = NORMAL;

enum drink_states
{
    PREMIX,
    MIX,
    AFTERMIX
};
enum drink_states CURRENT_MIX = PREMIX;

static lv_group_t *g;
lv_indev_t *button_indev;
lv_indev_data_t *button_data;

static lv_obj_t *list1;
static lv_obj_t *drink_status_label;

// screens
static lv_obj_t *scr_2;
// progress bar
static lv_obj_t *bar;

typedef struct
{
    potion_recipes *user_recipe;
    lv_event_t *user_event;
} timer_user_data;

static timer_user_data my_user_data;

typedef void (*pre_mix_cb)(lv_event_t *e);
typedef void (*mix_cb)(lv_event_t *e);
typedef void (*post_mix_cb)(lv_event_t *e);

typedef struct
{
    const char *name;
    pre_mix_cb on_pre_mix;
    mix_cb on_mix;
    post_mix_cb on_post_mix;
    uint8_t danger_level; // 0-255 (risk of disaster)

} potion_recipes;

const char *get_drink_name(enum drinks d)
{
    switch (d)
    {
    case VODKA:
        return "vodka";
    case GIN:
        return "gin";
    case JUICE:
        return "juice";
    case LIME:
        return "lime";
    case TONIC:
        return "tonic";

    default:
        return NULL;
    }
}


void pump_ingredient(int ml, enum drinks drink)
{
    add_drink(drink, ml);
    // NOTE: temp code to simulate selection of drinks
    printk("Pumping %d ml %s..\n", ml, get_drink_name(drink));
    char buffer[32]; // Make sure it's big enough
    snprintf(buffer, sizeof(buffer), "Pumping %d ml %s..\n", ml, get_drink_name(drink));
    lv_label_set_text(drink_status_label, buffer);
}

void stir(int seconds)
{
    printk("Stirring for %d..\n", seconds);
    // Create buffer and format the string into it
    char buffer[32]; // Make sure it's big enough
    snprintf(buffer, sizeof(buffer), "Stirring for %d..\n", seconds);
    lv_label_set_text(drink_status_label, buffer);
}

void stir_violently(int seconds)
{
    printk("Stirring for %d.. Violently.\n", seconds);
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Stirring for %d.. Violently.\n", seconds);
    lv_label_set_text(drink_status_label, buffer);
}

void pump_citrus()
{
    int citrus = 5;
    printk("Pumping %d ml citrus...\n", citrus);
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Pumping %d ml citrus...\n", citrus);
    lv_label_set_text(drink_status_label, buffer);
}


void my_timer(lv_timer_t *timer)
{
    /* Use the user_data */
    static bool drink_finished = false;
    timer_user_data *user_data = lv_timer_get_user_data(timer);
    lv_event_t *event = user_data->user_event;
    printf("my_timer called\n");
    // printf("Timer call: recipe = %p, mix = %d\n", user_data->user_recipe, CURRENT_MIX);
    if (CURRENT_MIX == MIX)
    {
        if (user_data->user_recipe->on_mix)
        {
            user_data->user_recipe->on_mix(event);
            lv_bar_set_value(bar, 66, LV_ANIM_ON);
        }
        CURRENT_MIX = AFTERMIX;
    }
    else if (CURRENT_MIX == AFTERMIX && !drink_finished)
    {
        if (user_data->user_recipe->on_post_mix)
        {
            user_data->user_recipe->on_post_mix(event);
            lv_bar_set_value(bar, 93, LV_ANIM_ON);
            drink_finished = true;
        }
    }
    else if (drink_finished)
    {
        lv_label_set_text(drink_status_label, "Drink finished");
        lv_bar_set_value(bar, 100, LV_ANIM_ON);
    }
}

void execute_one_recipe(const potion_recipes *recipe, lv_event_t *e)
{
    my_user_data.user_event = e;
    my_user_data.user_recipe = recipe;

    lv_scr_load(scr_2);
    lv_timer_t *timer = lv_timer_create(my_timer, 2500, &my_user_data);

    if (recipe->on_pre_mix)
    {
        recipe->on_pre_mix(e);
        CURRENT_MIX = MIX;
        lv_bar_set_value(bar, 33, LV_ANIM_ON);
    }
    lv_timer_set_repeat_count(timer, 3);
}

void calibrate(lv_event_t *e)
{
    printk("Calibrating pumps...\n");
    lv_label_set_text(drink_status_label, "Calibrating pumps...\n");
}

void calibrate_quickly(lv_event_t *e)
{
    printk("Calibrate only 1 pump...\n");
    lv_label_set_text(drink_status_label, "Calibrate only 1 pump...\n");
}

void surprise_stir(lv_event_t *e)
{
    // get temperature sensor value or maybe another sensor?
    // use that to produce a random value 3 - 15 seconds
    int random_val = 25; // get sensor value here
    int offset = (random_val % 5) - 2;
    if (random_val > 15)
    {
        random_val = 15;
    }
    else
    {
        random_val += offset;
    }

    stir(random_val);
}

void mix_vodka_lime(lv_event_t *e)
{
    pump_ingredient(50, VODKA);
    pump_ingredient(10, LIME);
}

void mix_gin_tonic(lv_event_t *e)
{
    pump_ingredient(30, GIN);
    pump_ingredient(10, TONIC);
    pump_ingredient(5, LIME);
}

void helper_print_state(enum states current_state)
{

    switch ((int)current_state)
    {
    case 0:
        printk("Current state :::: NORMAL\n");
        break;
    case 1:
        printk("Current state :::: DEVIOUS\n");
    case 2:
        printk("Current state :::: PANIC\n");

    default:
        break;
    }
}

void create_new_screen()
{
    scr_2 = lv_obj_create(NULL);
    drink_status_label = lv_label_create(scr_2);
    lv_obj_set_width(drink_status_label, 200);
    lv_obj_align(drink_status_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(drink_status_label, &lv_font_montserrat_24, 0);
}

void create_progress_bar()
{
    static lv_style_t style_bg;
    static lv_style_t style_indic;

    lv_style_init(&style_bg);
    lv_style_set_border_color(&style_bg, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_border_width(&style_bg, 2);
    lv_style_set_pad_all(&style_bg, 6); /*To make the indicator smaller*/
    lv_style_set_radius(&style_bg, 6);
    lv_style_set_anim_duration(&style_bg, 1000);

    lv_style_init(&style_indic);
    lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_radius(&style_indic, 3);

    bar = lv_bar_create(scr_2);
    lv_obj_remove_style_all(bar); /*To have a clean start*/
    lv_obj_add_style(bar, &style_bg, 0);
    lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);

    lv_obj_set_size(bar, 200, 20);
    lv_obj_center(bar);
    lv_obj_align_to(bar, NULL, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_bar_set_value(bar, 0, LV_ANIM_ON);
}

void lv_make_drink_screen(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    potion_recipes *data = lv_obj_get_user_data(btn);
    execute_one_recipe(data, e);
    lv_scr_load(scr_2);
}

static void event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target_obj(e);
    lv_obj_t *btn = lv_event_get_target(e);
    if (code == LV_EVENT_PRESSED)
    {
        // LV_UNUSED(obj);
        LV_LOG_USER("Clicked: %s", lv_list_get_button_text(list1, obj));
        LOG_INF("Clicked: %s", lv_list_get_button_text(list1, obj));
        lv_make_drink_screen(e);
    }
}

void lv_example_list_1(potion_recipes *recipes, size_t recipes_size)
{

    g = lv_group_create();
    lv_group_set_default(g);
    lv_indev_t *encoder_indev = lvgl_input_get_indev(lvgl_encoder);
    lv_indev_set_group(encoder_indev, g);

    /*Create a list*/
    list1 = lv_list_create(lv_screen_active());
    lv_group_add_obj(g, list1);
    lv_obj_set_size(list1, lv_pct(100), lv_pct(100));
    lv_obj_center(list1);
    // set style
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_border_width(&style, 0);
    lv_obj_add_style(list1, &style, 0);
    /*Add buttons to the list*/
    lv_obj_t *btn;
    // buttons
    for (int i = 0; i < recipes_size; i++)
    {
        btn = lv_list_add_button(list1, NULL, recipes[i].name);
        lv_obj_set_style_text_font(btn, &lv_font_montserrat_24, 0);
        lv_obj_set_size(btn, lv_pct(100), lv_pct(30));
        // lv_obj_update_layout(btn); // to get height
        lv_obj_set_style_text_align(btn, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_pad_top(btn, 42 - 12, 0); // Add 10px padding at the top
        lv_group_add_obj(g, btn);
        lv_obj_set_user_data(btn, &recipes[i]); // Explicitly set
        lv_obj_add_event_cb(btn, event_handler, LV_EVENT_PRESSED, NULL);
    }
}

int main(void)
{

    // configure display
    const struct device *display_dev;
    display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display_dev))
    {
        LOG_ERR("Device not ready, aborting.");
        return 0;
    }
    // add recipes
    // recipes
    static const potion_recipes vodka_lime_recipe = {.name = "vodka lime", .on_pre_mix = calibrate, .on_mix = mix_vodka_lime, .on_post_mix = pump_citrus};
    static const potion_recipes gin_recipe = {.name = "gin tonic", .on_pre_mix = calibrate, .on_mix = mix_gin_tonic, .on_post_mix = surprise_stir};
    static const potion_recipes unknown_recipe = {.name = "Aw@k3#d", .on_pre_mix = calibrate, .on_mix = mix_gin_tonic, .on_post_mix = surprise_stir};
    static potion_recipes recipes[] = {vodka_lime_recipe, gin_recipe, unknown_recipe};
    // table test
    size_t recipes_size = sizeof(recipes) / sizeof(recipes[0]);
    create_new_screen();
    create_progress_bar();
    lv_example_list_1(recipes, recipes_size);
    lv_timer_handler();
    display_blanking_off(display_dev);

    // char *drink_names[] = {"vodka", "gin", "juice", "lime", "tonic"};
    // select which drinks have been added
    // add later to uart or maybe even oled screen
    // enum drinks selected_liquids[] = {LIME, JUICE, VODKA};
    // initialize filled containers

    // user selected drinks have been added
    // add_liquids(selected_liquids, 3);

    helper_print_state(CURRENT_STATE);

    // while(!confirmed)
    // {

    // }

    // pour recipes
    // execute_recipe(recipes, 0);
    // execute_recipe(recipes, 1);

    // CURRENT_STATE = get_current_state();
    // helper_print_state(CURRENT_STATE);
    // // change recipe!
    // recipes[0].on_pre_mix = calibrate_quickly;
    // recipes[0].on_post_mix = surprise_stir;
    // printk("\n=== AFTER SWAPPING CALLBACKS ===\n");
    // execute_recipe(recipes, 0);

    // if 'random' liquid has less than 'random' ml left in container
    // enter panic mode
    // panic mode should randomly replace callbacks
    for (int i = 0; i < 3; ++i)
    {
        printk("%s has %d ml left.\n", get_drink_name(filled_containers[i].name),
               filled_containers[i].leftover);
    }

    while (1)
    {
        lv_timer_handler();
        // debug read button
        // bool val = gpio_pin_get_dt(&button);
        // printk("button: %d\n", val);
        k_sleep(K_MSEC(10));
    }
}
