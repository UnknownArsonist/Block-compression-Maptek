#!/usr/bin/env python3
"""
Simple Compression Benchmarker
Performance testing focused purely on compression metrics and timing.
No 3D dependencies - lightweight and fast.
"""

import time
import subprocess
import os
import json
import argparse
import numpy as np
from typing import Dict, List
import tempfile
from pathlib import Path

def convert_numpy_types(obj):
    """Convert numpy types to native Python types for JSON serialization."""
    if isinstance(obj, np.integer):
        return int(obj)
    elif isinstance(obj, np.floating):
        return float(obj)
    elif isinstance(obj, np.ndarray):
        return obj.tolist()
    raise TypeError(f'Object of type {obj.__class__.__name__} is not JSON serializable')

class SimpleBenchmark:
    def __init__(self, executable_path: str = "./myapp"):
        self.executable_path = executable_path
        
    def run_compression_test(self, input_file: str, timeout: float = 30.0) -> Dict:
        """Run compression test and measure performance."""
        start_time = time.time()
        
        try:
            # Run the compression algorithm
            with open(input_file, 'r') as f:
                input_data = f.read()
            
            result = subprocess.run(
                [self.executable_path],
                input=input_data,
                capture_output=True,
                text=True,
                timeout=timeout
            )
            
            end_time = time.time()
            processing_time = end_time - start_time
            
            # Calculate metrics
            input_size = len(input_data.encode())
            output_size = len(result.stdout.encode()) if result.stdout else 0
            compression_ratio = input_size / output_size if output_size > 0 else 0
            compressed_blocks = result.stdout.count('\n') if result.stdout else 0
            
            # Get dimensions from input file
            lines = input_data.strip().split('\n')
            dimensions = tuple(map(int, lines[0].split(',')[:3])) if lines else (0, 0, 0)
            original_voxels = np.prod(dimensions)
            
            return {
                'input_file': input_file,
                'dimensions': dimensions,
                'input_size_bytes': input_size,
                'output_size_bytes': output_size,
                'processing_time_seconds': processing_time,
                'compression_ratio': compression_ratio,
                'compressed_blocks': compressed_blocks,
                'original_voxels': original_voxels,
                'voxels_per_second': original_voxels / processing_time if processing_time > 0 else 0,
                'return_code': result.returncode,
                'success': result.returncode == 0,
                'output': result.stdout if result.returncode == 0 else None,
                'error': result.stderr if result.stderr else None
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
    
    def generate_test_suite(self, output_dir: str) -> List[str]:
        """Generate a comprehensive test suite."""
        os.makedirs(output_dir, exist_ok=True)
        test_files = []
        
        # Import the simple test generator
        import sys
        sys.path.append(os.path.dirname(__file__))
        from simple_test_generator import SimpleTestGenerator
        
        generator = SimpleTestGenerator()
        
        # Test scenarios
        scenarios = [
            # Small tests for quick validation
            ('uniform_small', 'blocks', (16, 16, 16)),
            ('random_small', 'random', (16, 16, 16)),
            ('layers_small', 'layers', (16, 16, 16)),
            ('mixed_small', 'mixed', (16, 16, 16)),
            
            # Medium tests for performance analysis
            ('uniform_medium', 'blocks', (32, 32, 32)),
            ('random_medium', 'random', (32, 32, 32)),
            ('layers_medium', 'layers', (32, 32, 32)),
            ('sphere_medium', 'sphere', (32, 32, 32)),
            
            # Large tests for scalability
            ('uniform_large', 'blocks', (64, 32, 32)),
            ('random_large', 'random', (32, 32, 64)),
            
            # Edge cases
            ('thin_layer', 'layers', (64, 64, 8)),
            ('tall_column', 'blocks', (16, 16, 64))
        ]
        
        print(f"Generating {len(scenarios)} test scenarios...")
        
        for name, pattern, dimensions in scenarios:
            filename = os.path.join(output_dir, f"{name}_{dimensions[0]}x{dimensions[1]}x{dimensions[2]}.txt")
            
            print(f"  {name}: {pattern} pattern, {dimensions}")
            voxels = generator.generate_test_data(dimensions, pattern)
            text_data = generator.voxels_to_text_format(voxels)
            
            with open(filename, 'w') as f:
                f.write(text_data)
            
            test_files.append(filename)
        
        print(f"Generated {len(test_files)} test files in {output_dir}")
        return test_files
    
    def run_benchmark_suite(self, test_files: List[str], iterations: int = 3) -> List[Dict]:
        """Run benchmark on all test files with multiple iterations."""
        all_results = []
        
        print(f"\nRunning benchmark on {len(test_files)} files with {iterations} iterations each...")
        
        for i, test_file in enumerate(test_files, 1):
            file_name = os.path.basename(test_file)
            print(f"\n[{i}/{len(test_files)}] Benchmarking {file_name}...")
            
            file_results = []
            for iteration in range(iterations):
                print(f"  Iteration {iteration+1}/{iterations}...", end=' ')
                result = self.run_compression_test(test_file)
                file_results.append(result)
                
                if result['success']:
                    print(f"✓ {result['processing_time_seconds']:.3f}s, {result['compression_ratio']:.2f}:1, {result['compressed_blocks']} blocks")
                else:
                    print(f"✗ {result.get('error', 'Unknown error')}")
            
            # Calculate statistics for this file
            successful_runs = [r for r in file_results if r['success']]
            
            if successful_runs:
                times = [r['processing_time_seconds'] for r in successful_runs]
                ratios = [r['compression_ratio'] for r in successful_runs]
                vps = [r['voxels_per_second'] for r in successful_runs]
                
                summary = {
                    'test_file': test_file,
                    'test_name': file_name,
                    'iterations': len(successful_runs),
                    'success_rate': len(successful_runs) / len(file_results),
                    'dimensions': successful_runs[0]['dimensions'],
                    'original_voxels': successful_runs[0]['original_voxels'],
                    'timing': {
                        'avg_time': np.mean(times),
                        'min_time': np.min(times),
                        'max_time': np.max(times),
                        'std_time': np.std(times)
                    },
                    'compression': {
                        'avg_ratio': np.mean(ratios),
                        'min_ratio': np.min(ratios),
                        'max_ratio': np.max(ratios),
                        'avg_blocks': np.mean([r['compressed_blocks'] for r in successful_runs])
                    },
                    'performance': {
                        'avg_voxels_per_second': np.mean(vps),
                        'max_voxels_per_second': np.max(vps)
                    },
                    'all_runs': file_results
                }
            else:
                summary = {
                    'test_file': test_file,
                    'test_name': file_name,
                    'iterations': 0,
                    'success_rate': 0,
                    'error': 'All runs failed',
                    'all_runs': file_results
                }
            
            all_results.append(summary)
        
        return all_results
    
    def generate_report(self, results: List[Dict], output_file: str):
        """Generate a comprehensive benchmark report."""
        successful_tests = [r for r in results if r.get('success_rate', 0) > 0]
        
        report = {
            'benchmark_timestamp': time.strftime('%Y-%m-%d %H:%M:%S'),
            'executable': self.executable_path,
            'total_tests': len(results),
            'successful_tests': len(successful_tests),
            'overall_success_rate': len(successful_tests) / len(results) if results else 0,
            'results': results
        }
        
        if successful_tests:
            # Overall statistics
            all_times = []
            all_ratios = []
            all_vps = []
            
            for test in successful_tests:
                if 'timing' in test:
                    all_times.append(test['timing']['avg_time'])
                    all_ratios.append(test['compression']['avg_ratio'])
                    all_vps.append(test['performance']['avg_voxels_per_second'])
            
            if all_times:
                report['summary'] = {
                    'fastest_test': min(successful_tests, key=lambda x: x.get('timing', {}).get('avg_time', float('inf')))['test_name'],
                    'slowest_test': max(successful_tests, key=lambda x: x.get('timing', {}).get('avg_time', 0))['test_name'],
                    'best_compression_test': max(successful_tests, key=lambda x: x.get('compression', {}).get('avg_ratio', 0))['test_name'],
                    'worst_compression_test': min(successful_tests, key=lambda x: x.get('compression', {}).get('avg_ratio', float('inf')))['test_name'],
                    'overall_stats': {
                        'avg_time': np.mean(all_times),
                        'avg_compression_ratio': np.mean(all_ratios),
                        'avg_voxels_per_second': np.mean(all_vps),
                        'fastest_time': np.min(all_times),
                        'best_compression': np.max(all_ratios),
                        'highest_throughput': np.max(all_vps)
                    }
                }
        
        # Save report
        with open(output_file, 'w') as f:
            json.dump(report, f, indent=2, default=convert_numpy_types)
        
        # Print summary
        self._print_summary(report)
    
    def _print_summary(self, report: Dict):
        """Print a console summary of the benchmark results."""
        print(f"\n{'='*80}")
        print("BENCHMARK RESULTS SUMMARY")
        print(f"{'='*80}")
        
        print(f"Timestamp: {report['benchmark_timestamp']}")
        print(f"Executable: {report['executable']}")
        print(f"Tests run: {report['total_tests']}")
        print(f"Successful: {report['successful_tests']} ({report['overall_success_rate']:.1%})")
        
        if 'summary' in report:
            s = report['summary']
            stats = s['overall_stats']
            
            print(f"\nPerformance Overview:")
            print(f"  Average processing time: {stats['avg_time']:.3f}s")
            print(f"  Fastest test: {stats['fastest_time']:.3f}s ({s['fastest_test']})")
            print(f"  Average throughput: {stats['avg_voxels_per_second']:,.0f} voxels/sec")
            print(f"  Peak throughput: {stats['highest_throughput']:,.0f} voxels/sec")
            
            print(f"\nCompression Overview:")
            print(f"  Average compression ratio: {stats['avg_compression_ratio']:.2f}:1")
            print(f"  Best compression: {stats['best_compression']:.2f}:1 ({s['best_compression_test']})")
            print(f"  Worst compression: from {s['worst_compression_test']}")
            
            print(f"\nTest Categories:")
            for result in report['results']:
                if result.get('success_rate', 0) > 0:
                    name = result['test_name']
                    timing = result.get('timing', {})
                    compression = result.get('compression', {})
                    print(f"  {name:30s} | {timing.get('avg_time', 0):.3f}s | {compression.get('avg_ratio', 0):.2f}:1")
        
        print(f"\nDetailed report saved to the specified JSON file")

def main():
    parser = argparse.ArgumentParser(description='Benchmark compression performance')
    parser.add_argument('--executable', '-e', default='./myapp', help='Path to compression executable')
    parser.add_argument('--output-dir', '-o', default='./simple_benchmark_results', help='Output directory for results')
    parser.add_argument('--test-dir', '-t', default='./simple_test_data', help='Directory for generated test data')
    parser.add_argument('--iterations', '-i', type=int, default=3, help='Number of iterations per test')
    parser.add_argument('--existing-tests', '-x', help='Use existing test files (comma-separated list)')
    parser.add_argument('--generate-only', '-g', action='store_true', help='Only generate test data, skip benchmarking')
    
    args = parser.parse_args()
    
    # Create output directory
    os.makedirs(args.output_dir, exist_ok=True)
    
    benchmark = SimpleBenchmark(args.executable)
    
    # Generate or use existing test data
    if args.existing_tests:
        test_files = [f.strip() for f in args.existing_tests.split(',')]
        print(f"Using existing test files: {test_files}")
    else:
        print("Generating test suite...")
        test_files = benchmark.generate_test_suite(args.test_dir)
    
    if args.generate_only:
        print("Test data generation complete.")
        return
    
    # Check if executable exists
    if not os.path.exists(args.executable):
        print(f"Error: Executable {args.executable} not found!")
        print("Make sure to compile the project first with 'make'")
        return
    
    # Run benchmarks
    results = benchmark.run_benchmark_suite(test_files, args.iterations)
    
    # Generate report
    report_file = os.path.join(args.output_dir, 'benchmark_report.json')
    benchmark.generate_report(results, report_file)

if __name__ == "__main__":
    main()
