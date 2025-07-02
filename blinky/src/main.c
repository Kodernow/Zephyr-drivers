/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/sys/printk.h>

/* PWM period in microseconds (1kHz frequency) */
#define PWM_PERIOD_US   1000U

/* Sleep time between fade steps */
#define FADE_STEP_MS    10

/* Number of fade steps */
#define FADE_STEPS      100

/* PWM device tree nodes */
#define PWM_LED0_NODE   DT_ALIAS(pwm_led0)
#define PWM_LED1_NODE   DT_ALIAS(pwm_led1)
#define PWM_LED2_NODE   DT_ALIAS(pwm_led2)
#define PWM_LED3_NODE   DT_ALIAS(pwm_led3)

/* Check if PWM LED nodes exist, fallback to regular LED nodes if not */
#if DT_NODE_EXISTS(PWM_LED0_NODE)
static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(PWM_LED0_NODE);
#else
#define PWM_LED0_NODE   DT_ALIAS(led0)
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

/* Array of PWM LED specifications */
static const struct pwm_dt_spec *pwm_leds[] = {
    &pwm_led0,
    &pwm_led1,
    &pwm_led2,
    &pwm_led3
};

#define NUM_LEDS ARRAY_SIZE(pwm_leds)

/**
 * @brief Fade LED in or out
 * @param led_spec PWM LED specification
 * @param fade_in true for fade in, false for fade out
 */
static void fade_led(const struct pwm_dt_spec *led_spec, bool fade_in)
{
    uint32_t pulse_width;
    
    for (int step = 0; step <= FADE_STEPS; step++) {
        if (fade_in) {
            pulse_width = (PWM_PERIOD_US * step) / FADE_STEPS;
        } else {
            pulse_width = (PWM_PERIOD_US * (FADE_STEPS - step)) / FADE_STEPS;
        }
        
        int ret = pwm_set_dt(led_spec, PWM_PERIOD_US, pulse_width);
        if (ret < 0) {
            printk("Error setting PWM: %d\n", ret);
            return;
        }
        
        k_msleep(FADE_STEP_MS);
    }
}

/**
 * @brief Set LED brightness (0-100%)
 * @param led_spec PWM LED specification
 * @param brightness Brightness percentage (0-100)
 */
static void set_led_brightness(const struct pwm_dt_spec *led_spec, uint8_t brightness)
{
    uint32_t pulse_width = (PWM_PERIOD_US * brightness) / 100;
    
    int ret = pwm_set_dt(led_spec, PWM_PERIOD_US, pulse_width);
    if (ret < 0) {
        printk("Error setting PWM brightness: %d\n", ret);
    }
}

/**
 * @brief Turn off all LEDs
 */
static void turn_off_all_leds(void)
{
    for (int i = 0; i < NUM_LEDS; i++) {
        pwm_set_dt(pwm_leds[i], PWM_PERIOD_US, 0);
    }
}

int main(void)
{
    int ret;
    
    printk("PWM Fading Blinky Sample for nRF5340 DK\n");
    
    /* Check if PWM devices are ready */
    for (int i = 0; i < NUM_LEDS; i++) {
        if (!device_is_ready(pwm_leds[i]->dev)) {
            printk("Error: PWM device %s is not ready\n", pwm_leds[i]->dev->name);
            return -1;
        }
        printk("PWM LED %d ready\n", i);
    }
    
    /* Turn off all LEDs initially */
    turn_off_all_leds();
    
    int current_led = 0;
    
    while (1) {
        printk("Fading LED %d\n", current_led);
        
        /* Fade in current LED */
        fade_led(pwm_leds[current_led], true);
        
        /* Keep LED on for a moment */
        k_msleep(200);
        
        /* Fade out current LED */
        fade_led(pwm_leds[current_led], false);
        
        /* Move to next LED */
        current_led = (current_led + 1) % NUM_LEDS;
        
        /* Small delay between LEDs */
        k_msleep(100);
    }
    
    return 0;
}