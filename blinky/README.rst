.. _blinky-sample:

PWM Fading Blinky
#################

Overview
********

The PWM Fading Blinky sample creates a smooth fading effect on LEDs using the :ref:`PWM API <pwm_api>`.

The source code shows how to:

#. Get PWM specifications from the :ref:`devicetree <dt-guide>` as :c:struct:`pwm_dt_spec`
#. Configure PWM channels for LED control
#. Create smooth fading effects by varying PWM duty cycle
#. Cycle through multiple LEDs with fading transitions

This sample demonstrates advanced LED control compared to the basic GPIO blinky sample.

.. _blinky-sample-requirements:

Requirements
************

Your board must:

#. Have LEDs connected via PWM-capable GPIO pins (these are called "User LEDs" on many of
   Zephyr's :ref:`boards`).
#. Have PWM controllers available and configured in devicetree.
#. Support for at least one PWM channel (up to 4 LEDs supported).

Building and Running
********************

Build and flash PWM Fading Blinky as follows, changing ``nrf5340dk_nrf5340_cpuapp`` for your board:

.. zephyr-app-commands::
   :zephyr-app: samples/basic/pwm_fading_blinky
   :board: nrf5340dk_nrf5340_cpuapp
   :goals: build flash
   :compact:

After flashing, the LEDs start to fade in and out in sequence. If a runtime error occurs, the sample
exits without printing to the console.

Build errors
************

You will see a build error at the source code line defining the ``struct
pwm_dt_spec`` variables if you try to build this sample for an unsupported
board.

The error typically indicates missing PWM controller configuration or unavailable PWM channels.

Adding board support
********************

To add support for your board, you need to configure PWM controllers and LED mappings in your devicetree:

.. code-block:: devicetree

   / {
       pwmleds {
           compatible = "pwm-leds";
           
           pwm_led0: pwm_led_0 {
               pwms = <&pwm0 0 PWM_MSEC(1) PWM_POLARITY_INVERTED>;
           };
       };
       
       aliases {
           pwm-led0 = &pwm_led0;
       };
   };

   &pwm0 {
       status = "okay";
       pinctrl-0 = <&pwm0_default>;
       pinctrl-names = "default";
   };

Tips:

- See :dtcompatible:`pwm-leds` for more information on defining PWM-based LEDs
  in devicetree.

- Configure pinctrl settings to map PWM channels to the correct GPIO pins.

- If you're not sure what to do, check the devicetree overlays for supported boards which
  use the same SoC as your target. See :ref:`get-devicetree-outputs` for details.

- See :zephyr_file:`include/zephyr/dt-bindings/pwm/pwm.h` for PWM-specific bindings.

- If the LEDs are built in to your board hardware, the PWM configuration should be defined in
  your :ref:`BOARD.dts file <devicetree-in-out-files>`. Otherwise, you can
  define one in a :ref:`devicetree overlay <set-devicetree-overlays>`.
