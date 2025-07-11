#include <stdio.h>
#include <string.h>
#include "chosen_drinks.h"

static enum drinks chosen_drinks[MADNESS_COUNTER] = {0};
liquid_containers filled_containers[MAX_NUM_OF_LIQUIDS]  = {0};

// subtract from liquid container
void subtract_liquid(enum drinks drink, int ml)
{
    for (int i = 0; i < MAX_NUM_OF_LIQUIDS; ++i)
    {
        if (filled_containers[i].name == drink)
        {
            filled_containers[i].leftover = filled_containers[i].container_size - ml;
        }
        
    }
}


// add drink to chosen_drinks
void add_drink(enum drinks new_drink, int ml)
{
    static enum drinks *drinks_ptr = chosen_drinks; // pointer to chosen_drinks[0]
    enum drinks *drinks_end = chosen_drinks + MADNESS_COUNTER;   // one past the last element
    // subtract ml from drink
    subtract_liquid(new_drink, ml);

    if (drinks_ptr != drinks_end)
    {
        *drinks_ptr = new_drink;
        drinks_ptr++;
    }
    else
    {
        memset(chosen_drinks, 0, sizeof(chosen_drinks));
    }

}

// choose a new state based on chosen drinks
enum states get_current_state()
{
    int sum_of_drinks = 0;
    size_t size_chosen_drinks = sizeof(chosen_drinks)/sizeof(chosen_drinks[0]);
    for (int i = 0; i< size_chosen_drinks; ++i)
    {
        sum_of_drinks += (int)chosen_drinks[i];
    }

    enum states state = (enum states)(sum_of_drinks % (3));

    return state;
}

void add_liquids(enum drinks liquids[], size_t num_liquids)
{

    for (int i = 0; i < num_liquids; ++i)
    {
        filled_containers[i].name = liquids[i];
        filled_containers[i].container_size = DRINK_CONTAINER;
        filled_containers[i].leftover = DRINK_CONTAINER;
    }
}

void add_liquid(enum drinks liquid)
{

    for (int i = 0; i < MAX_NUM_OF_LIQUIDS; ++i)
    {
        if (filled_containers[i].container_size == 0)
        {
            filled_containers[i].name = liquid;
            filled_containers[i].container_size = DRINK_CONTAINER;
            filled_containers[i].leftover = DRINK_CONTAINER;
            return;
        }

    }
}