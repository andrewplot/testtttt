"""
Game logic and state management for Tower Defense
"""

from map_render import MapRenderer
from enemy import Enemy
from tower import Tower
from ability import AbilityManager
from banner_plane import BannerPlane
import math


class Game:
    """Manages game state and logic"""
    
    def __init__(self, matrix, map_data):
        self.matrix = matrix
        self.map_data = map_data
        self.renderer = MapRenderer(matrix, map_data)
        
        # Game state
        self.enemies = []
        self.towers = []
        self.money = 200  # Starting money
        self.lives = 20  # Player health
        self.score = 0
        self.game_time = 0
        self.game_over = False
        
        # Tower placement state
        self.selected_tower_type = None
        self.show_tower_ranges = False
        
        # Ability system
        self.ability_manager = AbilityManager()
        
        # Banner plane (for wave announcements)
        self.banner_plane = None
        
        # Spawned enemies that need to be added (for splitters)
        self.pending_spawns = []
    
    def spawn_enemy(self, enemy_type):
        """Spawn a new enemy"""
        enemy = Enemy(enemy_type, self.map_data.path, self.game_time)
        self.enemies.append(enemy)
    
    def spawn_enemy_at_position(self, enemy_type, x, y):
        """Spawn an enemy at a specific position (for splitters)"""
        enemy = Enemy(enemy_type, self.map_data.path, self.game_time)
        enemy.x = x
        enemy.y = y
        # Find closest path point
        enemy.path_index = self._find_closest_path_index(x, y)
        enemy.path_progress = self._calculate_path_progress(enemy.path_index, x, y)
        self.pending_spawns.append(enemy)
    
    def _find_closest_path_index(self, x, y):
        """Find the closest path waypoint index"""
        min_dist = float('inf')
        closest_idx = 0
        for i, (px, py) in enumerate(self.map_data.path):
            dist = math.sqrt((px - x)**2 + (py - y)**2)
            if dist < min_dist:
                min_dist = dist
                closest_idx = i
        return max(0, closest_idx - 1)  # Return previous waypoint
    
    def _calculate_path_progress(self, path_index, x, y):
        """Calculate total path progress for an enemy at a position"""
        progress = 0
        for i in range(path_index):
            if i + 1 < len(self.map_data.path):
                x1, y1 = self.map_data.path[i]
                x2, y2 = self.map_data.path[i + 1]
                progress += math.sqrt((x2 - x1)**2 + (y2 - y1)**2)
        
        # Add distance from last waypoint to current position
        if path_index < len(self.map_data.path):
            wx, wy = self.map_data.path[path_index]
            progress += math.sqrt((x - wx)**2 + (y - wy)**2)
        
        return progress
    
    def spawn_banner_plane(self, wave_number):
        """
        Spawn a banner plane to announce a wave
        
        Args:
            wave_number: Wave number to display on banner
        """
        # Spawn from RIGHT side (off-screen) since plane flies right-to-left
        self.banner_plane = BannerPlane(70, 16, self.game_time, wave_number)
        return self.banner_plane
    
    def place_tower(self, tower_type, x, y):
        """
        Attempt to place a tower at given coordinates
        
        Returns:
            bool: True if tower was placed successfully
        """
        # Check if we have enough money
        cost = Tower.get_cost(tower_type)
        if self.money < cost:
            return False
        
        # Find nearest tower slot
        slot = self._find_nearest_tower_slot(x, y, max_distance=5)
        if not slot:
            return False
        
        slot_x, slot_y = slot
        
        # Check if slot is already occupied
        for tower in self.towers:
            if tower.x == slot_x and tower.y == slot_y:
                return False
        
        # Place the tower
        tower = Tower(tower_type, slot_x, slot_y)
        self.towers.append(tower)
        self.money -= cost
        return True
    
    def _find_nearest_tower_slot(self, x, y, max_distance=10):
        """Find the nearest tower slot to given coordinates"""
        nearest_slot = None
        nearest_distance = max_distance + 1
        
        for slot_x, slot_y in self.map_data.tower_slots:
            distance = ((slot_x - x) ** 2 + (slot_y - y) ** 2) ** 0.5
            if distance < nearest_distance:
                nearest_distance = distance
                nearest_slot = (slot_x, slot_y)
        
        return nearest_slot if nearest_distance <= max_distance else None
    
    def activate_ability(self, ability_type):
        """
        Activate a special ability
        
        Args:
            ability_type: Type of ability ("apache", "bomber")
            
        Returns:
            bool: True if activated successfully
        """
        if not self.ability_manager.can_activate(ability_type, self.money, self.game_time):
            return False
        
        # Deduct cost
        cost = AbilityManager.ABILITY_TYPES[ability_type]["cost"]
        self.money -= cost
        
        # Calculate average path Y coordinate for bomber targeting
        path_y = sum(y for x, y in self.map_data.path) / len(self.map_data.path) if self.map_data.path else 16
        
        # Activate
        self.ability_manager.activate(ability_type, self.game_time, self.matrix.width, path_y)
        return True
    
    def update_radar_detection(self):
        """Update which enemies are revealed by radar towers"""
        # Reset all revealed flags
        for enemy in self.enemies:
            enemy.revealed = False
        
        # Check each radar tower
        for tower in self.towers:
            if tower.is_radar:
                # Reveal enemies in range
                for enemy in self.enemies:
                    if enemy.invisible:
                        dx = enemy.x - tower.x
                        dy = enemy.y - tower.y
                        distance = math.sqrt(dx * dx + dy * dy)
                        if distance <= tower.range:
                            enemy.revealed = True
    
    def update(self, dt):
        """Update game state"""
        if self.game_over:
            return
        
        self.game_time += dt
        
        # Update banner plane
        if self.banner_plane and self.banner_plane.active:
            self.banner_plane.update(dt)
        
        # Update radar detection
        self.update_radar_detection()
        
        # Update abilities
        self.ability_manager.update(dt, self.enemies)
        
        # Update towers
        for tower in self.towers:
            tower.update(dt, self.enemies)
        
        # Update enemies
        for enemy in self.enemies[:]:
            reached_end = enemy.update(dt)
            
            if reached_end:
                self.enemies.remove(enemy)
                self.lives -= enemy.damage
                if self.lives <= 0:
                    self.lives = 0
                    self.game_over = True
            
            elif not enemy.alive:
                self.enemies.remove(enemy)
                self.money += enemy.reward
                self.score += enemy.reward
                
                # Handle splitting enemies
                if enemy.splits_on_death:
                    for i in range(enemy.split_count):
                        # Spawn splits slightly offset from death position
                        offset_x = (i - enemy.split_count / 2) * 2
                        self.spawn_enemy_at_position(enemy.split_type, enemy.x + offset_x, enemy.y)
        
        # Add pending spawns from splitters
        if self.pending_spawns:
            self.enemies.extend(self.pending_spawns)
            self.pending_spawns = []
    
    def draw(self):
        """Draw everything"""
        self.matrix.clear()
        self.renderer.draw_all()
        
        # Draw tower ranges if enabled
        if self.show_tower_ranges:
            for tower in self.towers:
                tower.draw_range(self.matrix)
        
        # Draw towers and enemies
        for tower in self.towers:
            tower.draw(self.matrix)
        for enemy in self.enemies:
            enemy.draw(self.matrix)
        
        # Draw abilities (on top layer)
        self.ability_manager.draw(self.matrix)
        
        # Draw banner plane (on top of everything)
        if self.banner_plane and self.banner_plane.active:
            self.banner_plane.draw(self.matrix)
    
    def handle_click(self, matrix_x, matrix_y):
        """Handle click for tower placement"""
        if self.selected_tower_type:
            return self.place_tower(self.selected_tower_type, matrix_x, matrix_y)
        return False