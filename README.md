
# RP2350B Tower Defense Game

A small-scale tower defense game implemented on the RP2350B microcontroller with LED matrix display and RFID-based tower selection.

---

## Project Overview
- **Microcontroller:** RP2350B  
- **Main Display:** 32x64 LED matrix  
- **Inputs:**  
  - RFID tags for tower selection  
  - Controller (buttons/joystick) for tower placement  
- **Secondary Display:** Small LCD for money/score display  
- **Game Mechanics:**  
  - Single-path map for enemies  
  - Three tower types  
  - Two enemy types  
  - Points/money displayed on LCD

---

## Peripherals & Wiring
1. **LED Matrix 32x64**  
   - Displays game map, towers, and enemies  
   - Use SPI/I2C for communication  

2. **RFID Reader**  
   - Reads tower selection tags  
   - Trigger tower selection logic  

3. **Controller**  
   - Input for tower placement  
   - Buttons or joystick for selecting placement square  

4. **LCD Display**  
   - Shows current money/points  
   - Updates on tower purchase or enemy defeat  

---

## Gameplay Flow (High Level)

```mermaid
flowchart TD
    A[Start / Power On RP2350B] --> B[Initialize Peripherals]
    B --> B1[LED Matrix 32x64]
    B --> B2[RFID Reader]
    B --> B3[Controller Inputs]
    B --> B4[LCD Display]
    
    B --> C[Display Main Menu / Game Info on LCD]
    
    C --> D[Wait for Tower Selection]
    D --> E{RFID Tag Scanned?}
    E -- Yes --> F[Determine Tower Type]
    F --> G[Show Selected Tower on LCD]
    G --> H[Wait for Placement Input from Controller]
    H --> I{Placement Valid?}
    I -- Yes --> J[Place Tower on Preset Square]
    I -- No --> H
    
    J --> K[Start Game Loop]
    
    K --> L[Move Enemies Along Path]
    L --> M[Check Tower Attack Range]
    M --> N[Attack Enemies if in Range]
    N --> O[Update Money / Points on LCD]
    O --> P{Enemies Remaining?}
    P -- Yes --> L
    P -- No --> Q[End Game / Show Results]
    Q --> R[Restart or Power Off]
    
    D -- No --> D
```

---

## Tower Placement & Attack Loop (Detailed)

```mermaid
flowchart TD
    A[Start Game Loop] --> B[Move Enemies Along Path]
    B --> C[For Each Tower]
    C --> D[Check if Enemy in Range]
    D -- Yes --> E[Attack Enemy]
    E --> F[Update Enemy HP]
    F --> G{Enemy HP <= 0?}
    G -- Yes --> H[Remove Enemy & Add Money/Points]
    G -- No --> I[Continue]
    D -- No --> I
    I --> J{All Towers Processed?}
    J -- No --> C
    J -- Yes --> K{All Enemies Processed?}
    K -- No --> B
    K -- Yes --> L{Wave Finished?}
    L -- No --> B
    L -- Yes --> M[Display Wave Result on LCD]
    M --> N{More Waves?}
    N -- Yes --> B
    N -- No --> O[End Game / Display Final Score]
```

---

## Coding Notes
- Use **preset tower placement squares** to simplify collision/placement logic  
- Keep **enemy types minimal** for initial implementation  
- Modularize code by peripheral:  
  - `led_matrix.c/h` → Drawing map and towers  
  - `rfid.c/h` → Tower selection logic  
  - `controller.c/h` → Tower placement input  
  - `lcd.c/h` → Money/points display  
  - `game.c/h` → Game logic, enemy movement, tower attacks  

---

## Future Enhancements
- Add more tower/enemy types  
- Multiple paths or map designs  
- Upgrade animations on LED matrix  
- Add sound effects using buzzer  
