# The Block Model Compression Algorithm

## Description

The Block Model Compression Algorithm is designed to reduce the size of geological block models while preserving critical spatial and compositional information. It works by identifying patterns and redundancies in the data, then compressing blocks of similar attributes together. This reduces storage requirements and speeds up data processing and transmission, while maintaining the accuracy of geological features for analysis, visualization, and modeling.

## Setup Instructions

### Prerequisites

- **C++ Compiler** (g++ recommended)
- **Make** (for Linux/macOS builds)
- **Git** (optional, for cloning the repository)

### Unix (with `make`)

1. Building The Program:
   ```bash
      make
   ```

### Windows

1. Open a command prompt in the project directory.

2. You can compile and use the single_file.cpp which uses OpenMP(https://www.openmp.org/)
   ```bash
     g++ -Wall -static -fopenmp single_file.cpp -o single_file.exe
   ```

3. Alternatively you can Compile the less-optimised OOP version using the C++ thread library:
   ```bash
     g++ -Wall -Iinclude -pthread -static -o myapp.exe src/Compressor.cpp src/DisplayOutput.cpp src/StreamProcessor.cpp src/StreamBuffer.cpp src/OctTreeNode.cpp src/InputStreamReader.cpp src/ProcessorModule.cpp src/main.cpp
   ```

### Building the Test Program

1. Linux/macOS:
   ```bash
      make win-test
   ```
2. Windows:
   ```bash
      g++ -Wall -Iinclude -pthread -static -o myapp_test.exe src/Compressor.cpp src/DisplayOutput.cpp src/StreamProcessor.cpp src/StreamBuffer.cpp src/OctTreeNode.cpp src/InputStreamReader.cpp src/ProcessorModule.cpp src/test.cpp
   ```

---

## Software Architecture

![Alt text](src/download.png)

---

## Features

- **InputStreamReader**: Parses input files and sends the data to the Compressor module for processing.
- **Compressor Algorithms**: Receives raw data from StreamProcessor and performs lossless compression to reduce data size while preserving all information.
- **Testing Tools**: Measures the compression rate and speed of the program.
- **Modularity**: Each component is implemented using OOP, making the system easier to scale, maintain, and modify.
- **Multi-Threading**: Enables concurrent processing of multiple tasks, improving performance and responsiveness of the application.

---

## Known Bugs & Limitations

- **No GUI Interface**: The application currently runs via the command line only.



# 🎯 Simple Compression Testing Suite

Streamlined version focused purely on compression analysis. No 3D visualization dependencies.

## 🚀 Quick Start

### 1. Setup
```bash
# Compile compression algorithm
make clean && make

# Install minimal dependencies
pip install numpy

# Make demo executable
chmod +x simple_demo.sh
```

### 2. Run Everything
```bash
./simple_demo.sh
```

## 📋 Individual Tools

### Generate Test Data
```bash
# Different patterns
python3 tools/simple_test_generator.py --pattern blocks --dimensions 32,32,32 --output test_blocks.txt
python3 tools/simple_test_generator.py --pattern random --dimensions 32,32,32 --output test_random.txt
python3 tools/simple_test_generator.py --pattern layers --dimensions 32,32,32 --output test_layers.txt
python3 tools/simple_test_generator.py --pattern mixed --dimensions 32,32,32 --output test_mixed.txt

# Generate data similar to existing file
python3 tools/simple_test_generator.py --like test_input.txt --pattern blocks --output similar_test.txt
```

### Test Compression
```bash
# Basic compression
./myapp < test_blocks.txt > compressed.txt

# With timing
time ./myapp < test_blocks.txt > compressed.txt
```

### Analyze Results
```bash
# Compare original vs compressed
python3 tools/simple_analyzer.py --compressed compressed.txt --original test_blocks.txt

# Just analyze compressed (provide dimensions)
python3 tools/simple_analyzer.py --compressed compressed.txt --dimensions 32,32,32

# Save analysis to JSON
python3 tools/simple_analyzer.py --compressed compressed.txt --original test_blocks.txt --output analysis.json
```

### Run Benchmarks
```bash
# Full benchmark suite (generates test data automatically)
python3 tools/simple_benchmark.py --iterations 5 --output-dir my_results

# Benchmark specific files
python3 tools/simple_benchmark.py --existing-tests "file1.txt,file2.txt" --iterations 3

# Generate test data only
python3 tools/simple_benchmark.py --generate-only --test-dir my_test_data
```

## 📊 Understanding Output

### Test Patterns
- **blocks**: Large uniform regions (expect high compression ~3:1+)
- **random**: Random materials (expect poor compression <1:1)
- **layers**: Horizontal material layers (expect medium compression ~2:1)
- **mixed**: Combination of patterns (realistic data)

### Key Metrics
- **Compression Ratio**: Original voxels ÷ compressed blocks (higher = better)
- **Volume Efficiency**: How well reconstruction preserves volume (~100% = lossless)
- **Processing Time**: Speed of compression algorithm
- **Voxels/Second**: Throughput metric

### Quality Assessment
- **Excellent**: Ratio ≥3.0, Efficiency ≥99%
- **Good**: Ratio ≥2.0, Efficiency ≥95%
- **Fair**: Ratio ≥1.0, Efficiency ≥90%
- **Poor**: Below fair thresholds

## 🎯 Example Workflow

```bash
# 1. Generate test data like your original
python3 tools/simple_test_generator.py --like test_input.txt --pattern mixed --output my_test.txt

# 2. Test compression
time ./myapp < my_test.txt > my_compressed.txt

# 3. Analyze results
python3 tools/simple_analyzer.py --compressed my_compressed.txt --original my_test.txt

# 4. Benchmark performance
python3 tools/simple_benchmark.py --existing-tests "my_test.txt" --iterations 5
```

## 📁 File Structure

```
simple_test_data/     # Generated test files
simple_results/       # Analysis results and compressed outputs
tools/
├── simple_test_generator.py  # Generate test data
├── simple_analyzer.py        # Analyze compression results
├── simple_benchmark.py       # Performance benchmarking
└── simple_requirements.txt   # Minimal dependencies
simple_demo.sh        # One-command demo
```

## 🎯 What Was Removed

Eliminated from the original suite:
- ❌ 3D model (.obj) input/output
- ❌ trimesh dependency
- ❌ 3D visualization features
- ❌ Mesh generation/reconstruction
- ❌ Complex 3D file format support

## ✅ What Remains

Pure compression focus:
- ✅ Test data generation (all patterns)
- ✅ Performance benchmarking
- ✅ Compression analysis
- ✅ Statistical reporting
- ✅ Support for your exact input format
- ✅ JSON output for automation

This streamlined version is **perfect for**:
- Algorithm development and optimization
- Performance testing and profiling
- Automated testing pipelines
- Research and analysis
- Production deployment scenarios

**Dependency**: Only `numpy` required (vs. trimesh + matplotlib + other 3D libraries)
