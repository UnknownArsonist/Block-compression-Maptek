# Archived 3D Visualization Tools

This directory contains the original complex tools that included 3D model processing and visualization capabilities.

## Archived Files:
- `model_to_text_converter.py` - 3D model to voxel conversion with trimesh
- `decompressor.py` - Full decompression with 3D mesh output  
- `benchmark_compression.py` - Complex benchmarking with 3D features
- `requirements.txt` - Heavy dependencies (trimesh, matplotlib, etc.)
- `setup_and_usage.md` - Detailed documentation for complex features

## Why Archived:
These tools were replaced with simpler, focused versions that:
- Don't require heavy 3D dependencies
- Focus purely on compression analysis
- Are faster and lighter
- Are easier to use and maintain

## To Restore:
If you need the 3D visualization features:
```bash
cp tools/archive_3d_tools/* tools/
pip install trimesh matplotlib pillow
```

## Current Simple Tools:
- `simple_test_generator.py` - Generate compression test data
- `simple_analyzer.py` - Analyze compression results  
- `simple_benchmark.py` - Performance benchmarking
- `simple_requirements.txt` - Minimal dependencies (just numpy)
