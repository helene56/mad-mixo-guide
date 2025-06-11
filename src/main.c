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

const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));

static uint8_t tx_buf[] = {"Software replacement for LCD\r\n"
                           "Please type 'lime' or 'vodka'\r\n"};

#define RECEIVE_BUFF_SIZE 10
static uint8_t rx_buf[RECEIVE_BUFF_SIZE] = {0};
#define RECEIVE_TIMEOUT 100

enum states CURRENT_STATE = NORMAL;

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
    if (strncmp(drink_choice, "vodka", strlen("vodka")) == 0)
    {
        return VODKA;
    }
    else if (strncmp(drink_choice, "gin", strlen("gin")) == 0)
    {
        return GIN;
    }
    else if (strncmp(drink_choice, "juice", strlen("juice")) == 0)
    {
        return JUICE;
    }
    else if (strncmp(drink_choice, "lime", strlen("lime")) == 0)
    {
        return LIME;
    }
    else if (strncmp(drink_choice, "tonic", strlen("tonic")) == 0)
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
        if ((evt->data.rx.len) > 1)
        {
            int drink = get_drink_enum(&evt->data.rx.buf[evt->data.rx.offset]);
            printk("%s is a valid choice\n", get_drink_name((enum drinks) drink));
            if (strncmp(&evt->data.rx.buf[evt->data.rx.offset], "lime", strlen("lime")) == 0)
            {
                printk("you typed lime!\n");
            }
            else if (strncmp(&evt->data.rx.buf[evt->data.rx.offset], "vodka", strlen("vodka")) == 0)
            {
                printk("you typed vodka\n");
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

int main(void)
{
    int ret;
    if (!device_is_ready(uart))
    {
        printk("UART device not ready\r\n");
        return 1;
    }

    ret = uart_callback_set(uart, uart_cb, NULL);
    if (ret)
    {
        return 1;
    }

    ret = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_US);
    if (ret)
    {
        return 1;
    }

    ret = uart_rx_enable(uart, rx_buf, sizeof rx_buf, RECEIVE_TIMEOUT);
    if (ret)
    {
        return 1;
    }

    // char *drink_names[] = {"vodka", "gin", "juice", "lime", "tonic"};
    // select which drinks have been added
    // add later to uart or maybe even oled screen
    enum drinks selected_liquids[] = {LIME, JUICE, VODKA};
    // initialize filled containers
    for (int i = 0; i < MAX_NUM_OF_LIQUIDS; ++i)
    {
        filled_containers[i].name = -1;
        filled_containers[i].container_size = -1;
        filled_containers[i].leftover = -1;
    }
    // user selected drinks have been added
    add_liquids(selected_liquids, 3);

    helper_print_state(CURRENT_STATE);
    // recipes
    const potion_recipes vodka_lime_recipe = {.on_pre_mix = calibrate, .on_mix = mix_vodka_lime, .on_post_mix = pump_citrus};
    const potion_recipes gin_recipe = {.on_pre_mix = calibrate, .on_mix = mix_gin_tonic, .on_post_mix = surprise_stir};
    potion_recipes recipes[] = {vodka_lime_recipe, gin_recipe};

    // pour recipes
    execute_recipe(recipes, 0);
    execute_recipe(recipes, 1);

    CURRENT_STATE = get_current_state();
    helper_print_state(CURRENT_STATE);
    // change recipe!
    recipes[0].on_pre_mix = calibrate_quickly;
    recipes[0].on_post_mix = surprise_stir;
    printk("\n=== AFTER SWAPPING CALLBACKS ===\n");
    execute_recipe(recipes, 0);

    // if 'random' liquid has less than 'random' ml left in container
    // enter panic mode
    // panic mode should randomly replace callbacks
    for (int i = 0; i < 3; ++i)
    {
        printk("%s has %d ml left.\n", get_drink_name(filled_containers[i].name),
               filled_containers[i].leftover);
    }
}
