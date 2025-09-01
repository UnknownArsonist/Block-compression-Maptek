#!/usr/bin/env python3
"""
Simple Test Data Generator for Compression Testing
Generates voxel data in the format expected by the compression system.
No 3D model dependencies - pure text-based voxel generation.
"""

import numpy as np
import argparse
import os
from typing import Tuple

class SimpleTestGenerator:
    def __init__(self):
        self.material_map = {
            'o': 'ore',
            't': 'titanium', 
            's': 'sulfur',
            'n': 'nickel'
        }
    
    def generate_test_data(self, dimensions: Tuple[int, int, int], pattern: str = 'random') -> np.ndarray:
        """Generate synthetic test data with specific patterns."""
        x, y, z = dimensions
        result = np.full((x, y, z), 'o', dtype=str)
        
        if pattern == 'random':
            materials = ['o', 't', 's', 'n']
            for i in range(x):
                for j in range(y):
                    for k in range(z):
                        result[i, j, k] = np.random.choice(materials)
        
        elif pattern == 'blocks':
            # Create large uniform blocks for testing compression efficiency
            block_size = 8
            materials = ['o', 't', 's', 'n']
            
            for i in range(0, x, block_size):
                for j in range(0, y, block_size):
                    for k in range(0, z, block_size):
                        material = np.random.choice(materials)
                        for di in range(min(block_size, x-i)):
                            for dj in range(min(block_size, y-j)):
                                for dk in range(min(block_size, z-k)):
                                    result[i+di, j+dj, k+dk] = material
        
        elif pattern == 'layers':
            # Create horizontal layers
            materials = ['s', 'o', 't', 'n']
            layer_height = max(1, z // len(materials))
            
            for k in range(z):
                material_idx = min(k // layer_height, len(materials) - 1)
                material = materials[material_idx]
                result[:, :, k] = material
        
        elif pattern == 'sphere':
            # Create a sphere of different material
            center = (x//2, y//2, z//2)
            radius = min(x, y, z) // 4
            
            for i in range(x):
                for j in range(y):
                    for k in range(z):
                        dist = np.sqrt((i-center[0])**2 + (j-center[1])**2 + (k-center[2])**2)
                        if dist <= radius:
                            result[i, j, k] = 't'
        
        elif pattern == 'mixed':
            # Create a mix of patterns for comprehensive testing
            # Bottom layer: uniform
            result[:, :, :z//3] = 's'
            # Middle layer: blocks
            mid_start, mid_end = z//3, 2*z//3
            block_size = 4
            for i in range(0, x, block_size):
                for j in range(0, y, block_size):
                    material = np.random.choice(['o', 't'])
                    for di in range(min(block_size, x-i)):
                        for dj in range(min(block_size, y-j)):
                            result[i+di, j+dj, mid_start:mid_end] = material
            # Top layer: random
            for i in range(x):
                for j in range(y):
                    for k in range(2*z//3, z):
                        result[i, j, k] = np.random.choice(['o', 't', 'n'])
        
        return result
    
    def voxels_to_text_format(self, voxels: np.ndarray, tag_table: dict = None) -> str:
        """Convert voxel array to the text format expected by the compression system."""
        if tag_table is None:
            tag_table = self.material_map
        
        x, y, z = voxels.shape
        
        # Pad dimensions to multiples of 8 to avoid segfaults
        pad_x = ((x + 7) // 8) * 8
        pad_y = ((y + 7) // 8) * 8
        pad_z = ((z + 7) // 8) * 8
        
        if pad_x != x or pad_y != y or pad_z != z:
            print(f"Padding from {x}x{y}x{z} to {pad_x}x{pad_y}x{pad_z} for compatibility")
            padded_voxels = np.full((pad_x, pad_y, pad_z), 'o', dtype=str)
            padded_voxels[:x, :y, :z] = voxels
            voxels = padded_voxels
            x, y, z = pad_x, pad_y, pad_z
        
        # Create header: dimensions and parent block size
        header = f"{x},{y},{z},8,8,8\n"
        
        # Add tag table
        for tag, name in tag_table.items():
            header += f"{tag},{name}\n"
        header += "\n"
        
        # Convert voxel data to 2D slices
        result = header
        for k in range(z):
            for j in range(y):
                row = ""
                for i in range(x):
                    row += voxels[i, j, k]
                result += row + "\n"
            result += "\n"
        
        return result
    
    def generate_like_original(self, base_file: str, pattern: str, output_file: str):
        """Generate test data similar to an existing file's format."""
        # Read the header from the base file to get dimensions
        with open(base_file, 'r') as f:
            header = f.readline().strip()
            dimensions = tuple(map(int, header.split(',')[:3]))
        
        print(f"Generating {pattern} data with dimensions {dimensions} (like {base_file})")
        
        voxels = self.generate_test_data(dimensions, pattern)
        text_data = self.voxels_to_text_format(voxels)
        
        os.makedirs(os.path.dirname(output_file) if os.path.dirname(output_file) else '.', exist_ok=True)
        
        with open(output_file, 'w') as f:
            f.write(text_data)
        
        print(f"Generated test data saved to {output_file}")

def main():
    parser = argparse.ArgumentParser(description='Generate compression test data')
    parser.add_argument('--output', '-o', required=True, help='Output text file')
    parser.add_argument('--dimensions', '-d', default='32,32,32', 
                       help='Dimensions for generated data (x,y,z)')
    parser.add_argument('--pattern', '-p', choices=['random', 'blocks', 'layers', 'sphere', 'mixed'],
                       default='blocks', help='Pattern for generated data')
    parser.add_argument('--like', '-l', help='Generate data similar to existing file format')
    
    args = parser.parse_args()
    
    generator = SimpleTestGenerator()
    
    if args.like:
        # Generate data similar to existing file
        generator.generate_like_original(args.like, args.pattern, args.output)
    else:
        # Generate data with specified dimensions
        dims = tuple(map(int, args.dimensions.split(',')))
        print(f"Generating {args.pattern} test data with dimensions {dims}")
        
        voxels = generator.generate_test_data(dims, args.pattern)
        text_data = generator.voxels_to_text_format(voxels)
        
        os.makedirs(os.path.dirname(args.output) if os.path.dirname(args.output) else '.', exist_ok=True)
        
        with open(args.output, 'w') as f:
            f.write(text_data)
        
        print(f"Generated test data saved to {args.output}")

if __name__ == "__main__":
    main()
