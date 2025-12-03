"""
Special Abilities for Tower Defense Game
Flying vehicle abilities that can be activated
"""

import math


class ApacheStrike:
    """
    Apache helicopter that flies across the screen and attacks enemies
    """
    
    def __init__(self, start_x, start_y, game_time):
        """
        Initialize the Apache Strike
        
        Args:
            start_x, start_y: Starting position (usually off-screen right)
            game_time: Current game time
        """
        self.x = start_x
        self.y = start_y
        self.spawn_time = game_time
        self.game_time = game_time  # Track for animation
        
        # Movement
        self.speed = 8  # Pixels per second (slow and menacing)
        self.target_x = -5  # Fly off screen to the left
        
        # Combat stats
        self.damage = 2
        self.fire_rate = 0.3  # Seconds between shots
        self.range = 20  # Vertical range to attack enemies
        self.time_since_last_shot = 0
        
        # Visual
        self.color = (50, 150, 50)  # Military green
        self.rotor_color = (100, 100, 100)  # Gray rotors
        self.size = 3
        
        # State
        self.active = True
        self.projectiles = []
    
    def update(self, dt, enemies):
        """
        Update helicopter movement and combat
        
        Args:
            dt: Delta time in seconds
            enemies: List of Enemy objects
        """
        # Update game time for animation (accumulate time)
        self.game_time += dt
        
        # Move left across screen
        self.x -= self.speed * dt
        
        # Check if off-screen
        if self.x < self.target_x:
            self.active = False
            return
        
        # Update shooting
        self.time_since_last_shot += dt
        
        # Update existing projectiles
        for projectile in self.projectiles[:]:
            if projectile.update(dt):
                self.projectiles.remove(projectile)
        
        # Try to shoot
        if self.time_since_last_shot >= self.fire_rate:
            target = self.find_target(enemies)
            if target:
                self.shoot(target)
                self.time_since_last_shot = 0
    
    def find_target(self, enemies):
        """
        Find the nearest enemy within range
        
        Args:
            enemies: List of Enemy objects
            
        Returns:
            Enemy object or None
        """
        nearest_enemy = None
        nearest_distance = self.range + 1
        
        for enemy in enemies:
            if not enemy.alive:
                continue
            
            # Calculate distance
            dx = enemy.x - self.x
            dy = enemy.y - self.y
            distance = math.sqrt(dx * dx + dy * dy)
            
            if distance < nearest_distance:
                nearest_distance = distance
                nearest_enemy = enemy
        
        return nearest_enemy
    
    def shoot(self, target):
        """
        Fire a bullet at the target
        
        Args:
            target: Enemy object to shoot at
        """
        from tower import Projectile  # Import here to avoid circular dependency
        
        projectile = Projectile(
            self.x,
            self.y + 2,  # Shoot from bottom of helicopter
            target,
            self.damage,
            60,  # Fast bullet speed
            (255, 255, 0)  # Yellow tracer
        )
        self.projectiles.append(projectile)
    
    def draw(self, matrix):
        """Draw the Apache helicopter on the LED matrix (top-down view)"""
        x = int(self.x)
        y = int(self.y)
        
        # Draw main body FIRST
        body_color = (50, 150, 50)  # Military green
        # Main cockpit/body (3x3 solid block)
        matrix.fill_rect(x - 1, y - 1, 3, 3, *body_color)
        
        # Draw tail boom (extending right since flying left)
        tail_color = (40, 120, 40)  # Slightly darker green
        matrix.fill_rect(x + 2, y, 2, 1, *tail_color)
        
        # Tail fin (small vertical piece at end)
        matrix.set_pixel(x + 3, y - 1, *tail_color)
        matrix.set_pixel(x + 3, y + 1, *tail_color)
        
        # Tail rotor (animated - 2 phase spin at the very end)
        blade_color = (120, 120, 120)  # Lighter gray for visibility
        tail_phase = int((self.game_time * 15) % 2)
        if tail_phase == 0:
            matrix.set_pixel(x + 4, y, *blade_color)
        else:
            matrix.set_pixel(x + 4, y - 1, *blade_color)
            matrix.set_pixel(x + 4, y + 1, *blade_color)
        
        # Draw main rotor blades AFTER body (animated 4-phase for smoother rotation)
        rotor_phase = int((self.game_time * 12) % 4)  # 4 phases for smoother animation
        
        if rotor_phase == 0:
            # Horizontal blades
            matrix.set_pixel(x - 3, y, *blade_color)
            matrix.set_pixel(x - 2, y, *blade_color)
            matrix.set_pixel(x + 2, y, *blade_color)
            matrix.set_pixel(x + 3, y, *blade_color)
        elif rotor_phase == 1:
            # Diagonal blades (top-left to bottom-right)
            matrix.set_pixel(x - 2, y - 2, *blade_color)
            matrix.set_pixel(x - 1, y - 1, *blade_color)
            matrix.set_pixel(x + 1, y + 1, *blade_color)
            matrix.set_pixel(x + 2, y + 2, *blade_color)
        elif rotor_phase == 2:
            # Vertical blades
            matrix.set_pixel(x, y - 3, *blade_color)
            matrix.set_pixel(x, y - 2, *blade_color)
            matrix.set_pixel(x, y + 2, *blade_color)
            matrix.set_pixel(x, y + 3, *blade_color)
        else:
            # Diagonal blades (top-right to bottom-left)
            matrix.set_pixel(x + 2, y - 2, *blade_color)
            matrix.set_pixel(x + 1, y - 1, *blade_color)
            matrix.set_pixel(x - 1, y + 1, *blade_color)
            matrix.set_pixel(x - 2, y + 2, *blade_color)
        
        # Rotor hub (center point - medium gray for hub)
        matrix.set_pixel(x, y, 90, 90, 90)
        
        # Draw projectiles
        for projectile in self.projectiles:
            projectile.draw(matrix)


