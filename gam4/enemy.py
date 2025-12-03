class Enemy:
    
    # Enemy type definitions
    ENEMY_TYPES = {
        "scout": {
            "health": 3,
            "speed": 4,
            "color": (200, 20, 20),
            "reward": 2,
            "damage": 1,
            "size": 1,
            "invisible": False,
            "splits_on_death": False
        },
        "tank": {
            "health": 10,
            "speed": 2,
            "color": (80, 80, 80),
            "armor_color": (255, 200, 0),  # Yellow stripes
            "reward": 6,
            "damage": 2,
            "size": 1,
            "invisible": False,
            "splits_on_death": False
        },
        "splitter": {
            "health": 2,
            "speed": 7,
            "color": (255, 180, 0),  # Orange
            "reward": 3,
            "damage": 1,
            "size": 1,
            "invisible": False,
            "splits_on_death": True,
            "split_count": 2,
            "split_type": "scout"
        },
        "ghost": {
            "health": 4,
            "speed": 5,
            "color": (150, 100, 255),  # Purple (not used when invisible)
            "reward": 5,
            "damage": 1,
            "size": 1,
            "invisible": True,
            "splits_on_death": False
        }
    }
    
    def __init__(self, enemy_type, path, spawn_time):
        """
        Initialize an enemy
        
        Args:
            enemy_type: Type of enemy
            path: List of (x, y) waypoints to follow
            spawn_time: Time when enemy was spawned
        """
        self.enemy_type = enemy_type
        self.path = path
        self.spawn_time = spawn_time
        
        # Load stats from enemy type
        stats = self.ENEMY_TYPES[enemy_type]
        self.max_health = stats["health"]
        self.health = self.max_health
        self.speed = stats["speed"]
        self.color = stats["color"]
        self.armor_color = stats.get("armor_color", None)
        self.reward = stats["reward"]
        self.damage = stats["damage"]
        self.size = stats["size"]
        self.invisible = stats["invisible"]
        self.splits_on_death = stats["splits_on_death"]
        self.split_count = stats.get("split_count", 0)
        self.split_type = stats.get("split_type", None)
        
        # Position tracking
        self.x, self.y = path[0]  # Start at first waypoint
        self.path_index = 0  # Current waypoint we're moving toward
        self.path_progress = 0.0  # Total distance traveled along path
        
        # State
        self.alive = True
        self.revealed = False  # For radar detection
        self.reveal_timer = 0.0  # Time remaining for reveal effect
    
    def update(self, dt):
        """
        Move enemy along the path
        
        Args:
            dt: Delta time (time since last update)
            
        Returns:
            bool: True if reached end of path, False otherwise
        """
        # Update reveal timer
        if self.reveal_timer > 0:
            self.reveal_timer -= dt
            if self.reveal_timer <= 0:
                self.revealed = False
        
        if self.path_index >= len(self.path) - 1:
            # Reached the end
            return True
        
        # Get current target waypoint
        target_x, target_y = self.path[self.path_index + 1] 
        
        # Calculate direction to target
        dx = target_x - self.x
        dy = target_y - self.y
        distance = (dx * dx + dy * dy) ** 0.5
        
        if distance == 0:
            # Already at waypoint, move to next
            self.path_index += 1
            return False
        
        # Calculate movement distance this frame
        move_distance = self.speed * dt
        
        if distance <= move_distance:
            # Reached waypoint, move to next
            self.x = target_x
            self.y = target_y
            self.path_progress += distance
            self.path_index += 1
        else:
            # Move toward waypoint
            # Normalize direction and scale by move_distance
            self.x += (dx / distance) * move_distance
            self.y += (dy / distance) * move_distance
            self.path_progress += move_distance
        
        return False  # Not at end yet
    
    def take_damage(self, damage):
        """
        Apply damage to enemy
        
        Args:
            damage: Amount of damage to apply
        """
        self.health -= damage
        if self.health <= 0:
            self.alive = False
    
    def is_visible_to_tower(self, tower):
        """
        Check if this enemy can be seen by a tower
        
        Args:
            tower: Tower object
            
        Returns:
            bool: True if visible
        """
        if not self.invisible:
            return True
        
        if self.revealed:
            return True
        
        return tower.can_see_invisible
    
    def draw(self, matrix):
        """Draw the enemy on the LED matrix"""
        x = int(self.x)
        y = int(self.y)
        
        # Ghost refraction effect (blend with background)
        if self.invisible and not self.revealed:
            # Get the background color at this position
            if 0 <= y < len(matrix.buffer) and 0 <= x < len(matrix.buffer[0]):
                bg_r, bg_g, bg_b = matrix.buffer[y][x]
                
                # Create very subtle "refraction" by barely shifting the color
                # Just add a tiny bit of blue tint (almost invisible)
                refract_r = min(255, int(bg_r * 1.0 + 3))
                refract_g = min(255, int(bg_g * 1.0 + 5))
                refract_b = min(255, int(bg_b * 1.0 + 15))
                
                matrix.set_pixel(x, y, refract_r, refract_g, refract_b)
        else:
            # Draw normally as single pixel
            matrix.set_pixel(x, y, *self.color)