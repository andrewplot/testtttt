#ifndef TOWER_HH
#define TOWER_HH

// Hardware tower type enum (used by RFID tags)
// Renamed to avoid conflict with game TowerType
enum HardwareTowerType {
    MACHINE_GUN,
    CANNON,
    SNIPER,
    RADAR,
    BLANK,

    TOWER_TYPE_COUNT
};


class HardwareTower {
    public:
        HardwareTowerType type;
        int x_pos;
        int y_pos;

};

#endif //TOWER_HH