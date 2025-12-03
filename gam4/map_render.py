"""
Map Renderer for Tower Defense Game
Draws map data to the LED matrix
"""
import random

class MapRenderer:
    """Draws map data to the LED matrix"""
    # Color definitions
    PATH_COLOR = (100, 100, 100)      # gray path
    TOWER_SLOT_COLOR = (128, 107, 0)  # gold tower slots
    TOWER_SELECTION_COLOR = (255, 215, 0)
    SPAWN_COLOR = (255, 255, 0)       # Yellow spawn
    END_COLOR = (255, 0, 255)         # Magenta end
    
    # Decoration colors
    TREE_COLOR = (0, 80, 0)
    TREETRUNK_COLOR = (100, 50, 0)
    ROCK_COLOR = (75, 75, 75)
    LAKE_COLOR = (0, 0, 150)
    
    def __init__(self, matrix, map_data):
        """
        Initialize the map renderer
        
        Args:
            matrix: LEDMatrixSimulator instance
            map_data: MapData instance to render
        """
        self.matrix = matrix
        self.map_data = map_data
        self.background_pixels = None  # Cache background
        self.path_pixels = {}  # Cache path texture: {(x, y): (r, g, b)}
        self._generate_background()
        self._generate_path()
    
    def _generate_background(self):
        """Generate the background with variations once"""
        variation = 5
        base_r, base_g, base_b = self.map_data.background_color
        self.background_pixels = []
        
        for y in range(self.matrix.height):
            row = []
            for x in range(self.matrix.width):
                r = max(0, min(255, base_r + random.randint(-variation, variation)))
                g = max(0, min(255, base_g + random.randint(-variation, variation)))
                b = max(0, min(255, base_b + random.randint(-variation, variation)))
                row.append((r, g, b))
            self.background_pixels.append(row)
    
    def _generate_path(self):
        """Generate textured path once"""
        self.path_pixels = {}
        variation = 8
        base_r, base_g, base_b = self.PATH_COLOR
        
        # Generate path texture for all path segments
        for i in range(len(self.map_data.path) - 1):
            x1, y1 = self.map_data.path[i]
            x2, y2 = self.map_data.path[i + 1]
            
            # Get all pixels along this path segment (3 pixels wide)
            path_points = self._get_path_segment_pixels(x1, y1, x2, y2)
            
            # Assign random texture to each pixel
            for px, py in path_points:
                r = max(0, min(255, base_r + random.randint(-variation, variation)))
                g = max(0, min(255, base_g + random.randint(-variation, variation)))
                b = max(0, min(255, base_b + random.randint(-variation, variation)))
                self.path_pixels[(px, py)] = (r, g, b)
    
    def _get_path_segment_pixels(self, x1, y1, x2, y2):
        """Get all pixels that make up a 3-pixel-wide path segment"""
        pixels = set()
        
        # Get center line pixels
        center_pixels = self._bresenham_line(x1, y1, x2, y2)
        
        # Add center and perpendicular offsets for width
        for cx, cy in center_pixels:
            pixels.add((cx, cy))
            
            if y1 == y2:  # Horizontal path
                pixels.add((cx, cy - 1))
                pixels.add((cx, cy + 1))
            else:  # Vertical path
                pixels.add((cx - 1, cy))
                pixels.add((cx + 1, cy))
        
        return pixels
    
    def _bresenham_line(self, x1, y1, x2, y2):
        """Get all pixels along a line using Bresenham's algorithm"""
        pixels = []
        dx = abs(x2 - x1)
        dy = abs(y2 - y1)
        sx = 1 if x1 < x2 else -1
        sy = 1 if y1 < y2 else -1
        err = dx - dy
        
        while True:
            pixels.append((x1, y1))
            if x1 == x2 and y1 == y2:
                break
            e2 = 2 * err
            if e2 > -dy:
                err -= dy
                x1 += sx
            if e2 < dx:
                err += dx
                y1 += sy
        
        return pixels

    def draw_background(self):
        """Draw the cached background color"""
        for y in range(self.matrix.height):
            for x in range(self.matrix.width):
                r, g, b = self.background_pixels[y][x]
                self.matrix.fill_rect(x, y, 1, 1, r, g, b)
    
    def draw_decorations(self):
        """Draw decorative elements (trees, rocks, etc)"""
        for deco in self.map_data.decorations:
            deco_type = deco.get("type", "tree")
            x = deco.get("x", 0)
            y = deco.get("y", 0)
            
            if deco_type == "tree":
                self.matrix.fill_rect(x - 1, y - 1, 3, 3, *self.TREE_COLOR)
                self.matrix.fill_rect(x, y + 2, 1, 2, *self.TREETRUNK_COLOR)
                self.matrix.fill_rect(x - 1, y + 3, 3, 1, *self.TREETRUNK_COLOR)
            elif deco_type == "rock":
                self.matrix.fill_rect(x - 1, y - 1, 2, 2, *self.ROCK_COLOR)
                self.matrix.fill_rect(x + 1, y, 1, 1, *self.ROCK_COLOR)
            elif deco_type == "lake":
                self.matrix.fill_rect(x-1, y-1, 6, 2, *self.LAKE_COLOR)
                self.matrix.fill_rect(x, y-2, 3, 1, *self.LAKE_COLOR)
                self.matrix.fill_rect(x+1, y+1, 3, 1, *self.LAKE_COLOR)
    
    def draw_path(self):
        """Draw the enemy path (3 pixels wide) with cached texture"""
        for (px, py), color in self.path_pixels.items():
            if 0 <= px < self.matrix.width and 0 <= py < self.matrix.height:
                self.matrix.set_pixel(px, py, *color)
    
    def draw_tower_slots(self):
        """Draw tower placement locations"""
        for x, y in self.map_data.tower_slots:
            self.matrix.fill_rect(x - 2, y - 2, 4, 4, *self.TOWER_SLOT_COLOR)
    
    def draw_spawn_and_end(self):
        """Draw spawn and end points"""
        sx, sy = self.map_data.spawn_point
        self.matrix.fill_rect(sx - 1, sy - 1, 2, 2, *self.SPAWN_COLOR)
        
        ex, ey = self.map_data.end_point
        self.matrix.fill_rect(ex - 1, ey - 1, 2, 2, *self.END_COLOR)
    
    def draw_all(self):
        """Draw the complete map (call this every frame)"""
        self.draw_background()
        self.draw_decorations()
        self.draw_path()
        self.draw_tower_slots()
        # self.draw_spawn_and_end()


# Test code
if __name__ == "__main__":
    from led_matrix import LEDMatrixSimulator
    from map_data import MapLoader
    import pygame
    
    print("Testing Map Renderer...")
    
    # Load a map
    try:
        map_data = MapLoader.load_json("maps/map1.json")
    except FileNotFoundError:
        print("Error: Run map_data.py first to create sample maps!")
        exit(1)
    
    print(f"Loaded: {map_data.name}")
    
    # Create matrix and renderer
    matrix = LEDMatrixSimulator(64, 32, pixel_size=15)
    renderer = MapRenderer(matrix, map_data)
    
    # Draw the map
    matrix.clear()
    renderer.draw_all()
    matrix.update_display()
    
    print("Displaying map. Close window to exit...")
    
    # Keep window open
    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
        matrix.tick(10)
    
    matrix.close()
    print("Done!")