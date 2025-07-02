/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * PWM Fading Blinky Sample for nRF5340 DK
 * 
 * This application demonstrates smooth LED fading effects using PWM (Pulse Width Modulation).
 * Instead of simply turning LEDs on/off like traditional blinky, this creates smooth
 * brightness transitions by varying the PWM duty cycle.
 * 
 * Key concepts demonstrated:
 * - PWM device tree integration
 * - Duty cycle manipulation for brightness control
 * - Sequential LED control with fading effects
 * - Error handling for PWM operations
 */

#include <zephyr/kernel.h>      /* Core Zephyr kernel functions */
#include <zephyr/drivers/pwm.h> /* PWM driver API */
#include <zephyr/sys/printk.h>  /* Console output functions */

/*
 * PWM Configuration Constants
 * These define the timing and behavior of the fading effect
 */
#define PWM_PERIOD_US   1000U  /* PWM period in microseconds (1kHz frequency)
                                * Lower frequency = smoother fading but more visible flicker
                                * Higher frequency = less smooth but no visible flicker */

#define FADE_STEP_MS    10     /* Time between each fade step in milliseconds
                                * Smaller values = smoother but slower fading
                                * Larger values = faster but more stepped fading */

#define FADE_STEPS      100    /* Number of steps in fade transition
                                * More steps = smoother fading but takes longer
                                * Fewer steps = faster but more noticeable steps */

/*
 * Device Tree Node Definitions
 * These macros get the PWM LED nodes from the device tree overlay
 * DT_ALIAS() retrieves nodes defined in the "aliases" section
 */
#define PWM_LED0_NODE   DT_ALIAS(pwm_led0)  /* References pwm_led0 alias */
#define PWM_LED1_NODE   DT_ALIAS(pwm_led1)  /* References pwm_led1 alias */
#define PWM_LED2_NODE   DT_ALIAS(pwm_led2)  /* References pwm_led2 alias */
#define PWM_LED3_NODE   DT_ALIAS(pwm_led3)  /* References pwm_led3 alias */

/*
 * PWM Device Specifications
 * These structures contain all the information needed to control each PWM LED
 * PWM_DT_SPEC_GET() extracts PWM controller, channel, period, and flags from device tree
 * 
 * Fallback mechanism: If pwm-led nodes don't exist, try regular led nodes
 * This provides compatibility with boards that don't have PWM LED definitions
 */
#if DT_NODE_EXISTS(PWM_LED0_NODE)
static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(PWM_LED0_NODE);
#else
#define PWM_LED0_NODE   DT_ALIAS(led0)  /* Fallback to regular LED node */
static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET_BY_IDX(PWM_LED0_NODE, 0);
#endif

#if DT_NODE_EXISTS(PWM_LED1_NODE)
static const struct pwm_dt_spec pwm_led1 = PWM_DT_SPEC_GET(PWM_LED1_NODE);
#else
#define PWM_LED1_NODE   DT_ALIAS(led1)
static const struct pwm_dt_spec pwm_led1 = PWM_DT_SPEC_GET_BY_IDX(PWM_LED1_NODE, 0);
#endif

#if DT_NODE_EXISTS(PWM_LED2_NODE)
static const struct pwm_dt_spec pwm_led2 = PWM_DT_SPEC_GET(PWM_LED2_NODE);
#else
#define PWM_LED2_NODE   DT_ALIAS(led2)
static const struct pwm_dt_spec pwm_led2 = PWM_DT_SPEC_GET_BY_IDX(PWM_LED2_NODE, 0);
#endif

#if DT_NODE_EXISTS(PWM_LED3_NODE)
static const struct pwm_dt_spec pwm_led3 = PWM_DT_SPEC_GET(PWM_LED3_NODE);
#else
#define PWM_LED3_NODE   DT_ALIAS(led3)
static const struct pwm_dt_spec pwm_led3 = PWM_DT_SPEC_GET_BY_IDX(PWM_LED3_NODE, 0);
#endif

/*
 * Array of PWM LED specifications for easy iteration
 * This allows us to loop through all LEDs instead of handling each individually
 */
static const struct pwm_dt_spec *pwm_leds[] = {
    &pwm_led0,  /* LED 1 on nRF5340 DK */
    &pwm_led1,  /* LED 2 on nRF5340 DK */
    &pwm_led2,  /* LED 3 on nRF5340 DK */
    &pwm_led3   /* LED 4 on nRF5340 DK */
};

#define NUM_LEDS ARRAY_SIZE(pwm_leds)  /* Calculate number of LEDs automatically */

/**
 * @brief Fade LED in or out with smooth transition
 * 
 * This function creates a smooth fading effect by gradually changing the PWM duty cycle.
 * PWM duty cycle determines how long the signal is HIGH vs LOW in each period:
 * - 0% duty cycle = LED off (signal always LOW)
 * - 50% duty cycle = LED at half brightness (signal HIGH 50% of time)
 * - 100% duty cycle = LED at full brightness (signal always HIGH)
 * 
 * @param led_spec Pointer to PWM LED specification structure
 * @param fade_in true for fade in (dark to bright), false for fade out (bright to dark)
 */
