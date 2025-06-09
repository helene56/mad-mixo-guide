/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

typedef void (*pre_mix_cb)(void);
typedef void (*mix_cb)(int ml);
typedef void (*post_mix_cb)();

typedef struct
{
    const char *name;
    pre_mix_cb on_pre_mix;
    mix_cb on_mix;
    post_mix_cb on_post_mix;
    uint8_t danger_level;   // 0-255 (risk of disaster)

} potion_recipes;

void calibrate()
{
    printf("Calibrating pumps...\n");
}

void calibrate_quickly()
{
    printf("Calibrate only 1 pump...\n");
}

void pump_juice(int ml)
{
    printf("Pumping %d ml juice...\n", ml);
}

void pump_vodka(int ml)
{
    printf("Pumping %d ml vodka...\n", ml);
}

void pump_gin(int ml)
{
    printf("Pumping %d ml gin...\n", ml);
}

void pump_citrus()
{
    int citrus = 5;
    printf("Pumping %d ml citrus...\n", citrus);
}

void pump_ingredient(int ml)
{
    
}

void stir()
{
    printf("Stirring..\n");
}

void stir_violently()
{
    printf("Stirring.. Violently.\n");
}

void execute_recipe(const potion_recipes recipes[], int index, int ml)
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
        recipes[index].on_mix(ml);
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

int main(void)
{
    // recipes
    const potion_recipes juice_recipe = { .on_mix = pump_juice, .on_post_mix = stir};

    const potion_recipes vodka_mix_recipe = {.on_pre_mix = calibrate_quickly, .on_mix = pump_vodka, .on_post_mix = stir};

    const potion_recipes sour_vodka_recipe = {.on_pre_mix = calibrate, .on_mix = pump_vodka, .on_post_mix = pump_citrus};

    const potion_recipes gin_recipe = {.on_pre_mix = calibrate, .on_mix = pump_gin, .on_post_mix = stir_violently};

    potion_recipes recipes[] = {juice_recipe, vodka_mix_recipe, sour_vodka_recipe, gin_recipe};

    // pour recipes
    execute_recipe(recipes, 0, 50);

    // change recipe!
    recipes[0].on_pre_mix = calibrate_quickly;
    recipes[0].on_post_mix = stir_violently;
    printf("\n=== AFTER SWAPPING CALLBACKS ===\n");
    execute_recipe(recipes, 0, 50);
}
