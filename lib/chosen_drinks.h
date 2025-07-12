#if !defined(CHOSEN_DRINKS)
#define CHOSEN_DRINKS

#include "mood_states.h"

#define MADNESS_COUNTER 10
#define MAX_NUM_OF_LIQUIDS 5
#define DRINK_CONTAINER 80; // ml
enum drinks
{
    VODKA,
    GIN,
    JUICE,
    LIME,
    TONIC
};

typedef struct
{
    enum drinks name;
    int container_size;
    int leftover;
} liquid_containers;

extern liquid_containers filled_containers[MAX_NUM_OF_LIQUIDS];

enum states get_current_state();
void add_drink(enum drinks new_drink, int ml);
void add_liquids(enum drinks liquids[], size_t num_of_liquids);
void subtract_liquid(enum drinks drink, int ml);
void add_liquid(enum drinks liquid);

#endif // CHOSEN_DRINKS