static void fade_led(const struct pwm_dt_spec *led_spec, bool fade_in)
{
    uint32_t pulse_width;  /* PWM pulse width in microseconds */
    
    /* 
     * Loop through all fade steps to create smooth transition
     * Each step calculates a new pulse width based on the current step
     */
    for (int step = 0; step <= FADE_STEPS; step++) {
        if (fade_in) {
            /* Fade in: Start at 0% and increase to 100% */
            pulse_width = (PWM_PERIOD_US * step) / FADE_STEPS;
        } else {
            /* Fade out: Start at 100% and decrease to 0% */
            pulse_width = (PWM_PERIOD_US * (FADE_STEPS - step)) / FADE_STEPS;
        }
        
        /*
         * Set the PWM signal with calculated pulse width
         * pwm_set_dt() configures:
         * - PWM period (how long each cycle lasts)
         * - Pulse width (how long signal is HIGH in each cycle)
         * The ratio pulse_width/period determines brightness
         */
        int ret = pwm_set_dt(led_spec, PWM_PERIOD_US, pulse_width);
        if (ret < 0) {
            printk("Error setting PWM: %d\n", ret);
            return;  /* Exit on error to prevent further issues */
        }
        
        /* 
         * Small delay between steps creates the fading effect
         * Without this delay, the change would be instantaneous
         */
        k_msleep(FADE_STEP_MS);
    }
}

/**
 * @brief Set LED brightness to specific percentage
 * 
 * This function provides direct brightness control without fading animation.
 * Useful for setting initial states or immediate brightness changes.
 * 
 * @param led_spec Pointer to PWM LED specification structure
 * @param brightness Brightness percentage (0-100)
 *                   0 = completely off, 100 = maximum brightness
 */
static void set_led_brightness(const struct pwm_dt_spec *led_spec, uint8_t brightness)
{
    /* Convert percentage to pulse width in microseconds */
    uint32_t pulse_width = (PWM_PERIOD_US * brightness) / 100;
    
    /* Apply the PWM setting immediately */
    int ret = pwm_set_dt(led_spec, PWM_PERIOD_US, pulse_width);
    if (ret < 0) {
        printk("Error setting PWM brightness: %d\n", ret);
    }
}

/**
 * @brief Turn off all LEDs immediately
 * 
 * Sets all LEDs to 0% brightness (pulse width = 0).
 * Useful for initialization and cleanup.
 */
static void turn_off_all_leds(void)
{
    /* Loop through all LEDs and set them to off */
    for (int i = 0; i < NUM_LEDS; i++) {
        pwm_set_dt(pwm_leds[i], PWM_PERIOD_US, 0);  /* 0 pulse width = LED off */
    }
}

/**
 * @brief Main application entry point
 * 
 * This function:
 * 1. Initializes and checks PWM devices
 * 2. Runs the main LED fading loop
 * 3. Cycles through each LED with fade in/out effects
 * 
 * @return 0 on success, negative error code on failure
 */
int main(void)
{
    int ret;  /* Variable to store return codes for error checking */
    
    printk("PWM Fading Blinky Sample for nRF5340 DK\n");
    printk("This sample demonstrates smooth LED fading using PWM\n");
    
    /*
     * Device Readiness Check
     * Before using any PWM device, we must verify it's properly initialized
     * device_is_ready() returns true if the device driver is loaded and functional
     */
    for (int i = 0; i < NUM_LEDS; i++) {
        if (!device_is_ready(pwm_leds[i]->dev)) {
            printk("Error: PWM device %s is not ready\n", pwm_leds[i]->dev->name);
            return -1;  /* Exit with error code */
        }
        printk("PWM LED %d ready (device: %s)\n", i, pwm_leds[i]->dev->name);
    }
    
    /*
     * Initialize all LEDs to off state
     * This ensures a clean starting point regardless of previous state
     */
    turn_off_all_leds();
    printk("All LEDs initialized to OFF state\n");
    
    /*
     * Main Application Loop
     * Continuously cycles through LEDs with fading effects
     */
    int current_led = 0;  /* Index of currently active LED */
    
    while (1) {  /* Infinite loop - typical for embedded applications */
        printk("Fading LED %d (User LED %d on board)\n", current_led, current_led + 1);
        
        /*
         * Fade in sequence:
         * 1. Gradually increase brightness from 0% to 100%
         * 2. Hold at full brightness briefly
         * 3. Gradually decrease brightness from 100% to 0%
         */
        
        /* Phase 1: Fade in (dark to bright) */
        fade_led(pwm_leds[current_led], true);
        
        /* Phase 2: Hold at full brightness */
        k_msleep(200);  /* Keep LED on for 200ms */
        
        /* Phase 3: Fade out (bright to dark) */
        fade_led(pwm_leds[current_led], false);
        
        /*
         * Move to next LED in sequence
         * Modulo operation (%) ensures we wrap around to LED 0 after the last LED
         * This creates a continuous cycling pattern: 0 -> 1 -> 2 -> 3 -> 0 -> ...
         */
        current_led = (current_led + 1) % NUM_LEDS;
        
        /* Small pause between LEDs for visual separation */
        k_msleep(100);
    }
    
    /*
     * This return statement will never be reached due to the infinite loop above
     * It's included for completeness and to satisfy the function signature
     */
    return 0;
}