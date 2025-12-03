import math


class Projectile:
    """Represents a projectile fired by a tower"""
    
    def __init__(self, x, y, target, damage, speed, color, splash_radius=0):
        """
        Initialize a projectile
        
        Args:
            x, y: Starting position
            target: Enemy object to track
            damage: Damage to deal on hit
            speed: Projectile speed (pixels per second), 0 for instant
            color: RGB tuple for projectile color
            splash_radius: Radius for splash damage (0 = no splash)
        """
        self.x = x
        self.y = y
        self.target = target
        self.damage = damage
        self.speed = speed
        self.color = color
        self.splash_radius = splash_radius
        self.active = True
    
    def update(self, dt, all_enemies):
        """
        Move projectile toward target
        
        Args:
            dt: Delta time in seconds
            all_enemies: List of all enemies (for splash damage)
            
        Returns:
            bool: True if hit target or target died, False if still traveling
        """
        if not self.target.alive:
            self.active = False
            return True
        
        # Instant hit (sniper)
        if self.speed == 0:
            self.hit_target(all_enemies)
            return True
        
        # Calculate direction to target
        dx = self.target.x - self.x
        dy = self.target.y - self.y
        distance = math.sqrt(dx * dx + dy * dy)
        
        if distance == 0:
            self.hit_target(all_enemies)
            return True
        
        # Calculate movement distance this frame
        move_distance = self.speed * dt
        
        if distance <= move_distance:
            # Hit target
            self.hit_target(all_enemies)
            return True
        else:
            # Move toward target
            self.x += (dx / distance) * move_distance
            self.y += (dy / distance) * move_distance
            return False
    
    def hit_target(self, all_enemies):
        """Apply damage to target and splash if applicable"""
        # Direct damage
        self.target.take_damage(self.damage)
        
        # Splash damage
        if self.splash_radius > 0:
            for enemy in all_enemies:
                if not enemy.alive or enemy == self.target:
                    continue
                
                # Check if in splash radius
                dx = enemy.x - self.target.x
                dy = enemy.y - self.target.y
                distance = math.sqrt(dx * dx + dy * dy)
                
                if distance <= self.splash_radius:
                    # Splash does 50% damage
                    enemy.take_damage(self.damage * 0.5)
        
        self.active = False
    
    def draw(self, matrix):
        """Draw the projectile on the LED matrix"""
        x = int(self.x)
        y = int(self.y)
        matrix.set_pixel(x, y, *self.color)


