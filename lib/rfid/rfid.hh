#ifndef RFID_HH
#define RFID_HH

#include "tower.hh"

extern volatile bool rfid_flag;

/**
 * @brief initialize rfid scanner and start timer
 */
void init_rfid();

/**
 * @brief samples rfid one time returns scanned_tower
 * 
 * @return TowerType of rfid scanned
 */
TowerType sample_rfid();


#endif // RFID_HH