class BomberStrike:
    """
    Bomber plane that drops bombs along its path
    """
    
    def __init__(self, start_x, start_y, game_time, path_y=16):
        """
        Initialize the Bomber Strike
        
        Args:
            start_x, start_y: Starting position
            game_time: Current game time
            path_y: Y coordinate of the enemy path (where bombs should target)
        """
        self.x = start_x
        self.y = start_y
        self.spawn_time = game_time
        self.path_y = path_y  # Target altitude for bombing
        
        # Movement (straight horizontal path)
        self.speed_x = -12  # Horizontal speed
        
        # Combat stats
        self.damage = 4
        self.splash_radius = 4
        self.drop_interval = 0.8  # Seconds between bomb drops
        self.time_since_last_drop = 0
        
        # Visual
        self.color = (120, 120, 120)  # Gray bomber
        self.size = 4
        
        # State
        self.active = True
        self.bombs = []
    
    def update(self, dt, enemies):
        """Update bomber movement and bombing"""
        # Move horizontally (left)
        self.x += self.speed_x * dt
        
        # Check if off-screen
        if self.x < -10:
            self.active = False
            return
        
        # Update dropping
        self.time_since_last_drop += dt
        
        # Update bombs
        for bomb in self.bombs[:]:
            bomb.update(dt, enemies)
            if not bomb.active:
                self.bombs.remove(bomb)
        
        # Drop bombs
        if self.time_since_last_drop >= self.drop_interval:
            self.drop_bomb()
            self.time_since_last_drop = 0
    
    def drop_bomb(self):
        """Drop a bomb"""
        # FIXED: Pass path_y to Bomb constructor
        bomb = Bomb(self.x, self.y + 3, self.damage, self.splash_radius, self.path_y)
        self.bombs.append(bomb)
    
    def draw(self, matrix):
        """Draw the bomber plane (top-down view, flying left)"""
        x = int(self.x)
        y = int(self.y)
        
        # Top-down view of bomber flying left
        # Wings FIRST (so fuselage overlays them properly)
        wing_color = (100, 100, 100)
        matrix.fill_rect(x, y - 5, 2, 10, *wing_color)
        matrix.fill_rect(x + 1, y - 5, 1, 10, *self.color)  # Wing detail
        
        # Engine pods on wings (darker spots with glow)
        engine_color = (40, 40, 40)
        engine_glow = (255, 100, 0)
        # Left engine
        matrix.fill_rect(x + 1, y - 4, 1, 2, *engine_color)
        matrix.set_pixel(x + 2, y - 3, *engine_glow)  # Exhaust
        # Right engine
        matrix.fill_rect(x + 1, y + 2, 1, 2, *engine_color)
        matrix.set_pixel(x + 2, y + 3, *engine_glow)  # Exhaust
        
        # Main fuselage OVER wings (horizontal body since flying left)
        matrix.fill_rect(x - 2, y - 1, 6, 2, *self.color)
        
        # Cockpit/nose (front - pointing left, brighter)
        nose_color = (180, 180, 180)
        matrix.fill_rect(x - 3, y - 1, 2, 2, *nose_color)
        matrix.set_pixel(x - 4, y, *nose_color)
        
        # Tail fins (back - pointing right)
        matrix.fill_rect(x + 4, y - 2, 1, 4, *wing_color)
        
        # Draw bombs
        for bomb in self.bombs:
            bomb.draw(matrix)


