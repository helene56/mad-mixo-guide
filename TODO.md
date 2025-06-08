## Get hardware

3-6 Liquid Pumps (peristaltic or solenoid valves)

Liquid Containers (for "potions")

Load Cells or Flow Sensors (or fake it with timers)

RGB LEDs (for "magic ambiance")

OLED Display (to show cursed messages)

Rotary Encoder + Button (for recipe selection)

Buzzer/Speaker (for evil sound effects)

Optional Madness:

Conductivity Sensor (to detect "poison")

Water Sensor (to detect spills)

Microphone (for "voice-activated chaos")

Big Red Button (for emergency overrides)

WiFi/Bluetooth (for remote corruption)

## Define the Recipe Grimoire (Function Pointer Hell)
Create a struct full of callback for each recipe

example
```c
typedef struct {
    const char *name;       // "Elixir of Life"
    void (*pre_mix)(void);  // *calibrate_pumps
    void (*mix)(void);      // *pump_vodka, *stir
    void (*post_mix)(void); // *play_jingle
    uint8_t danger_level;   // 0-255 (risk of disaster)
} potion_recipe_t;
```

## implement core callbacks
actual functions referenced in the grimoire

```c
void mix_vodka_lime() {
    pump(25, VODKA_PUMP);  // 25ml vodka
    pump(10, LIME_PUMP);   // 10ml lime
    stir(3);               // Stir for 3s
}

void crash_mcu() {
    void (*death)() = (void (*)()) 0xDEADBEEF; // Jump to oblivion
    death();
}
```

## build the ui (recipe selector)
scroll thorugh grimoire[] with rotary encoder
button press to execute selected recipe

oled display to show the recipe name and danger level


## Add optional maddness

store recipes in non volatile memory
after x mixes randomly swap two callbacks

