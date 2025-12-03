// rfid_bridge.cpp – Bridge between hardware RFID and game tower types
#include "game_types.h" // includes game TowerType with TOWER_BLANK - INCLUDE FIRST
#include "rfid_bridge.hh"
#include "rfid.hh"      // includes tower.hh (hardware HardwareTowerType with BLANK)
#include <stdio.h>
#include "joystick.hh"

void rfid_setup() {
    init_rfid();       // from rfid.hh
}

// This is the GAME TowerType (TOWER_MACHINE_GUN, etc.)
TowerType scanned_tower = TOWER_BLANK;

const char *directions[] = {"Left", "Right", "Up", "Down", "Center" };
bool last_select = false;
JoystickDirection last_x = center;
JoystickDirection last_y = center;

// Convert hardware tower enum value to game TowerType
static TowerType convert_hardware_to_game_tower(HardwareTowerType hw_type) {
    // Hardware enum values (from lib/tower/tower.hh):
    // MACHINE_GUN = 0, CANNON = 1, RADAR = 2, SNIPER = 3, BLANK = 4
    
    // Game enum values (from game_types.h):
    // TOWER_MACHINE_GUN = 0, TOWER_CANNON = 1, TOWER_SNIPER = 2, TOWER_RADAR = 3, TOWER_BLANK = 4
    
    switch (hw_type) {
        case MACHINE_GUN: return TOWER_MACHINE_GUN;
        case CANNON:      return TOWER_CANNON;
        case SNIPER:      return TOWER_SNIPER;
        case RADAR:       return TOWER_RADAR;
        case BLANK:       
        default:          return TOWER_BLANK;
    }
}

void sample_peripherals() {
    if (rfid_flag) {
        rfid_flag = false;
        
        // sample_rfid() returns hardware HardwareTowerType (from lib/tower/tower.hh)
        HardwareTowerType hw_tower = sample_rfid();
        
        // Convert to game TowerType
        scanned_tower = convert_hardware_to_game_tower(hw_tower);
        
        if (scanned_tower != TOWER_BLANK) {
            printf("RFID: Hardware tower %d -> Game tower %d\n", hw_tower, scanned_tower);
        }
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
    // This function can stay as-is or be updated to use the new scanned_tower
    // For now, keeping it for compatibility
    HardwareTowerType hw_tower = sample_rfid();
    
    switch (hw_tower) {
        case MACHINE_GUN: return 1;
        case CANNON:      return 2;
        case SNIPER:      return 3;
        case RADAR:       return 4;
        case BLANK:       
        default:          return 0;
    }
}