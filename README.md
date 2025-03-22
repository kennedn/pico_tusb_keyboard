# Pico TinyUSB Keyboard

USB HID Keyboard for Pico RP2040 with TinyUSB.

This is an adapted copy of the hid_composite example from [TinyUSB](https://github.com/hathach/tinyusb/tree/master/examples/device/hid_composite) showing how to build with TinyUSB when using the Raspberry Pi Pico SDK.

## Keyboard

In `main.c` a USB HID Keyboard report is created from pin inputs on the Pico.

Connect pico gpio pins to usb hid keyboard keys:

```c
#define NUM_PINS 4
// Note: Tiny USB HID_KEY_* mappings assume a US keyboard layout. I have a UK keyboard layout. Hence the discrepancy between comment and code.
const PinKey pin_keys[NUM_PINS] = {
    {12, KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_GRAVE},                          // Ctrl + Shift + `
    {13, KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_APOSTROPHE},                     // Ctrl + Shift + #
    {14, KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_BACKSLASH},                      // Ctrl + Shift + '
    {15, KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_LEFTALT, HID_KEY_V},  // Ctrl + Shift + Alt + v
};
```

Make sure `NUM_PINS` is equal to the amount of pin-key combinations specified in `pin_keys`. In this case, there are 4 connected pins. Note that the pico has 26 usable gpio pins.

> NOTE: If the Pico LED remains on, it means that the Pico has not been able to establish a USB connection with your device.

## Hardware

The input pins use the internal pull up resistor on the pico. They read `1` when **not** pressed. Connect a pin to ground to generate a key press for that pin.

```
            button
              __
    pin -----+  +----- ground
```

The pins are polled once every 10ms.

## Building

Clone repository and cd
```shell
git clone https://github.com/kennedn/pico_tusb_keyboard.git
cd pico_tusb_keyboard
```

Make build folder and cd
```shell
mkdir build
cd build
```

Run cmake
```shell
cmake -DCMAKE_BUILD_TYPE=Release ..
```

Compile the application
```shell
cd src
make -j$(nproc)
```
