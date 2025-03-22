/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "pico/stdlib.h"

#include "bsp/board.h"
#include "tusb.h"

#include "usb_descriptors.h"


typedef struct
{
    uint8_t pin;      // Pico pin number
    uint8_t modifier; // HID_MODIFIER_*
    uint8_t key;      // HID_KEY_*
} PinKey;

#define NUM_PINS 4
// Note: Tiny USB HID_KEY_* mappings assume a US keyboard layout. I have a UK keyboard layout. Hence the discrepancy between comment and code.
const PinKey pin_keys[NUM_PINS] = {
    {12, KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_GRAVE},                          // Ctrl + Shift + `
    {13, KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_APOSTROPHE},                     // Ctrl + Shift + #
    {14, KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_BACKSLASH},                      // Ctrl + Shift + '
    {15, KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_LEFTALT, HID_KEY_V},  // Ctrl + Shift + Alt + v
};

// Invoked when received GET_REPORT control request, Stub
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    return 0;
}

// Invoked when received SET_REPORT control request, Stub
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    return;
}

PinKey* get_keypress() {
    for (size_t i = 0; i < NUM_PINS; i++) {
        int pin_state = gpio_get(pin_keys[i].pin);
        if (pin_state == 0) {
            return (PinKey*) &pin_keys[i];
        }
    }

    return NULL;
}

static void send_hid_report(PinKey *pin_key) {
    // skip if hid is not ready yet
    if (!tud_hid_ready())
    {
        return;
    }

    // avoid sending multiple zero reports
    static bool send_empty = false;

    if (pin_key) {
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, pin_key->modifier, (uint8_t[6]){pin_key->key});
        send_empty = true;
    }
    else if (send_empty){
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        send_empty = false;
    }
}

// Every 10ms, we poll the pins and send a report
void hid_task(void) {
    // Poll every 10ms
    const uint32_t interval_ms = 10;
    static uint32_t sw_timer = 0;

    if (board_millis() - sw_timer < interval_ms) {
        return; // not enough time has passed
    }

    sw_timer = board_millis();

    // Check for keys pressed
    PinKey *pin_key = get_keypress();

    // Remote wakeup
    if (tud_suspended() && pin_key) {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    }
    else {
        // send a report
        send_hid_report(pin_key);
    }
}

int main(void) {
    board_init();
    tusb_init();

    for (size_t i = 0; i < NUM_PINS; i++) {
        gpio_init(pin_keys[i].pin);
        gpio_pull_up(pin_keys[i].pin);
        gpio_set_dir(pin_keys[i].pin, GPIO_IN);
    }

    while (1) {
        tud_task(); // tinyusb device task
        hid_task(); // keyboard implementation
    }

    return 0;
}