class Bomb:
    """Falling bomb that explodes on impact"""
    
    def __init__(self, x, y, damage, splash_radius, path_y):
        """
        Initialize a bomb
        
        Args:
            x, y: Starting position
            damage: Damage dealt on explosion
            splash_radius: Radius of explosion
            path_y: Y coordinate to explode at (the path level)
        """
        self.x = x
        self.y = y
        self.damage = damage
        self.splash_radius = splash_radius
        self.path_y = path_y
        self.fall_speed = 15  # Slower fall speed so it travels further
        self.active = True
        self.exploded = False
        self.explosion_timer = 0  # For explosion animation
        self.color = (255, 200, 0)  # Yellow bomb
    
    def update(self, dt, enemies):
        """Update bomb falling"""
        if self.exploded:
            # Show explosion briefly
            self.explosion_timer += dt
            if self.explosion_timer > 0.15:  # 150ms explosion
                self.active = False
            return
        
        # Fall down
        self.y += self.fall_speed * dt
        
        # Explode when bomb reaches or passes the path level
        # Add a small buffer (+2) so it explodes slightly below the path for better visuals
        if self.y >= self.path_y + 2:
            self.explode(enemies)
    
    def explode(self, enemies):
        """Explode and damage nearby enemies"""
        self.exploded = True
        
        # Damage all enemies in splash radius
        for enemy in enemies:
            if not enemy.alive:
                continue
            
            dx = enemy.x - self.x
            dy = enemy.y - self.y
            distance = math.sqrt(dx * dx + dy * dy)
            
            if distance <= self.splash_radius:
                enemy.take_damage(self.damage)
    
    def draw(self, matrix):
        """Draw the bomb"""
        x = int(self.x)
        y = int(self.y)
        
        if not self.exploded:
            # Draw falling bomb
            matrix.fill_rect(x, y, 2, 2, *self.color)
        else:
            # Draw explosion effect
            explosion_color = (255, 100, 0)  # Orange explosion
            explosion_size = int(self.splash_radius * 1.5)
            matrix.fill_rect(
                x - explosion_size // 2, 
                y - explosion_size // 2, 
                explosion_size, 
                explosion_size, 
                *explosion_color
            )
            # Bright center
            matrix.fill_rect(x - 1, y - 1, 2, 2, 255, 255, 0)
            
    def draw2(self, matrix):
        x = int(self.x)
        y = int(self.y)

        if not self.exploded:
            # Falling bomb — small glowing pixel
            matrix.fill_rect(x, y, 2, 2, *self.color)
        else:
            # Explosion phase — smooth radial gradient
            radius = int(self.splash_radius * 1.2)
            for dx in range(-radius, radius + 1):
                for dy in range(-radius, radius + 1):
                    dist = math.sqrt(dx * dx + dy * dy)
                    if dist <= radius:
                        # Fade from bright center → dim edge
                        intensity = int(255 * (1 - dist / radius))
                        matrix.set_pixel(
                            x + dx, y + dy,
                            intensity, int(intensity * 0.5), 0
                        )


class AbilityManager:
    """
    Manages special abilities and cooldowns
    """
    
    ABILITY_TYPES = {
        "apache": {
            "name": "Apache Strike",
            "cost": 50,
            "cooldown": 15,  # Seconds
            "description": "Helicopter gunship"
        },
        "bomber": {
            "name": "Bomber Run",
            "cost": 75,
            "cooldown": 20,
            "description": "Carpet bombing"
        }
    }
    
    def __init__(self):
        """Initialize ability manager"""
        self.active_abilities = []
        self.cooldowns = {}  # Track cooldown timers
    
    def can_activate(self, ability_type, money, current_time):
        """
        Check if ability can be activated
        
        Args:
            ability_type: Type of ability ("apache", "bomber")
            money: Current player money
            current_time: Current game time
            
        Returns:
            bool: True if can activate
        """
        stats = self.ABILITY_TYPES[ability_type]
        
        # Check cost
        if money < stats["cost"]:
            return False
        
        # Check cooldown
        if ability_type in self.cooldowns:
            time_remaining = self.cooldowns[ability_type] - current_time
            if time_remaining > 0:
                return False
        
        return True
    
    def activate(self, ability_type, current_time, matrix_width=64, path_y=16):
        """
        Activate an ability
        
        Args:
            ability_type: Type of ability to activate
            current_time: Current game time
            matrix_width: Width of display (for spawn position)
            path_y: Y coordinate of enemy path (for bomber targeting)
            
        Returns:
            Ability object or None
        """
        stats = self.ABILITY_TYPES[ability_type]
        
        # Set cooldown
        self.cooldowns[ability_type] = current_time + stats["cooldown"]
        
        # Create ability
        if ability_type == "apache":
            ability = ApacheStrike(matrix_width + 5, 5, current_time)
        elif ability_type == "bomber":
            # Spawn bomber higher on screen so it's fully visible
            ability = BomberStrike(matrix_width + 5, 8, current_time, path_y)
        else:
            return None
        
        self.active_abilities.append(ability)
        return ability
    
    def update(self, dt, enemies):
        """Update all active abilities"""
        for ability in self.active_abilities[:]:
            ability.update(dt, enemies)
            if not ability.active:
                self.active_abilities.remove(ability)
    
    def draw(self, matrix):
        """Draw all active abilities"""
        for ability in self.active_abilities:
            ability.draw(matrix)
    
    def get_cooldown_remaining(self, ability_type, current_time):
        """
        Get remaining cooldown time for an ability
        
        Returns:
            float: Seconds remaining (0 if ready)
        """
        if ability_type not in self.cooldowns:
            return 0
        
        remaining = self.cooldowns[ability_type] - current_time
        return max(0, remaining)