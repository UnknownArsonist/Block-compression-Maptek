#!/usr/bin/env python3
"""
Decompressor for the block compression system.
Converts compressed output back to 3D voxel data and various output formats.
"""

import numpy as np
import argparse
import json
import struct
from typing import List, Tuple, Dict

try:
    import trimesh
    TRIMESH_AVAILABLE = True
except ImportError:
    TRIMESH_AVAILABLE = False

class Decompressor:
    def __init__(self):
        self.tag_table = {}
        self.reverse_tag_table = {}
    
    def parse_subblock_output(self, output_text: str) -> List[dict]:
        """Parse the compressed output format into subblock data."""
        subblocks = []
        
        lines = output_text.strip().split('\n')
        for line in lines:
            if not line.strip():
                continue
            
            # Expected format: x,y,z,length,width,height,material_name
            parts = line.split(',')
            if len(parts) >= 7:
                try:
                    subblock = {
                        'x': int(parts[0]),
                        'y': int(parts[1]),
                        'z': int(parts[2]),
                        'length': int(parts[3]),
                        'width': int(parts[4]),
                        'height': int(parts[5]),
                        'material': parts[6]
                    }
                    subblocks.append(subblock)
                except ValueError as e:
                    print(f"Error parsing line: {line} - {e}")
        
        return subblocks
    
    def parse_binary_output(self, binary_data: bytes) -> List[dict]:
        """Parse binary SubBlock output."""
        subblocks = []
        
        # SubBlock structure: int x,y,z,l,w,h; char tag
        struct_format = 'iiiiiiic'  # 6 ints + 1 char
        struct_size = struct.calcsize(struct_format)
        
        offset = 0
        while offset + struct_size <= len(binary_data):
            try:
                data = struct.unpack_from(struct_format, binary_data, offset)
                subblock = {
                    'x': data[0],
                    'y': data[1],
                    'z': data[2],
                    'length': data[3],
                    'width': data[4],
                    'height': data[5],
                    'material': data[6].decode('ascii')
                }
                subblocks.append(subblock)
                offset += struct_size
            except struct.error as e:
                print(f"Error parsing binary data at offset {offset}: {e}")
                break
        
        return subblocks
    
    def reconstruct_voxels(self, subblocks: List[dict], dimensions: Tuple[int, int, int]) -> np.ndarray:
        """Reconstruct 3D voxel array from subblocks."""
        x_dim, y_dim, z_dim = dimensions
        voxels = np.full((x_dim, y_dim, z_dim), ' ', dtype=str)
        
        for subblock in subblocks:
            x_start = subblock['x']
            y_start = subblock['y']
            z_start = subblock['z']
            
            x_end = x_start + subblock['length']
            y_end = y_start + subblock['width']
            z_end = z_start + subblock['height']
            
            # Ensure bounds
            x_end = min(x_end, x_dim)
            y_end = min(y_end, y_dim)
            z_end = min(z_end, z_dim)
            
            material_name = subblock['material']
            
            # Convert material name back to character
            material_map = {
                'ore': 'o',
                'titanium': 't',
                'sulfur': 's',
                'nickel': 'n'
            }
            material = material_map.get(material_name, material_name[0] if material_name else 'o')
            
            for x in range(x_start, x_end):
                for y in range(y_start, y_end):
                    for z in range(z_start, z_end):
                        if 0 <= x < x_dim and 0 <= y < y_dim and 0 <= z < z_dim:
                            voxels[x, y, z] = material
        
        return voxels
    
    def voxels_to_mesh(self, voxels: np.ndarray, voxel_size: float = 1.0) -> object:
        """Convert voxel array to 3D mesh."""
        if not TRIMESH_AVAILABLE:
            print("trimesh not available for mesh generation")
            return None
        
        # Create mesh from voxels
        mesh_parts = []
        
        for x in range(voxels.shape[0]):
            for y in range(voxels.shape[1]):
                for z in range(voxels.shape[2]):
                    if voxels[x, y, z] != ' ':
                        # Create a cube at this position
                        cube = trimesh.creation.box(extents=[voxel_size, voxel_size, voxel_size])
                        cube.apply_translation([x * voxel_size, y * voxel_size, z * voxel_size])
                        
                        # Color based on material
                        material = voxels[x, y, z]
                        if material == 'o':
                            color = [139, 69, 19, 255]  # Brown for ore
                        elif material == 't':
                            color = [192, 192, 192, 255]  # Silver for titanium
                        elif material == 's':
                            color = [255, 255, 0, 255]  # Yellow for sulfur
                        elif material == 'n':
                            color = [128, 128, 128, 255]  # Gray for nickel
                        else:
                            color = [255, 255, 255, 255]  # White for unknown
                        
                        cube.visual.face_colors = color
                        mesh_parts.append(cube)
        
        if mesh_parts:
            combined_mesh = trimesh.util.concatenate(mesh_parts)
            return combined_mesh
        
        return None
    
    def save_as_obj(self, voxels: np.ndarray, output_file: str, voxel_size: float = 1.0):
        """Save voxels as OBJ file."""
        mesh = self.voxels_to_mesh(voxels, voxel_size)
        if mesh:
            mesh.export(output_file)
            print(f"Mesh saved to {output_file}")
        else:
            print("Failed to generate mesh")
    
    def save_as_text(self, voxels: np.ndarray, output_file: str):
        """Save voxels as text format."""
        result = []
        
        # Add dimensions
        x, y, z = voxels.shape
        result.append(f"{x},{y},{z},8,8,8")
        
        # Add material mapping
        materials = set()
        for x_i in range(x):
            for y_i in range(y):
                for z_i in range(z):
                    materials.add(voxels[x_i, y_i, z_i])
        
        material_names = {
            'o': 'ore',
            't': 'titanium',
            's': 'sulfur', 
            'n': 'nickel',
            ' ': 'air'
        }
        
        for material in sorted(materials):
            if material in material_names:
                result.append(f"{material},{material_names[material]}")
        
        result.append("")  # Empty line
        
        # Add voxel data
        for z_i in range(z):
            for y_i in range(y):
                row = ""
                for x_i in range(x):
                    row += voxels[x_i, y_i, z_i]
                result.append(row)
            result.append("")  # Empty line after each slice
        
        with open(output_file, 'w') as f:
            f.write('\n'.join(result))
        
        print(f"Voxel data saved to {output_file}")
    
    def analyze_compression(self, original_voxels: np.ndarray, subblocks: List[dict]) -> dict:
        """Analyze compression efficiency."""
        original_size = original_voxels.size
        compressed_blocks = len(subblocks)
        
        # Calculate total compressed size (assuming each subblock takes 7 values)
        compressed_size = compressed_blocks * 7  # x,y,z,l,w,h,material
        
        compression_ratio = original_size / compressed_size if compressed_size > 0 else 0
        
        # Analyze block size distribution
        block_sizes = [sb['length'] * sb['width'] * sb['height'] for sb in subblocks]
        
        analysis = {
            'original_voxels': original_size,
            'compressed_blocks': compressed_blocks,
            'compression_ratio': compression_ratio,
            'average_block_size': np.mean(block_sizes) if block_sizes else 0,
            'max_block_size': max(block_sizes) if block_sizes else 0,
            'min_block_size': min(block_sizes) if block_sizes else 0,
            'block_size_std': np.std(block_sizes) if block_sizes else 0
        }
        
        return analysis

