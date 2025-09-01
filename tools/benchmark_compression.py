#!/usr/bin/env python3
"""
Compression Algorithm Benchmarking Suite
Compares different compression algorithms on various test datasets.
"""

import time
import subprocess
import os
import json
import argparse
import numpy as np
from typing import Dict, List, Tuple
import tempfile
import threading
import queue
from pathlib import Path

class CompressionBenchmark:
    def __init__(self, executable_path: str = "./myapp"):
        self.executable_path = executable_path
        self.results = []
        
    def run_compression_test(self, input_file: str, timeout: float = 30.0) -> Dict:
        """Run the compression algorithm on a test file and measure performance."""
        start_time = time.time()
        
        try:
            # Run the compression algorithm
            result = subprocess.run(
                [self.executable_path],
                input=open(input_file, 'r').read(),
                capture_output=True,
                text=True,
                timeout=timeout
            )
            
            end_time = time.time()
            processing_time = end_time - start_time
            
            # Get file sizes
            input_size = os.path.getsize(input_file)
            output_size = len(result.stdout.encode()) if result.stdout else 0
            
            compression_ratio = input_size / output_size if output_size > 0 else 0
            
            # Parse compression statistics from output if available
            compressed_blocks = result.stdout.count('\n') if result.stdout else 0
            
            return {
                'input_file': input_file,
                'input_size_bytes': input_size,
                'output_size_bytes': output_size,
                'processing_time_seconds': processing_time,
                'compression_ratio': compression_ratio,
                'compressed_blocks': compressed_blocks,
                'return_code': result.returncode,
                'stderr': result.stderr,
                'success': result.returncode == 0
            }
            
        except subprocess.TimeoutExpired:
            return {
                'input_file': input_file,
                'processing_time_seconds': timeout,
                'success': False,
                'error': 'Timeout'
            }
        except Exception as e:
            return {
                'input_file': input_file,
                'success': False,
                'error': str(e)
            }
    
    def generate_test_datasets(self, output_dir: str) -> List[str]:
        """Generate various test datasets for benchmarking."""
        os.makedirs(output_dir, exist_ok=True)
        test_files = []
        
        # Import the converter
        import sys
        sys.path.append('/Users/a1847007/Desktop/Block-compression-Maptek/tools')
        from model_to_text_converter import ModelToTextConverter
        
        converter = ModelToTextConverter()
        
        # Test case 1: Small uniform blocks (should compress well)
        print("Generating uniform blocks test...")
        voxels = converter.generate_test_data((16, 16, 16), 'blocks')
        text_data = converter.voxels_to_text_format(voxels)
        
        uniform_file = os.path.join(output_dir, "test_uniform_16x16x16.txt")
        with open(uniform_file, 'w') as f:
            f.write(text_data)
        test_files.append(uniform_file)
        
        # Test case 2: Random data (should compress poorly)
        print("Generating random data test...")
        voxels = converter.generate_test_data((16, 16, 16), 'random')
        text_data = converter.voxels_to_text_format(voxels)
        
        random_file = os.path.join(output_dir, "test_random_16x16x16.txt")
        with open(random_file, 'w') as f:
            f.write(text_data)
        test_files.append(random_file)
        
        # Test case 3: Layered data (medium compression)
        print("Generating layered data test...")
        voxels = converter.generate_test_data((16, 16, 16), 'layers')
        text_data = converter.voxels_to_text_format(voxels)
        
        layered_file = os.path.join(output_dir, "test_layered_16x16x16.txt")
        with open(layered_file, 'w') as f:
            f.write(text_data)
        test_files.append(layered_file)
        
        # Test case 4: Sphere (geometric compression)
        print("Generating sphere data test...")
        voxels = converter.generate_test_data((16, 16, 16), 'sphere')
        text_data = converter.voxels_to_text_format(voxels)
        
        sphere_file = os.path.join(output_dir, "test_sphere_16x16x16.txt")
        with open(sphere_file, 'w') as f:
            f.write(text_data)
        test_files.append(sphere_file)
        
        # Test case 5: Large dataset
        print("Generating large uniform test...")
        voxels = converter.generate_test_data((64, 64, 32), 'blocks')
        text_data = converter.voxels_to_text_format(voxels)
        
        large_file = os.path.join(output_dir, "test_large_64x64x32.txt")
        with open(large_file, 'w') as f:
            f.write(text_data)
        test_files.append(large_file)
        
        # Test case 6: Very large random (stress test)
        print("Generating large random test...")
        voxels = converter.generate_test_data((32, 32, 64), 'random')
        text_data = converter.voxels_to_text_format(voxels)
        
        large_random_file = os.path.join(output_dir, "test_large_random_32x32x64.txt")
        with open(large_random_file, 'w') as f:
            f.write(text_data)
        test_files.append(large_random_file)
        
        return test_files
    
    def run_benchmark_suite(self, test_files: List[str], iterations: int = 3) -> List[Dict]:
        """Run benchmark on all test files with multiple iterations."""
        all_results = []
        
        for test_file in test_files:
            print(f"\\nBenchmarking {os.path.basename(test_file)}...")
            
            file_results = []
            for i in range(iterations):
                print(f"  Iteration {i+1}/{iterations}...")
                result = self.run_compression_test(test_file)
                file_results.append(result)
                
                if result['success']:
                    print(f"    Time: {result['processing_time_seconds']:.3f}s, "
                          f"Ratio: {result['compression_ratio']:.2f}, "
                          f"Blocks: {result['compressed_blocks']}")
                else:
                    print(f"    Failed: {result.get('error', 'Unknown error')}")
            
            # Calculate statistics for this file
            successful_runs = [r for r in file_results if r['success']]
            
            if successful_runs:
                times = [r['processing_time_seconds'] for r in successful_runs]
                ratios = [r['compression_ratio'] for r in successful_runs]
                
                summary = {
                    'test_file': test_file,
                    'iterations': len(successful_runs),
                    'success_rate': len(successful_runs) / len(file_results),
                    'avg_time': np.mean(times),
                    'min_time': np.min(times),
                    'max_time': np.max(times),
                    'std_time': np.std(times),
                    'avg_compression_ratio': np.mean(ratios),
                    'input_size': successful_runs[0]['input_size_bytes'],
                    'avg_output_size': np.mean([r['output_size_bytes'] for r in successful_runs]),
                    'all_runs': file_results
                }
            else:
                summary = {
                    'test_file': test_file,
                    'iterations': 0,
                    'success_rate': 0,
                    'error': 'All runs failed',
                    'all_runs': file_results
                }
            
            all_results.append(summary)
        
        return all_results
    
    def generate_report(self, results: List[Dict], output_file: str):
        """Generate a comprehensive benchmark report."""
        report = {
            'benchmark_timestamp': time.strftime('%Y-%m-%d %H:%M:%S'),
            'executable': self.executable_path,
            'total_tests': len(results),
            'results': results
        }
        
        # Calculate overall statistics
        successful_tests = [r for r in results if r.get('success_rate', 0) > 0]
        
        if successful_tests:
            report['summary'] = {
                'successful_tests': len(successful_tests),
                'overall_success_rate': len(successful_tests) / len(results),
                'fastest_test': min(successful_tests, key=lambda x: x['avg_time'])['test_file'],
                'fastest_time': min(r['avg_time'] for r in successful_tests),
                'slowest_test': max(successful_tests, key=lambda x: x['avg_time'])['test_file'],
                'slowest_time': max(r['avg_time'] for r in successful_tests),
                'best_compression': max(r['avg_compression_ratio'] for r in successful_tests),
                'worst_compression': min(r['avg_compression_ratio'] for r in successful_tests),
                'avg_compression_ratio': np.mean([r['avg_compression_ratio'] for r in successful_tests])
            }
        
        # Save report
        with open(output_file, 'w') as f:
            json.dump(report, f, indent=2)
        
        # Print summary to console
        print(f"\\n{'='*60}")
        print("BENCHMARK RESULTS SUMMARY")
        print(f"{'='*60}")
        
        if successful_tests:
            print(f"Successful tests: {len(successful_tests)}/{len(results)}")
            print(f"Fastest: {report['summary']['fastest_time']:.3f}s "
                  f"({os.path.basename(report['summary']['fastest_test'])})")
            print(f"Slowest: {report['summary']['slowest_time']:.3f}s "
                  f"({os.path.basename(report['summary']['slowest_test'])})")
            print(f"Average compression ratio: {report['summary']['avg_compression_ratio']:.2f}")
            print(f"Best compression ratio: {report['summary']['best_compression']:.2f}")
            print(f"Worst compression ratio: {report['summary']['worst_compression']:.2f}")
        else:
            print("No successful tests!")
        
        print(f"\\nDetailed report saved to: {output_file}")
    
    def compare_algorithms(self, algorithm_configs: List[Dict]) -> Dict:
        """Compare multiple algorithm configurations."""
        # This would be extended to test different algorithm implementations
        # For now, it's a placeholder for future algorithm variants
        pass

