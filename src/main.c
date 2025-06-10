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

#define MADNESS_COUNTER 10

// enum drinks {VODKA, GIN, JUICE, LIME, TONIC};
// enum states {NORMAL, DEVIOUS, PANIC};
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


void pump_ingredient(int ml, enum drinks drink)
{
    add_drink(drink);
    // NOTE: temp code to simulate selection of drinks
    if (drink == VODKA)
    {
        printf("Pumping %d ml vodka..\n", ml);
    }
    else if (drink == GIN)
    {
        printf("Pumping %d ml gin..\n", ml);
    }
    else if (drink == JUICE)
    {
        printf("Pumping %d ml juice..\n", ml);
    }
    else if (drink == LIME)
    {
        printf("Pumping %d ml lime..\n", ml);
    }
    else if (drink == TONIC)
    {
        printf("Pumping %d ml tonic..\n", ml);
    }

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



int main(void)
{
    // recipes
    const potion_recipes vodka_lime_recipe = {.on_pre_mix = calibrate, .on_mix = mix_vodka_lime, .on_post_mix = pump_citrus};
    const potion_recipes gin_recipe = {.on_pre_mix = calibrate, .on_mix = mix_gin_tonic, .on_post_mix = surprise_stir};
    potion_recipes recipes[] = {vodka_lime_recipe, gin_recipe};

    // pour recipes
    execute_recipe(recipes, 0);
    execute_recipe(recipes, 1);
    // change recipe!
    recipes[0].on_pre_mix = calibrate_quickly;
    recipes[0].on_post_mix = surprise_stir;
    printf("\n=== AFTER SWAPPING CALLBACKS ===\n");
    execute_recipe(recipes, 0);

    // if 'random' liquid has less than 'random' ml left in container
    // enter panic mode
    // panic mode should randomly replace callbacks
}