def main():
    parser = argparse.ArgumentParser(description='Decompress block compression output')
    parser.add_argument('--input', '-i', required=True, help='Compressed input file')
    parser.add_argument('--output', '-o', help='Output file')
    parser.add_argument('--format', '-f', choices=['obj', 'text', 'analysis'], 
                       default='text', help='Output format')
    parser.add_argument('--dimensions', '-d', default='32,32,32',
                       help='Original dimensions (x,y,z)')
    parser.add_argument('--binary', '-b', action='store_true',
                       help='Input is binary format')
    parser.add_argument('--voxel-size', '-v', type=float, default=1.0,
                       help='Voxel size for mesh generation')
    
    args = parser.parse_args()
    
    decompressor = Decompressor()
    
    # Read input file
    if args.binary:
        with open(args.input, 'rb') as f:
            data = f.read()
        subblocks = decompressor.parse_binary_output(data)
    else:
        with open(args.input, 'r') as f:
            text_data = f.read()
        subblocks = decompressor.parse_subblock_output(text_data)
    
    print(f"Parsed {len(subblocks)} subblocks")
    
    # Parse dimensions
    dimensions = tuple(map(int, args.dimensions.split(',')))
    
    # Reconstruct voxels
    voxels = decompressor.reconstruct_voxels(subblocks, dimensions)
    
    # Generate output
    if not args.output:
        base_name = args.input.rsplit('.', 1)[0]
        if args.format == 'obj':
            args.output = f"{base_name}_reconstructed.obj"
        elif args.format == 'text':
            args.output = f"{base_name}_reconstructed.txt"
        else:
            args.output = f"{base_name}_analysis.json"
    
    if args.format == 'obj':
        decompressor.save_as_obj(voxels, args.output, args.voxel_size)
    elif args.format == 'text':
        decompressor.save_as_text(voxels, args.output)
    elif args.format == 'analysis':
        # For analysis, we need original data for comparison
        print("Analysis requires original voxel data for comparison")
        analysis = decompressor.analyze_compression(voxels, subblocks)
        
        with open(args.output, 'w') as f:
            json.dump(analysis, f, indent=2)
        
        print(f"Analysis saved to {args.output}")
        print(f"Compression ratio: {analysis['compression_ratio']:.2f}")
        print(f"Average block size: {analysis['average_block_size']:.2f}")

if __name__ == "__main__":
    main()
