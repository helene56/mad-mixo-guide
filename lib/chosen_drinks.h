#if !defined(CHOSEN_DRINKS)
#define CHOSEN_DRINKS

#include "mood_states.h"

#define MADNESS_COUNTER 10
enum drinks {VODKA, GIN, JUICE, LIME, TONIC};

enum states get_current_state();
void add_drink(enum drinks new_drink);


#endif // CHOSEN_DRINKS
