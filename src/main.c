/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>
// user lib
#include "../lib/chosen_drinks.h"
#include "../lib/mood_states.h"

#define DRINK_CONTAINER 80; // ml

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
    uint8_t danger_level;   // 0-255 (risk of disaster)

} potion_recipes;


typedef struct
{
    const char *name;
    int container_size;
    int leftover;
} liquid_containers;

const char* get_drink_name(enum drinks d) 
{
    switch(d) 
    {
    case VODKA: return "vodka";
    case GIN:   return "gin";
    case JUICE: return "juice";
    case LIME:  return "lime";
    case TONIC: return "tonic";

    default:
        return NULL;

    }
}


void pump_ingredient(int ml, enum drinks drink)
{
    add_drink(drink, ml);
    // NOTE: temp code to simulate selection of drinks
    printf("Pumping %d ml %s..\n", ml, get_drink_name(drink));

}

void stir(int seconds)
{
    printf("Stirring for %d..\n", seconds);
}

void stir_violently(int seconds)
{
    printf("Stirring for %d.. Violently.\n", seconds);
}

void pump_citrus()
{
    int citrus = 5;
    printf("Pumping %d ml citrus...\n", citrus);
}

void execute_recipe(const potion_recipes recipes[], int index)
{
    
    if (recipes[index].on_pre_mix)
    {
        recipes[index].on_pre_mix();
    }
    else
    {
        printf("Missing pre mix recipe. \n");
    }
    if (recipes[index].on_mix)
    {
        recipes[index].on_mix();
    }
    else
    {
        printf("Missing mix recipe. \n");
    }
    if (recipes[index].on_post_mix)
    {
        recipes[index].on_post_mix();
    }
    else
    {
        printf("Missing post mix recipe. \n");
    }
   
}


void calibrate()
{
    printf("Calibrating pumps...\n");
}

void calibrate_quickly()
{
    printf("Calibrate only 1 pump...\n");
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
    case 0 :
        printf("Current state :::: NORMAL\n");
        break;
    case 1 :
        printf("Current state :::: DEVIOUS\n");
    case 2 :
        printf("Current state :::: PANIC\n");
    
    default:
        break;
    }
}



int main(void)
{
    // char *drink_names[] = {"vodka", "gin", "juice", "lime", "tonic"};
    // select which drinks have been added
    // add later to uart or maybe even oled screen
    enum drinks selected_liquids[] = {LIME, JUICE, VODKA}; 
    liquid_containers filled_containers[sizeof(selected_liquids)/sizeof(selected_liquids[0])];
    for (int i = 0; i < sizeof(selected_liquids)/sizeof(selected_liquids[0]); ++i)
    {
        
        filled_containers[i].name = get_drink_name(selected_liquids[i]);
        filled_containers[i].container_size = DRINK_CONTAINER;
        filled_containers[i].leftover = DRINK_CONTAINER;
    }

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
    printf("\n=== AFTER SWAPPING CALLBACKS ===\n");
    execute_recipe(recipes, 0);

    // if 'random' liquid has less than 'random' ml left in container
    // enter panic mode
    // panic mode should randomly replace callbacks
}
