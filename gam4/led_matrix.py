"""
LED Matrix Simulator for Tower Defense Game
Simulates a 64x32 RGB LED matrix using pygame
"""

import pygame


class LEDMatrixSimulator:
    """Simulates an RGB LED matrix display"""
    
    def __init__(self, width=64, height=32, pixel_size=10):
        """
        Initialize the LED matrix simulator
        
        Args:
            width: Matrix width in pixels (default 64)
            height: Matrix height in pixels (default 32)
            pixel_size: Size of each LED in screen pixels (default 10)
        """
        self.width = width
        self.height = height
        self.pixel_size = pixel_size
        
        # 3D array: [row][column][RGB]
        self.buffer = [[[0, 0, 0] for _ in range(width)] for _ in range(height)]
        
        # Initialize pygame
        pygame.init()
        self.screen = pygame.display.set_mode((width * pixel_size, height * pixel_size))
        pygame.display.set_caption(f"LED Matrix Simulator ({width}x{height})")
        self.clock = pygame.time.Clock()
    
    def set_pixel(self, x, y, r, g, b):
        """
        Set a single pixel color
        
        Args:
            x, y: Pixel coordinates
            r, g, b: RGB color values (0-255)
        """
        if 0 <= x < self.width and 0 <= y < self.height:
            self.buffer[y][x] = [r, g, b]
    
    def clear(self):
        """Clear the entire display to black"""
        self.buffer = [[[0, 0, 0] for _ in range(self.width)] for _ in range(self.height)]
    
    def fill_rect(self, x, y, width, height, r, g, b):
        """
        Draw a filled rectangle
        
        Args:
            x, y: Top-left corner
            width, height: Rectangle dimensions
            r, g, b: RGB color values (0-255)
        """
        for dy in range(height):
            for dx in range(width):
                self.set_pixel(x + dx, y + dy, r, g, b)
    
    def draw_line(self, x1, y1, x2, y2, r, g, b):
        """
        Draw a line using Bresenham's algorithm
        
        Args:
            x1, y1: Start point
            x2, y2: End point
            r, g, b: RGB color values (0-255)
        """
        dx = abs(x2 - x1)
        dy = abs(y2 - y1)
        sx = 1 if x1 < x2 else -1
        sy = 1 if y1 < y2 else -1
        err = dx - dy
        
        while True:
            self.set_pixel(x1, y1, r, g, b)
            if x1 == x2 and y1 == y2:
                break
            e2 = 2 * err
            if e2 > -dy:
                err -= dy
                x1 += sx
            if e2 < dx:
                err += dx
                y1 += sy
    
    def update_display(self):
        """
        Render the buffer to the pygame window
        
        Returns:
            bool: False if window was closed, True otherwise
        """
        # Check for quit event
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return False
        
        # Draw each pixel as a rectangle
        for y in range(self.height):
            for x in range(self.width):
                r, g, b = self.buffer[y][x]
                color = (r, g, b)
                rect = pygame.Rect(
                    x * self.pixel_size,
                    y * self.pixel_size,
                    self.pixel_size,
                    self.pixel_size
                )
                pygame.draw.rect(self.screen, color, rect)
        
        pygame.display.flip()
        return True
    
    def tick(self, fps=60):
        """
        Control frame rate
        
        Args:
            fps: Target frames per second
        """
        self.clock.tick(fps)
    
    def close(self):
        """Clean up pygame resources"""
        pygame.quit()


# Test code
if __name__ == "__main__":
    import time
    
    print("Testing LED Matrix Simulator...")
    matrix = LEDMatrixSimulator(64, 32, pixel_size=15)
    
    # Test drawing
    matrix.clear()
    matrix.fill_rect(10, 10, 5, 5, 255, 0, 0)  # Red square
    matrix.draw_line(0, 16, 63, 16, 0, 255, 0)  # Green line
    matrix.set_pixel(32, 16, 0, 0, 255)  # Blue pixel
    matrix.update_display()
    
    print("Close the window to exit...")
    running = True
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
        matrix.tick(10)
    
    matrix.close()
    print("Done!")