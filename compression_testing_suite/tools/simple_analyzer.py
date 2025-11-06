#!/usr/bin/env python3
"""
Simple Compression Analyzer
Analyzes compressed output without 3D visualization dependencies.
Focuses purely on compression metrics and performance analysis.
"""

import json
import argparse
import numpy as np
from typing import List, Dict, Tuple

def convert_numpy_types(obj):
    """Convert numpy types to native Python types for JSON serialization."""
    if isinstance(obj, np.integer):
        return int(obj)
    elif isinstance(obj, np.floating):
        return float(obj)
    elif isinstance(obj, np.ndarray):
        return obj.tolist()
    raise TypeError(f'Object of type {obj.__class__.__name__} is not JSON serializable')

class SimpleAnalyzer:
    def __init__(self):
        pass
    
    def parse_compressed_output(self, output_text: str) -> List[dict]:
        """Parse the compressed output format into block data."""
        blocks = []
        
        lines = output_text.strip().split('\n')
        for line_num, line in enumerate(lines, 1):
            if not line.strip():
                continue
            
            # Expected format: x,y,z,length,width,height,material_name
            parts = line.split(',')
            if len(parts) >= 7:
                try:
                    block = {
                        'x': int(parts[0]),
                        'y': int(parts[1]),
                        'z': int(parts[2]),
                        'length': int(parts[3]),
                        'width': int(parts[4]),
                        'height': int(parts[5]),
                        'material': parts[6],
                        'volume': int(parts[3]) * int(parts[4]) * int(parts[5])
                    }
                    blocks.append(block)
                except ValueError as e:
                    print(f"Warning: Error parsing line {line_num}: {line} - {e}")
            else:
                print(f"Warning: Invalid format on line {line_num}: {line}")
        
        return blocks
    
    def analyze_compression(self, blocks: List[dict], original_dimensions: Tuple[int, int, int]) -> Dict:
        """Analyze compression effectiveness."""
        if not blocks:
            return {
                'error': 'No blocks found in compressed data',
                'original_voxels': np.prod(original_dimensions),
                'compressed_blocks': 0
            }
        
        original_voxels = np.prod(original_dimensions)
        compressed_blocks = len(blocks)
        
        # Calculate volumes
        block_volumes = [block['volume'] for block in blocks]
        total_compressed_volume = sum(block_volumes)
        
        # Calculate block size statistics
        avg_block_size = np.mean(block_volumes) if block_volumes else 0
        max_block_size = max(block_volumes) if block_volumes else 0
        min_block_size = min(block_volumes) if block_volumes else 0
        std_block_size = np.std(block_volumes) if block_volumes else 0
        
        # Compression ratio (higher is better)
        compression_ratio = original_voxels / compressed_blocks if compressed_blocks > 0 else 0
        
        # Volume efficiency (should be close to 1.0 for lossless)
        volume_efficiency = total_compressed_volume / original_voxels if original_voxels > 0 else 0
        
        # Material distribution
        materials = {}
        for block in blocks:
            material = block['material']
            if material not in materials:
                materials[material] = {'count': 0, 'volume': 0}
            materials[material]['count'] += 1
            materials[material]['volume'] += block['volume']
        
        # Block size distribution
        size_distribution = {
            'small_blocks': len([b for b in block_volumes if b <= 8]),
            'medium_blocks': len([b for b in block_volumes if 8 < b <= 64]),
            'large_blocks': len([b for b in block_volumes if b > 64])
        }
        
        analysis = {
            'dimensions': original_dimensions,
            'original_voxels': original_voxels,
            'compressed_blocks': compressed_blocks,
            'compression_ratio': compression_ratio,
            'volume_efficiency': volume_efficiency,
            'block_statistics': {
                'average_size': avg_block_size,
                'max_size': max_block_size,
                'min_size': min_block_size,
                'std_deviation': std_block_size
            },
            'material_distribution': materials,
            'size_distribution': size_distribution,
            'compression_quality': self._assess_quality(compression_ratio, volume_efficiency)
        }
        
        return analysis
    
    def _assess_quality(self, compression_ratio: float, volume_efficiency: float) -> str:
        """Assess the quality of compression based on metrics."""
        if compression_ratio >= 3.0 and volume_efficiency >= 0.99:
            return "Excellent"
        elif compression_ratio >= 2.0 and volume_efficiency >= 0.95:
            return "Good"
        elif compression_ratio >= 1.0 and volume_efficiency >= 0.90:
            return "Fair"
        else:
            return "Poor"
    
    def compare_original_vs_compressed(self, original_file: str, compressed_file: str) -> Dict:
        """Compare original input file with compressed output."""
        # Get original file info
        original_size = 0
        dimensions = None
        
        try:
            with open(original_file, 'r') as f:
                lines = f.readlines()
                original_size = sum(len(line.encode()) for line in lines)
                
                # Parse dimensions from first line
                header = lines[0].strip()
                dimensions = tuple(map(int, header.split(',')[:3]))
        except Exception as e:
            return {'error': f"Failed to read original file: {e}"}
        
        # Get compressed file info
        compressed_size = 0
        blocks = []
        
        try:
            with open(compressed_file, 'r') as f:
                compressed_text = f.read()
                compressed_size = len(compressed_text.encode())
                blocks = self.parse_compressed_output(compressed_text)
        except Exception as e:
            return {'error': f"Failed to read compressed file: {e}"}
        
        # Analyze compression
        analysis = self.analyze_compression(blocks, dimensions)
        
        # Add file size comparison
        analysis['file_sizes'] = {
            'original_bytes': original_size,
            'compressed_bytes': compressed_size,
            'file_compression_ratio': original_size / compressed_size if compressed_size > 0 else 0
        }
        
        return analysis
    
    def print_analysis_report(self, analysis: Dict):
        """Print a human-readable analysis report."""
        if 'error' in analysis:
            print(f"Error: {analysis['error']}")
            return
        
        print("=" * 60)
        print("COMPRESSION ANALYSIS REPORT")
        print("=" * 60)
        
        # Basic metrics
        print(f"Original dimensions: {analysis['dimensions']}")
        print(f"Original voxels: {analysis['original_voxels']:,}")
        print(f"Compressed blocks: {analysis['compressed_blocks']:,}")
        print(f"Compression ratio: {analysis['compression_ratio']:.2f}:1")
        print(f"Volume efficiency: {analysis['volume_efficiency']:.1%}")
        print(f"Overall quality: {analysis['compression_quality']}")
        
        # File sizes (if available)
        if 'file_sizes' in analysis:
            fs = analysis['file_sizes']
            print(f"\nFile size comparison:")
            print(f"  Original: {fs['original_bytes']:,} bytes")
            print(f"  Compressed: {fs['compressed_bytes']:,} bytes")
            print(f"  File compression: {fs['file_compression_ratio']:.2f}:1")
        
        # Block statistics
        bs = analysis['block_statistics']
        print(f"\nBlock statistics:")
        print(f"  Average block size: {bs['average_size']:.1f} voxels")
        print(f"  Largest block: {bs['max_size']} voxels")
        print(f"  Smallest block: {bs['min_size']} voxels")
        print(f"  Size std deviation: {bs['std_deviation']:.1f}")
        
        # Size distribution
        sd = analysis['size_distribution']
        print(f"\nBlock size distribution:")
        print(f"  Small (≤8): {sd['small_blocks']} blocks")
        print(f"  Medium (9-64): {sd['medium_blocks']} blocks")
        print(f"  Large (>64): {sd['large_blocks']} blocks")
        
        # Material distribution
        print(f"\nMaterial distribution:")
        for material, info in analysis['material_distribution'].items():
            percentage = (info['volume'] / analysis['original_voxels']) * 100
            print(f"  {material}: {info['count']} blocks, {info['volume']:,} voxels ({percentage:.1f}%)")

