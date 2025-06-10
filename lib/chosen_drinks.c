#include <stdio.h>
#include <string.h>
#include "chosen_drinks.h"

static enum drinks chosen_drinks[MADNESS_COUNTER] = {0};


// add drink to chosen_drinks
void add_drink(enum drinks new_drink)
{
    static enum drinks *drinks_ptr = chosen_drinks; // pointer to chosen_drinks[0]
    enum drinks *drinks_end = chosen_drinks + MADNESS_COUNTER;   // one past the last element

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
enum states handle_drink_event()
{
    int sum_of_drinks = 0;
    size_t size_chosen_drinks = sizeof(chosen_drinks)/sizeof(chosen_drinks[0]);
    for (int i = 0; i< size_chosen_drinks; ++i)
    {
        sum_of_drinks += (int)chosen_drinks[i];
    }

    enum states state = (enum states)(sum_of_drinks % (size_chosen_drinks + 1));

    return state;
}