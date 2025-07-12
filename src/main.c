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

static lv_group_t *g;
lv_indev_t *button_indev;
lv_indev_data_t *button_data;

typedef void (*pre_mix_cb)(void);
typedef void (*mix_cb)(void);
typedef void (*post_mix_cb)(void);

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

int get_drink_enum(char *drink_choice)
{
    if (strcmp(drink_choice, "vodka") == 0)
    {
        return VODKA;
    }
    else if (strcmp(drink_choice, "gin") == 0)
    {
        return GIN;
    }
    else if (strcmp(drink_choice, "juice") == 0)
    {
        return JUICE;
    }
    else if (strcmp(drink_choice, "lime") == 0)
    {
        return LIME;
    }
    else if (strcmp(drink_choice, "tonic") == 0)
    {
        return TONIC;
    }
    else
    {
        return -1;
    }
}

void pump_ingredient(int ml, enum drinks drink)
{
    add_drink(drink, ml);
    // NOTE: temp code to simulate selection of drinks
    printk("Pumping %d ml %s..\n", ml, get_drink_name(drink));
}

void stir(int seconds)
{
    printk("Stirring for %d..\n", seconds);
}

void stir_violently(int seconds)
{
    printk("Stirring for %d.. Violently.\n", seconds);
}

void pump_citrus()
{
    int citrus = 5;
    printk("Pumping %d ml citrus...\n", citrus);
}

void execute_recipe(const potion_recipes recipes[], int index)
{

    if (recipes[index].on_pre_mix)
    {
        recipes[index].on_pre_mix();
    }
    else
    {
        printk("Missing pre mix recipe. \n");
    }
    if (recipes[index].on_mix)
    {
        recipes[index].on_mix();
    }
    else
    {
        printk("Missing mix recipe. \n");
    }
    if (recipes[index].on_post_mix)
    {
        recipes[index].on_post_mix();
    }
    else
    {
        printk("Missing post mix recipe. \n");
    }
}

void calibrate()
{
    printk("Calibrating pumps...\n");
}

void calibrate_quickly()
{
    printk("Calibrate only 1 pump...\n");
}

void surprise_stir()
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

void mix_vodka_lime()
{
    pump_ingredient(50, VODKA);
    pump_ingredient(10, LIME);
}

void mix_gin_tonic()
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

static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
    switch (evt->type)
    {
    case UART_RX_RDY:
        if (evt->data.rx.len >= RECEIVE_BUFF_SIZE)
        {
            printk("Error: Received message too long!\n");
            return;
        }
        if ((evt->data.rx.len) > 0)
        {
            rx_buf[evt->data.rx.len] = '\0'; // ensure string is properly terminated
            int drink = get_drink_enum(&evt->data.rx.buf[evt->data.rx.offset]);
            if (drink > -1)
            {
                const char *drink_name = get_drink_name((enum drinks)drink);

                printk("%s is a valid choice\n", drink_name);
                // add to filled_containers
                add_liquid((enum drinks)drink);
            }
            else if (strcmp(&evt->data.rx.buf[evt->data.rx.offset], "confirm") == 0)
            {
                printk("confirmed drink containers\n");
                confirmed = true;
                // here should no more liquied be added
                // for (int i = 0; i < 3; ++i)
                // {
                //     printk("%s has %d ml left.\n", get_drink_name(filled_containers[i].name),
                //            filled_containers[i].leftover);
                // }
            }
            else
            {
                printk("not valid.\n");
            }
        }
        break;
    case UART_RX_DISABLED:
        uart_rx_enable(dev, rx_buf, sizeof rx_buf, RECEIVE_TIMEOUT);
        break;
    default:
        break;
    }
}

static lv_obj_t *list1;

void lv_make_drink_screen()
{
}

static void event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target_obj(e);
    if (code == LV_EVENT_PRESSED)
    {
        // LV_UNUSED(obj);
        LV_LOG_USER("Clicked: %s", lv_list_get_button_text(list1, obj));
        LOG_INF("Clicked: %s", lv_list_get_button_text(list1, obj));
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
    const potion_recipes vodka_lime_recipe = {.name = "vodka lime", .on_pre_mix = calibrate, .on_mix = mix_vodka_lime, .on_post_mix = pump_citrus};
    const potion_recipes gin_recipe = {.name = "gin tonic", .on_pre_mix = calibrate, .on_mix = mix_gin_tonic, .on_post_mix = surprise_stir};
    const potion_recipes unknown_recipe = {.name = "Aw@k3#d", .on_pre_mix = calibrate, .on_mix = mix_gin_tonic, .on_post_mix = surprise_stir};
    potion_recipes recipes[] = {vodka_lime_recipe, gin_recipe, unknown_recipe};
    // table test
    size_t recipes_size = sizeof(recipes) / sizeof(recipes[0]);
    lv_example_list_1(recipes, recipes_size);
    lv_timer_handler();
    display_blanking_off(display_dev);

    // int ret;
    // if (!device_is_ready(uart))
    // {
    //     printk("UART device not ready\r\n");
    //     return 1;
    // }

    // ret = uart_callback_set(uart, uart_cb, NULL);
    // if (ret)
    // {
    //     return 1;
    // }

    // ret = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_US);
    // if (ret)
    // {
    //     return 1;
    // }

    // ret = uart_rx_enable(uart, rx_buf, sizeof rx_buf, RECEIVE_TIMEOUT);
    // if (ret)
    // {
    //     return 1;
    // }

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