def main():
    parser = argparse.ArgumentParser(description='Analyze compression results')
    parser.add_argument('--compressed', '-c', required=True, help='Compressed output file')
    parser.add_argument('--original', '-o', help='Original input file (for comparison)')
    parser.add_argument('--dimensions', '-d', help='Original dimensions (x,y,z) if no original file')
    parser.add_argument('--output', '-j', help='Save analysis as JSON file')
    parser.add_argument('--quiet', '-q', action='store_true', help='Only output JSON, no console report')
    
    args = parser.parse_args()
    
    analyzer = SimpleAnalyzer()
    
    if args.original:
        # Compare with original file
        analysis = analyzer.compare_original_vs_compressed(args.original, args.compressed)
    elif args.dimensions:
        # Analyze compressed file with provided dimensions
        dimensions = tuple(map(int, args.dimensions.split(',')))
        try:
            with open(args.compressed, 'r') as f:
                compressed_text = f.read()
            blocks = analyzer.parse_compressed_output(compressed_text)
            analysis = analyzer.analyze_compression(blocks, dimensions)
        except Exception as e:
            analysis = {'error': f"Failed to analyze: {e}"}
    else:
        print("Error: Must provide either --original file or --dimensions")
        return
    
    # Save JSON output if requested
    if args.output:
        with open(args.output, 'w') as f:
            json.dump(analysis, f, indent=2, default=convert_numpy_types)
        if not args.quiet:
            print(f"Analysis saved to {args.output}")
    
    # Print report unless quiet mode
    if not args.quiet:
        analyzer.print_analysis_report(analysis)

if __name__ == "__main__":
    main()
