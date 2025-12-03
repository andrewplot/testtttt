"""
Main entry point for Tower Defense Game
"""

import pygame
import os
from led_matrix import LEDMatrixSimulator
from map_data import MapLoader, create_sample_maps
from game import Game
from tower import Tower


def setup_maps():
    """Create sample maps if they don't exist"""
    if not os.path.exists("maps"):
        os.makedirs("maps", exist_ok=True)
        maps = create_sample_maps()
        for i, map_data in enumerate(maps, 1):
            MapLoader.save_json(f"maps/map{i}.json", map_data)


def main():
    """Main game loop"""
    
    # Setup
    setup_maps()
    map_data = MapLoader.load_json("maps/forest.json")
    matrix = LEDMatrixSimulator(64, 32, pixel_size=15)
    game = Game(matrix, map_data)
    
    # Game loop
    clock = pygame.time.Clock()
    running = True
    
    print("\n=== TOWER DEFENSE CONTROLS ===")
    print("\nTOWERS:")
    print("  1 = Machine Gun ($30) - Fast fire, general purpose")
    print("  2 = Cannon ($50) - Splash damage, short range")
    print("  3 = Sniper ($65) - High damage, sees invisible")
    print("  4 = Radar ($40) - Reveals invisible enemies")
    print("\nENEMIES:")
    print("  Q = Scout (3 HP, normal speed)")
    print("  W = Tank (10 HP, slow, 2 damage)")
    print("  E = Splitter (2 HP, fast, splits into 2 scouts)")
    print("  R = Ghost (4 HP, invisible)")
    print("\nABILITIES:")
    print("  A = Apache Strike ($50)")
    print("  B = Bomber Run ($75)")
    print("\nBANNER PLANE:")
    print("  N = Spawn banner plane (for testing)")
    print("      Format: 'WAVE X' where X is 0-9")
    print("\nOTHER:")
    print("  T = Toggle tower ranges")
    print("  Click = Place selected tower")
    print("  ESC = Quit")
    print("\n===============================\n")
    
    # For banner testing
    current_test_wave = 1
    
    while running:
        dt = clock.get_time() / 1000.0
        
        # Handle events
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    running = False
                
                # Tower selection
                elif event.key == pygame.K_1:
                    game.selected_tower_type = "machine_gun"
                    print("Selected: Machine Gun ($30)")
                elif event.key == pygame.K_2:
                    game.selected_tower_type = "cannon"
                    print("Selected: Cannon ($50)")
                elif event.key == pygame.K_3:
                    game.selected_tower_type = "sniper"
                    print("Selected: Sniper ($65)")
                elif event.key == pygame.K_4:
                    game.selected_tower_type = "radar"
                    print("Selected: Radar ($40)")
                
                # Enemy spawning
                elif event.key == pygame.K_q:
                    game.spawn_enemy("scout")
                    print("Spawned: Scout")
                elif event.key == pygame.K_w:
                    game.spawn_enemy("tank")
                    print("Spawned: Tank")
                elif event.key == pygame.K_e:
                    game.spawn_enemy("splitter")
                    print("Spawned: Splitter")
                elif event.key == pygame.K_r:
                    game.spawn_enemy("ghost")
                    print("Spawned: Ghost (invisible)")
                
                # Abilities
                elif event.key == pygame.K_a:
                    if game.activate_ability("apache"):
                        print("Apache Strike activated!")
                    else:
                        print("Can't activate Apache (cost: $50, cooldown or insufficient funds)")
                elif event.key == pygame.K_b:
                    if game.activate_ability("bomber"):
                        print("Bomber Run activated!")
                    else:
                        print("Can't activate Bomber (cost: $75, cooldown or insufficient funds)")
                
                # Banner plane (testing)
                elif event.key == pygame.K_n:
                    game.spawn_banner_plane(current_test_wave)
                    print(f"Banner plane spawned: WAVE {current_test_wave}")
                    current_test_wave += 1
                    if current_test_wave > 9:
                        current_test_wave = 1
                
                # Toggle ranges
                elif event.key == pygame.K_t:
                    game.show_tower_ranges = not game.show_tower_ranges
                    print(f"Tower ranges: {'ON' if game.show_tower_ranges else 'OFF'}")
            
            elif event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
                pos = event.pos
                matrix_x = pos[0] // matrix.pixel_size
                matrix_y = pos[1] // matrix.pixel_size
                if game.handle_click(matrix_x, matrix_y):
                    print(f"Placed {game.selected_tower_type} at ({matrix_x}, {matrix_y})")
                else:
                    print("Cannot place tower there")
        
        # Update and draw
        game.update(dt)
        game.draw()
        
        # Display game info in window title
        apache_cd = game.ability_manager.get_cooldown_remaining("apache", game.game_time)
        bomber_cd = game.ability_manager.get_cooldown_remaining("bomber", game.game_time)
        
        tower_type_name = game.selected_tower_type if game.selected_tower_type else "None"
        
        banner_status = "FLYING" if (game.banner_plane and game.banner_plane.active) else "---"
        
        pygame.display.set_caption(
            f"TD | ${game.money} | Lives:{game.lives} | Score:{game.score} | "
            f"Selected:{tower_type_name} | Apache:{apache_cd:.1f}s | Bomber:{bomber_cd:.1f}s | Banner:{banner_status}"
        )
        
        if not matrix.update_display():
            running = False
        
        clock.tick(30)
    
    matrix.close()


if __name__ == "__main__":
    main()