class Tower:
    """Base tower class for tower defense game"""
    
    # Tower type definitions
    TOWER_TYPES = {
        "machine_gun": {
            "name": "Machine Gun",
            "cost": 30,
            "damage": 1,
            "range": 14,
            "fire_rate": 0.5,
            "projectile_speed": 60,
            "color": (100, 150, 50),  # Green/brown
            "projectile_color": (200, 200, 200),  # Gray
            "size": 3,
            "can_see_invisible": False,
            "splash_radius": 0,
            "description": "Fast firing, general purpose"
        },
        "cannon": {
            "name": "Cannon",
            "cost": 50,
            "damage": 2,
            "range": 10,
            "fire_rate": 1.5,
            "projectile_speed": 40,
            "color": (80, 80, 80),  # Dark gray
            "projectile_color": (255, 200, 0),  # Yellow shell
            "size": 3,
            "can_see_invisible": False,
            "splash_radius": 3,
            "description": "Splash damage, short range"
        },
        "sniper": {
            "name": "Sniper",
            "cost": 65,
            "damage": 4,
            "range": 26,
            "fire_rate": 2.0,
            "projectile_speed": 0,  # Instant hit
            "color": (40, 40, 40),  # Black
            "projectile_color": (255, 255, 100),  # Bright tracer
            "size": 3,
            "can_see_invisible": True,
            "splash_radius": 0,
            "description": "High damage, long range, sees invisible"
        },
        "radar": {
            "name": "Radar",
            "cost": 40,
            "damage": 0,
            "range": 22,
            "fire_rate": 999,  # Never fires
            "projectile_speed": 0,
            "color": (50, 150, 200),  # Blue
            "projectile_color": (0, 0, 0),  # Not used
            "size": 3,
            "can_see_invisible": True,
            "splash_radius": 0,
            "is_radar": True,
            "radar_sweep_speed": 2.0,  # Radians per second
            "description": "Reveals invisible enemies"
        }
    }
    
    def __init__(self, tower_type, x, y):
        """
        Initialize a tower
        
        Args:
            tower_type: Type of tower
            x, y: Position on the map
        """
        self.tower_type = tower_type
        self.x = x
        self.y = y
        
        # Load stats from tower type
        stats = self.TOWER_TYPES[tower_type]
        self.name = stats["name"]
        self.damage = stats["damage"]
        self.range = stats["range"]
        self.fire_rate = stats["fire_rate"]
        self.projectile_speed = stats["projectile_speed"]
        self.color = stats["color"]
        self.projectile_color = stats["projectile_color"]
        self.size = stats["size"]
        self.can_see_invisible = stats["can_see_invisible"]
        self.splash_radius = stats["splash_radius"]
        
        # Radar-specific
        self.is_radar = stats.get("is_radar", False)
        self.radar_angle = 0  # Current sweep angle
        self.radar_sweep_speed = stats.get("radar_sweep_speed", 2.0)
        
        # State tracking
        self.time_since_last_shot = 0
        self.target = None
        self.projectiles = []
    
    def update(self, dt, enemies):
        """
        Update tower logic (targeting and shooting)
        
        Args:
            dt: Delta time in seconds
            enemies: List of Enemy objects
        """
        # Radar towers don't shoot, but they sweep
        if self.is_radar:
            self.radar_angle += self.radar_sweep_speed * dt
            if self.radar_angle > 2 * math.pi:
                self.radar_angle -= 2 * math.pi
            
            # Reveal enemies that the sweep passes over
            self._update_radar_sweep(enemies)
            return
        
        self.time_since_last_shot += dt
        
        # Update existing projectiles
        for projectile in self.projectiles[:]:
            if projectile.update(dt, enemies):
                self.projectiles.remove(projectile)
        
        # Check if we can shoot
        if self.time_since_last_shot >= self.fire_rate:
            target = self.find_target(enemies)
            if target:
                self.shoot(target)
                self.time_since_last_shot = 0
    
    def _update_radar_sweep(self, enemies):
        """Update which enemies are revealed by radar sweep"""
        sweep_width = 0.3  # Radians of sweep arc width
        
        for enemy in enemies:
            if not enemy.invisible:
                continue
            
            # Check if enemy is in range
            dx = enemy.x - self.x
            dy = enemy.y - self.y
            distance = math.sqrt(dx * dx + dy * dy)
            
            if distance <= self.range:
                # Calculate angle to enemy
                angle_to_enemy = math.atan2(dy, dx)
                
                # Normalize angles to 0-2π
                if angle_to_enemy < 0:
                    angle_to_enemy += 2 * math.pi
                
                # Check if enemy is within sweep arc
                angle_diff = abs(angle_to_enemy - self.radar_angle)
                if angle_diff > math.pi:
                    angle_diff = 2 * math.pi - angle_diff
                
                if angle_diff < sweep_width:
                    enemy.revealed = True
                    enemy.reveal_timer = 0.5  # Stay revealed for 0.5 seconds after sweep passes
    
    def find_target(self, enemies):
        """
        Find the best enemy to target
        Uses "first" targeting - enemy furthest along the path
        
        Args:
            enemies: List of Enemy objects
            
        Returns:
            Enemy object or None
        """
        best_target = None
        best_progress = -1
        
        for enemy in enemies:
            if not enemy.alive:
                continue
            
            # Check if enemy is invisible and we can't see it
            if enemy.invisible and not self.can_see_invisible:
                continue
            
            # Check if enemy is in range
            distance = self.distance_to(enemy)
            if distance <= self.range:
                # Prioritize enemies furthest along the path
                if enemy.path_progress > best_progress:
                    best_progress = enemy.path_progress
                    best_target = enemy
        
        return best_target
    
    def distance_to(self, enemy):
        """Calculate distance to an enemy"""
        dx = enemy.x - self.x
        dy = enemy.y - self.y
        return math.sqrt(dx * dx + dy * dy)
    
    def shoot(self, target):
        """
        Fire projectile at target
        
        Args:
            target: Enemy object to shoot at
        """
        projectile = Projectile(
            self.x, self.y,
            target,
            self.damage,
            self.projectile_speed,
            self.projectile_color,
            self.splash_radius
        )
        self.projectiles.append(projectile)
    
    def draw(self, matrix):
        """Draw the tower on the LED matrix"""
        # Radar-specific drawing (draw range circle FIRST, under everything)
        if self.is_radar:
            # Draw single translucent green circle outline
            radius = int(self.range)
            cx = int(self.x)
            cy = int(self.y)
            
            # Dim translucent green for the border
            circle_color = (0, 80, 0)  # Darker, more subtle green
            
            x = radius
            y = 0
            err = 0
            
            while x >= y:
                # Draw 8 symmetric points (single pixel outline)
                points = [
                    (cx + x, cy + y), (cx + y, cy + x),
                    (cx - y, cy + x), (cx - x, cy + y),
                    (cx - x, cy - y), (cx - y, cy - x),
                    (cx + y, cy - x), (cx + x, cy - y)
                ]
                
                for px, py in points:
                    if 0 <= px < matrix.width and 0 <= py < matrix.height:
                        matrix.set_pixel(px, py, *circle_color)
                
                if err <= 0:
                    y += 1
                    err += 2 * y + 1
                
                if err > 0:
                    x -= 1
                    err -= 2 * x + 1
        
        # Draw tower base
        half_size = self.size // 2
        matrix.fill_rect(
            int(self.x) - half_size,
            int(self.y) - half_size,
            self.size,
            self.size,
            *self.color
        )
        
        # Radar rotating sweep line (extends to full range)
        if self.is_radar:
            # Draw sweep line extending to the edge of range (subtract 1 to stay inside circle)
            sweep_length = int(self.range) - 1
            end_x = int(self.x + math.cos(self.radar_angle) * sweep_length)
            end_y = int(self.y + math.sin(self.radar_angle) * sweep_length)
            # Translucent green for sweep line
            matrix.draw_line(int(self.x), int(self.y), end_x, end_y, 0, 120, 0)
        
        # Draw projectiles
        for projectile in self.projectiles:
            projectile.draw(matrix)
        
        # Draw sniper tracer line
        if self.tower_type == "sniper" and self.target and self.target.alive:
            if self.time_since_last_shot < 0.1:  # Flash for 100ms
                matrix.draw_line(
                    int(self.x), int(self.y),
                    int(self.target.x), int(self.target.y),
                    *self.projectile_color
                )
    
    def draw_range(self, matrix):
        """
        Draw tower range indicator (for UI/debugging)
        """
        range_color = (100, 100, 100)  # Gray
        
        # Draw circle using midpoint circle algorithm
        radius = int(self.range)
        cx = int(self.x)
        cy = int(self.y)
        
        x = radius
        y = 0
        err = 0
        
        while x >= y:
            # Draw 8 symmetric points
            points = [
                (cx + x, cy + y), (cx + y, cy + x),
                (cx - y, cy + x), (cx - x, cy + y),
                (cx - x, cy - y), (cx - y, cy - x),
                (cx + y, cy - x), (cx + x, cy - y)
            ]
            
            for px, py in points:
                if 0 <= px < matrix.width and 0 <= py < matrix.height:
                    matrix.set_pixel(px, py, *range_color)
            
            if err <= 0:
                y += 1
                err += 2 * y + 1
            
            if err > 0:
                x -= 1
                err -= 2 * x + 1
    
    @classmethod
    def get_cost(cls, tower_type):
        """Get the cost of a tower type"""
        return cls.TOWER_TYPES[tower_type]["cost"]
    
    @classmethod
    def get_info(cls, tower_type):
        """Get information about a tower type"""
        return cls.TOWER_TYPES[tower_type]