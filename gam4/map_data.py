"""
Map Data structures and loader for Tower Defense Game
Handles loading/saving map files in JSON format
"""

import json


class MapData:
    """Container for map data"""
    
    def __init__(self):
        self.name = ""
        self.width = 64
        self.height = 32
        self.path = []  # List of (x, y) tuples - waypoints for enemy path
        self.tower_slots = []  # List of (x, y) tuples for tower placement
        self.spawn_point = (0, 0)  # Where enemies spawn
        self.end_point = (0, 0)  # Where enemies exit
        self.waves = []  # Number of enemies per wave
        self.decorations = []  # List of decorations (trees, rocks, etc)
        self.background_color = (0, 50, 0)  # Default dark green
    
    def __str__(self):
        return (f"Map: {self.name}\n"
                f"Size: {self.width}x{self.height}\n"
                f"Path waypoints: {len(self.path)}\n"
                f"Tower slots: {len(self.tower_slots)}\n"
                f"Decorations: {len(self.decorations)}\n"
                f"Waves: {len(self.waves)}")


class MapLoader:
    """Load and save map files"""
    
    @staticmethod
    def load_json(filename):
        """
        Load map from JSON file
        
        Args:
            filename: Path to JSON file
            
        Returns:
            MapData object
        """
        with open(filename, 'r') as f:
            data = json.load(f)
        
        map_data = MapData()
        map_data.name = data.get("name", "Unnamed")
        map_data.width = data.get("width", 64)
        map_data.height = data.get("height", 32)
        map_data.path = [tuple(p) for p in data["path"]]
        map_data.tower_slots = [tuple(t) for t in data.get("towers", [])]
        map_data.spawn_point = tuple(data.get("spawn", [0, 0]))
        map_data.end_point = tuple(data.get("end", [63, 31]))
        map_data.waves = data.get("waves", [5, 8, 12])
        map_data.decorations = data.get("decorations", [])
        
        # Load background color if specified
        bg = data.get("background_color", [0, 50, 0])
        map_data.background_color = tuple(bg)
        
        return map_data
    
    @staticmethod
    def save_json(filename, map_data):
        """
        Save map to JSON file
        
        Args:
            filename: Path to save JSON file
            map_data: MapData object to save
        """
        data = {
            "name": map_data.name,
            "width": map_data.width,
            "height": map_data.height,
            "background_color": list(map_data.background_color),
            "path": list(map_data.path),
            "towers": list(map_data.tower_slots),
            "spawn": list(map_data.spawn_point),
            "end": list(map_data.end_point),
            "decorations": map_data.decorations,
            "waves": map_data.waves
        }
        
        with open(filename, 'w') as f:
            json.dump(data, f, indent=2)


def create_sample_maps():
    """Create some sample maps for testing"""
    
    # Map 1: Forest Path (Right to Left with trees)
    map1 = MapData()
    map1.name = "Forest Path"
    map1.width = 64
    map1.height = 32
    map1.background_color = (0, 50, 0)  # Dark green
    
    # Right to left path
    map1.path = [
        (63, 15),  # Start right
        (50, 15),
        (50, 25),
        (30, 25),
        (30, 10),
        (15, 10),
        (15, 20),
        (0, 20)    # Exit left
    ]
    
    map1.tower_slots = [
        (55, 8), (55, 22), (38, 18), (20, 6), (20, 28)
    ]
    
    map1.spawn_point = (63, 15)
    map1.end_point = (0, 20)
    map1.waves = [5, 8, 10, 12, 15, 20]
    
    # Add tree decorations
    map1.decorations = [
        {"type": "tree", "x": 10, "y": 5},
        {"type": "tree", "x": 10, "y": 26},
        {"type": "tree", "x": 25, "y": 4},
        {"type": "tree", "x": 35, "y": 28},
        {"type": "tree", "x": 45, "y": 8},
        {"type": "tree", "x": 58, "y": 5},
        {"type": "tree", "x": 58, "y": 28},
    ]
    
    # Map 2: Desert Path (Straight shot, no trees)
    map2 = MapData()
    map2.name = "Desert Straight"
    map2.width = 64
    map2.height = 32
    map2.background_color = (80, 60, 20)  # Sandy brown
    map2.path = [
        (63, 16), (0, 16)  # Simple right to left
    ]
    map2.tower_slots = [
        (53, 8), (53, 24), (32, 8), (32, 24), (11, 8), (11, 24)
    ]
    map2.spawn_point = (63, 16)
    map2.end_point = (0, 16)
    map2.waves = [3, 5, 8, 12, 15]
    
    # Add rocks for desert theme
    map2.decorations = [
        {"type": "rock", "x": 20, "y": 5},
        {"type": "rock", "x": 45, "y": 26},
        {"type": "rock", "x": 55, "y": 10},
    ]
    
    return [map1, map2]


# Test code
if __name__ == "__main__":
    import os
    
    # Create maps directory if it doesn't exist
    os.makedirs("maps", exist_ok=True)
    
    # Create and save sample maps
    maps = create_sample_maps()
    
    for i, map_data in enumerate(maps, 1):
        filename = f"maps/map{i}.json"
        MapLoader.save_json(filename, map_data)
        print(f"Created {filename}")
        print(map_data)
        print()
    
    # Test loading
    print("Testing load...")
    loaded = MapLoader.load_json("maps/map1.json")
    print(f"Loaded: {loaded.name}")
    print("Success!")