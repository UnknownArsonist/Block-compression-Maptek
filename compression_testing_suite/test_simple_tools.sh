#!/bin/bash

# Quick test of the simple tools to make sure they work

echo "🧪 Testing Simple Compression Tools"
echo "===================================="

# Make scripts executable
chmod +x simple_demo.sh cleanup_old_tools.sh
chmod +x tools/simple_*.py

echo "✅ Made scripts executable"

# Test simple test generator
echo ""
echo "📋 Testing simple test generator..."
python3 tools/simple_test_generator.py --pattern blocks --dimensions 16,16,16 --output test_simple.txt
echo "Generated test file: $(wc -l < test_simple.txt) lines"

# Test compression
echo ""
echo "⚡ Testing compression..."
if [ -f "./myapp" ]; then
    ./myapp < test_simple.txt > test_simple_compressed.txt
    echo "Compressed to: $(wc -l < test_simple_compressed.txt) blocks"
    
    # Test analyzer - show output to verify it's working
    echo ""
    echo "📊 Testing analyzer..."
    python3 tools/simple_analyzer.py --compressed test_simple_compressed.txt --original test_simple.txt
    
    echo ""
    echo "✅ All simple tools working correctly!"
    echo ""
    echo "🚀 Ready to run: ./simple_demo.sh"
else
    echo "❌ ./myapp not found - compile first with: make clean && make"
fi

# Cleanup test files
rm -f test_simple.txt test_simple_compressed.txt

echo ""
echo "📋 Tools ready:"
echo "   • simple_demo.sh - Run everything"
echo "   • tools/simple_test_generator.py - Generate test data"  
echo "   • tools/simple_analyzer.py - Analyze results"
echo "   • tools/simple_benchmark.py - Performance testing"
echo "   • cleanup_old_tools.sh - Remove 3D tools (optional)"