class AlternativeAlgorithms:
    """Implementation of alternative compression algorithms for comparison."""
    
    @staticmethod
    def run_length_encoding(voxels: np.ndarray) -> List[Tuple]:
        """Simple run-length encoding compression."""
        compressed = []
        
        # Flatten the 3D array
        flat = voxels.flatten()
        
        if len(flat) == 0:
            return compressed
        
        current_char = flat[0]
        count = 1
        
        for i in range(1, len(flat)):
            if flat[i] == current_char:
                count += 1
            else:
                compressed.append((current_char, count))
                current_char = flat[i]
                count = 1
        
        compressed.append((current_char, count))
        return compressed
    
    @staticmethod
    def octree_compression(voxels: np.ndarray, max_depth: int = 6) -> Dict:
        """Octree-based compression (simplified implementation)."""
        def is_uniform(arr):
            return np.all(arr == arr.flat[0])
        
        def compress_octree(arr, depth=0):
            if depth >= max_depth or arr.size <= 8 or is_uniform(arr):
                return {'type': 'leaf', 'value': arr.flat[0], 'size': arr.size}
            
            # Split into 8 octants
            mid_x, mid_y, mid_z = arr.shape[0]//2, arr.shape[1]//2, arr.shape[2]//2
            
            octants = []
            for x_slice in [slice(0, mid_x), slice(mid_x, None)]:
                for y_slice in [slice(0, mid_y), slice(mid_y, None)]:
                    for z_slice in [slice(0, mid_z), slice(mid_z, None)]:
                        octant = arr[x_slice, y_slice, z_slice]
                        if octant.size > 0:
                            octants.append(compress_octree(octant, depth + 1))
            
            return {'type': 'node', 'children': octants}
        
        return compress_octree(voxels)

