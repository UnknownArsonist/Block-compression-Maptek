#!/usr/bin/env python3
"""
3D Model to Text Input Converter
Converts 3D model files (OBJ, PLY, STL) to the text format used by the compression system.
"""

import numpy as np
import argparse
import sys
import os
from typing import Tuple, List, Optional

try:
    import trimesh
    TRIMESH_AVAILABLE = True
except ImportError:
    TRIMESH_AVAILABLE = False
    print("Warning: trimesh not available. Install with: pip install trimesh")

class ModelToTextConverter:
    def __init__(self):
        self.material_map = {
            'ore': 'o',
            'titanium': 't', 
            'sulfur': 's',
            'nickel': 'n',
            'air': 'a',
            'empty': ' '
        }
    
    def load_model(self, filepath: str) -> Optional[object]:
        """Load a 3D model file using trimesh."""
        if not TRIMESH_AVAILABLE:
            print("Error: trimesh required for loading 3D models")
            return None
            
        try:
            mesh = trimesh.load(filepath)
            return mesh
        except Exception as e:
            print(f"Error loading model {filepath}: {e}")
            return None
    
    def voxelize_mesh(self, mesh, resolution: int = 32) -> np.ndarray:
        """Convert mesh to voxel grid."""
        if not TRIMESH_AVAILABLE:
            return None
            
        # Create voxel grid
        voxels = mesh.voxelized(pitch=mesh.scale / resolution)
        return voxels.matrix
    
    def assign_materials(self, voxel_grid: np.ndarray, material_strategy: str = 'random') -> np.ndarray:
        """Assign materials to voxels based on strategy."""
        result = np.full(voxel_grid.shape, ' ', dtype=str)
        
        if material_strategy == 'random':
            # Randomly assign materials to filled voxels
            materials = ['o', 't', 's', 'n']
            filled_voxels = voxel_grid.astype(bool)
            
            for i in range(voxel_grid.shape[0]):
                for j in range(voxel_grid.shape[1]):
                    for k in range(voxel_grid.shape[2]):
                        if filled_voxels[i, j, k]:
                            result[i, j, k] = np.random.choice(materials)
                        else:
                            result[i, j, k] = 'o'  # background material
        
        elif material_strategy == 'layered':
            # Assign materials based on height layers
            height = voxel_grid.shape[2]
            filled_voxels = voxel_grid.astype(bool)
            
            for i in range(voxel_grid.shape[0]):
                for j in range(voxel_grid.shape[1]):
                    for k in range(voxel_grid.shape[2]):
                        if filled_voxels[i, j, k]:
                            if k < height * 0.25:
                                result[i, j, k] = 's'  # sulfur at bottom
                            elif k < height * 0.5:
                                result[i, j, k] = 'o'  # ore in middle-lower
                            elif k < height * 0.75:
                                result[i, j, k] = 't'  # titanium in middle-upper
                            else:
                                result[i, j, k] = 'n'  # nickel at top
                        else:
                            result[i, j, k] = 'o'  # background
        
        return result
    
    def generate_test_data(self, dimensions: Tuple[int, int, int], 
                          pattern: str = 'random') -> np.ndarray:
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
            layer_height = z // len(materials)
            
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
        
        return result
    
    def voxels_to_text_format(self, voxels: np.ndarray, tag_table: dict = None) -> str:
        """Convert voxel array to the text format expected by the compression system."""
        if tag_table is None:
            tag_table = {
                'o': 'ore',
                't': 'titanium', 
                's': 'sulfur',
                'n': 'nickel'
            }
        
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
        header = f"{x},{y},{z},{8},{8},{8}\n"
        
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
    
    def convert_model_file(self, input_file: str, output_file: str, 
                          resolution: int = 32, material_strategy: str = 'random'):
        """Convert a 3D model file to text format."""
        mesh = self.load_model(input_file)
        if mesh is None:
            return False
        
        print(f"Loaded model: {mesh.vertices.shape[0]} vertices, {mesh.faces.shape[0]} faces")
        
        # Voxelize the mesh
        voxels = self.voxelize_mesh(mesh, resolution)
        if voxels is None:
            return False
        
        print(f"Voxelized to {voxels.shape} grid")
        
        # Assign materials
        material_voxels = self.assign_materials(voxels, material_strategy)
        
        # Convert to text format
        text_data = self.voxels_to_text_format(material_voxels)
        
        # Write to file
        with open(output_file, 'w') as f:
            f.write(text_data)
        
        print(f"Converted model saved to {output_file}")
        return True

def main():
    parser = argparse.ArgumentParser(description='Convert 3D models to compression test format')
    parser.add_argument('--input', '-i', help='Input 3D model file (OBJ, PLY, STL)')
    parser.add_argument('--output', '-o', help='Output text file')
    parser.add_argument('--resolution', '-r', type=int, default=32, 
                       help='Voxel resolution (default: 32)')
    parser.add_argument('--material', '-m', choices=['random', 'layered'], 
                       default='random', help='Material assignment strategy')
    parser.add_argument('--generate', '-g', action='store_true', help='Generate test data instead of converting model')
    parser.add_argument('--dimensions', '-d', default='32,32,32', 
                       help='Dimensions for generated data (x,y,z)')
    parser.add_argument('--pattern', '-p', choices=['random', 'blocks', 'layers', 'sphere'],
                       default='random', help='Pattern for generated data')
    
    args = parser.parse_args()
    
    converter = ModelToTextConverter()
    
    if args.generate:
        # Generate synthetic test data
        dims = tuple(map(int, args.dimensions.split(',')))
        print(f"Generating {args.pattern} test data with dimensions {dims}")
        
        voxels = converter.generate_test_data(dims, args.pattern)
        text_data = converter.voxels_to_text_format(voxels)
        
        output_file = args.output or f"test_data_{args.pattern}_{dims[0]}x{dims[1]}x{dims[2]}.txt"
        
        # Ensure directory exists
        import os
        os.makedirs(os.path.dirname(output_file) if os.path.dirname(output_file) else '.', exist_ok=True)
        
        with open(output_file, 'w') as f:
            f.write(text_data)
        
        print(f"Generated test data saved to {output_file}")
    
    elif args.input:
        # Convert 3D model file
        if not args.output:
            base_name = os.path.splitext(args.input)[0]
            args.output = f"{base_name}_voxels.txt"
        
        success = converter.convert_model_file(args.input, args.output, 
                                             args.resolution, args.material)
        if not success:
            sys.exit(1)
    
    else:
        parser.print_help()
        sys.exit(1)

if __name__ == "__main__":
    main()
