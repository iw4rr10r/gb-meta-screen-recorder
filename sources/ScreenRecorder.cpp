/*
 * +---------------------------------------+
 * |    Gamebuino META Screen Recorder     | 
 * | © 2019 Stéphane Calderoni (aka Steph) |
 * |     https://gamebuino.com/@steph      |
 * +---------------------------------------+
 */

#include <Gamebuino-Meta.h>
#include "ScreenRecorder.h"

const uint16_t ScreenRecorder::SERIAL_DATA_RATE = 115200;
const Color    ScreenRecorder::FLASHING_COLOR   = gb.createColor(255, 0, 0);

uint8_t  ScreenRecorder::screenWidth  = 0;
uint8_t  ScreenRecorder::screenHeight = 0;
uint8_t  ScreenRecorder::sliceHeight  = 0;

uint32_t ScreenRecorder::timer        = 0;
bool     ScreenRecorder::initialized  = false;
bool     ScreenRecorder::recording    = false;
bool     ScreenRecorder::ready        = false;
bool     ScreenRecorder::lightsOn     = false;

void ScreenRecorder::init(uint8_t sliceHeight) {
    initialized = (sliceHeight ==  2) ||
                  (sliceHeight ==  4) ||
                  (sliceHeight ==  8) ||
                  (sliceHeight == 16) ||
                  (sliceHeight == 64);

    bool hd = sliceHeight != 64;
    ScreenRecorder::screenWidth  = hd ? 160 : 80;
    ScreenRecorder::screenHeight = hd ? 128 : 64;
    ScreenRecorder::sliceHeight  = sliceHeight;
}

void ScreenRecorder::tick() {
    if (initialized) {
        checkButtons();
        if (recording) {
            handleLEDs();
        }
    }
}

void ScreenRecorder::checkButtons() {
    if (recording) {
        if (gb.buttons.pressed(BUTTON_MENU)) {
            stopRecording();
        }
    } else {
        if (gb.buttons.pressed(BUTTON_MENU)) {
            timer = millis();
        } else if (gb.buttons.repeat(BUTTON_MENU, 0)) {
            if (millis() - timer > 1000) {
                startRecording();
             }
        }
    }
}

void ScreenRecorder::handleLEDs() {
    if (gb.frameCount % 10 == 0) {
        turnOnLEDs();
    } else if (lightsOn) {
        turnOffLEDs();
    }
}

void ScreenRecorder::turnOnLEDs() {
    gb.lights.fill(FLASHING_COLOR);
    lightsOn = true;
}

void ScreenRecorder::turnOffLEDs() {
    gb.lights.fill(BLACK);
    lightsOn = false;
}

void ScreenRecorder::startRecording() {
    recording = true;
    SerialUSB.print("start");
    SerialUSB.write(screenWidth);
    SerialUSB.write(screenHeight);
    SerialUSB.write(sliceHeight);
}

void ScreenRecorder::stopRecording() {
    SerialUSB.print("stop");
    recording = false;
    timer = millis();
    turnOffLEDs();
}

void ScreenRecorder::capture(uint16_t* buffer) {
    SerialUSB.write((const uint8_t*) buffer, 2 * screenWidth * sliceHeight);
}

void ScreenRecorder::monitor(uint16_t* buffer, uint16_t sliceIndex) {
    tick();

    if (recording) {
        if (!ready) {
            ready = sliceIndex == 0;
        }
        
        if (ready) {
            capture(buffer);
        }
    }
}

bool ScreenRecorder::isRecording() {
    return recording;
}