def main():
    parser = argparse.ArgumentParser(description='Benchmark compression algorithms')
    parser.add_argument('--executable', '-e', default='./myapp',
                       help='Path to compression executable')
    parser.add_argument('--output-dir', '-o', default='./benchmark_results',
                       help='Output directory for results')
    parser.add_argument('--test-dir', '-t', default='./test_data',
                       help='Directory for test data generation')
    parser.add_argument('--iterations', '-i', type=int, default=3,
                       help='Number of iterations per test')
    parser.add_argument('--generate-only', '-g', action='store_true',
                       help='Only generate test data, skip benchmarking')
    parser.add_argument('--existing-tests', '-x', 
                       help='Use existing test files (comma-separated list)')
    
    args = parser.parse_args()
    
    # Create output directory
    os.makedirs(args.output_dir, exist_ok=True)
    
    benchmark = CompressionBenchmark(args.executable)
    
    # Generate or use existing test data
    if args.existing_tests:
        test_files = [f.strip() for f in args.existing_tests.split(',')]
        print(f"Using existing test files: {test_files}")
    else:
        print("Generating test datasets...")
        test_files = benchmark.generate_test_datasets(args.test_dir)
        print(f"Generated {len(test_files)} test files")
    
    if args.generate_only:
        print("Test data generation complete. Exiting.")
        return
    
    # Check if executable exists
    if not os.path.exists(args.executable):
        print(f"Error: Executable {args.executable} not found!")
        print("Make sure to compile the project first with 'make'")
        return
    
    # Run benchmarks
    print(f"\\nRunning benchmark suite...")
    results = benchmark.run_benchmark_suite(test_files, args.iterations)
    
    # Generate report
    report_file = os.path.join(args.output_dir, 'benchmark_report.json')
    benchmark.generate_report(results, report_file)

if __name__ == "__main__":
    main()
