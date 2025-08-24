# 3D Model Compression Testing Suite

This suite provides comprehensive tools for testing and benchmarking your 3D block compression algorithm.

## Setup

### 1. Install Python Dependencies

```bash
cd tools
pip install -r requirements.txt
```

### 2. Compile the C++ Project

```bash
cd ./Block-compression-Maptek
make clean
make
```

### 3. Make Tools Executable

```bash
chmod +x tools/*.py
```

## Usage Guide

### 1. Generate Test Data

#### Generate synthetic test datasets:
```bash
python tools/model_to_text_converter.py --generate --pattern blocks --dimensions 32,32,32 --output test_blocks_32x32x32.txt
python tools/model_to_text_converter.py --generate --pattern random --dimensions 16,16,16 --output test_random_16x16x16.txt
python tools/model_to_text_converter.py --generate --pattern layers --dimensions 64,32,16 --output test_layers_64x32x16.txt
python tools/model_to_text_converter.py --generate --pattern sphere --dimensions 24,24,24 --output test_sphere_24x24x24.txt
```

#### Convert 3D model files (if you have .obj, .ply, .stl files):
```bash
python tools/model_to_text_converter.py --input model.obj --output model_voxels.txt --resolution 32 --material layered
```

### 2. Test Individual Files

Run compression on a single test file:
```bash
./myapp < test_blocks_32x32x32.txt > output_blocks.txt
```

### 3. Run Comprehensive Benchmarks

#### Generate test data and run full benchmark suite:
```bash
python tools/benchmark_compression.py --executable ./myapp --iterations 5 --output-dir benchmark_results
```

#### Run benchmarks on existing test files:
```bash
python tools/benchmark_compression.py --existing-tests "test_input.txt,test_input_random_large.txt" --iterations 3
```

### 4. Analyze Results and Convert Back to 3D

#### Decompress and analyze compression output:
```bash
python tools/decompressor.py --input output_blocks.txt --format analysis --dimensions 32,32,32
python tools/decompressor.py --input output_blocks.txt --format text --output reconstructed.txt
python tools/decompressor.py --input output_blocks.txt --format obj --output reconstructed.obj
```

## Understanding the Results

### Benchmark Metrics

- **Processing Time**: How long compression takes
- **Compression Ratio**: `original_size / compressed_size`
- **Success Rate**: Percentage of successful compression runs
- **Block Count**: Number of compressed subblocks generated

### Test Patterns

- **Blocks**: Large uniform regions (high compression ratio expected)
- **Random**: Random voxel assignment (low compression ratio expected)
- **Layers**: Horizontal material layers (medium compression)
- **Sphere**: Geometric shape (geometric compression patterns)

### Performance Analysis

The benchmark suite generates JSON reports with:
- Individual test results
- Statistical summaries
- Performance comparisons
- Compression efficiency metrics

## Comparing Algorithms

### Current Algorithm Analysis

Your current algorithm uses rectangular block detection:
1. Finds largest uniform rectangular regions
2. Stores position, dimensions, and material
3. Works well for: geometric shapes, layered data, uniform blocks
4. Struggles with: highly random data, noise

### Alternative Algorithm Ideas

Based on the benchmarking results, you might consider:

1. **Hybrid Approach**: Use block compression for uniform areas, run-length encoding for others
2. **Octree Compression**: Better for sparse or hierarchical data
3. **Predictive Coding**: Use neighboring voxel values to predict and compress
4. **Hierarchical Blocks**: Multi-level block sizes for better adaptation

### Speed Optimization

If your current algorithm is too slow:

1. **Parallel Processing**: Use OpenMP to parallelize block detection
2. **Early Termination**: Stop searching when blocks get too small
3. **Memory Optimization**: Reduce memory allocations in hot paths
4. **Algorithm Pruning**: Skip regions that are unlikely to compress well

## File Structure

```
Block-compression-Maptek/
├── tools/
│   ├── model_to_text_converter.py  # 3D model → text format
│   ├── decompressor.py             # Compressed data → 3D model
│   ├── benchmark_compression.py    # Performance testing suite
│   ├── requirements.txt            # Python dependencies
│   └── setup_and_usage.md         # This guide
├── test_data/                     # Generated test files
├── benchmark_results/             # Benchmark output
└── ... (your existing C++ code)
```

## Next Steps

1. **Run Initial Benchmarks**: Start with the basic benchmark suite
2. **Analyze Bottlenecks**: Look at timing and compression ratio results
3. **Generate Specific Test Cases**: Create data that matches your real-world use cases
4. **Implement Alternatives**: Try different algorithms on problematic cases
5. **Optimize**: Focus on the biggest performance/compression issues

## Troubleshooting

### Common Issues

1. **"myapp not found"**: Make sure you compiled with `make`
2. **"trimesh not available"**: Install with `pip install trimesh`
3. **Benchmark timeouts**: Reduce test data size or increase timeout
4. **Memory issues**: Use smaller test datasets initially

### Debug Mode

Add debug output to your C++ code to trace compression decisions:
```cpp
printf("Processing block at (%d,%d,%d) size (%d,%d,%d) material %c\\n", 
       x, y, z, maxX-x, maxY-y, maxZ-z, target);
```
