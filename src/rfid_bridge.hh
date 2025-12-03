// rfid_bridge.hh
#ifndef RFID_BRIDGE_HH
#define RFID_BRIDGE_HH

#include "game_types.h"  // For game TowerType

// External declaration of scanned tower (game TowerType)
extern TowerType scanned_tower;

// Initialize RFID hardware (wraps init_rfid from rfid.hh)
void rfid_setup();

// Sample all peripherals (RFID and joystick)
void sample_peripherals();

// Return a small code representing the scanned tag.
// 0 = no valid tag / nothing
// 1,2,3,4 = tower choices
int rfid_get_tower_code();

#endif // RFID_BRIDGE_HH