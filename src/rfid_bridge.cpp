// rfid_bridge.cpp – only hardware TowerType lives here
#include "rfid_bridge.hh"
#include "rfid.hh"      // includes tower.hh and declares init_rfid(), sample_rfid()
#include <stdio.h>
#include "joystick.hh"

void rfid_setup() {
    init_rfid();       // from rfid.hh
}

TowerType scanned_tower = BLANK;
char *directions[] = {"Left", "Right", "Up", "Down", "Center" };
bool last_select = false;
JoystickDirection last_x = center;
JoystickDirection last_y = center;

void sample_peripherals() {
    if (rfid_flag) {
        rfid_flag = false;
        scanned_tower = sample_rfid();
        printf("Scanned Tower: %d\n", scanned_tower);
    }

    if (joystick_flag) {
        joystick_flag = false;
        JoystickDirection x = sample_js_x();
        JoystickDirection y = sample_js_y();
        bool select = sample_js_select();

        if (x != last_x) {
            printf("Joystick X: %s\n", directions[x]);
            last_x = x;
        } 
        if (y != last_y) {
            printf("Joystick Y: %s\n", directions[y]);
            last_y = y;
        }
        if (select != last_select) {
            printf("Joystick Sel: %s\n", select ? "true" : "false");
            last_select = select;
        }
    }
}


int rfid_get_tower_code() {
    // TowerType here is the *hardware* TowerType from tower.hh
    TowerType hw_type = sample_rfid();

    switch (hw_type) {
        case MACHINE_GUN:  return 1;
        case CANNON:       return 2;
        case SNIPER:       return 3;
        case RADAR:        return 4;
        default:           return 0;   // no valid tag
    }
}
