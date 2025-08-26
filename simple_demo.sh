#!/bin/bash

# Simple Compression Testing Demo
# Streamlined version focused purely on compression analysis

echo "🎯 Simple Compression Testing Suite"
echo "====================================="

# Check if compiled executable exists
if [ ! -f "./myapp" ]; then
    echo "❌ Error: ./myapp not found"
    echo "Please compile first with: make clean && make"
    exit 1
fi

# Create directories
mkdir -p simple_test_data
mkdir -p simple_results

echo ""
echo "📋 Step 1: Generating test datasets..."

# Generate different test patterns
echo "  • Uniform blocks (high compression expected)..."
python3 tools/simple_test_generator.py --pattern blocks --dimensions 32,32,32 --output simple_test_data/uniform_blocks.txt

echo "  • Random data (low compression expected)..."
python3 tools/simple_test_generator.py --pattern random --dimensions 32,32,32 --output simple_test_data/random_data.txt

echo "  • Layered materials (medium compression expected)..."
python3 tools/simple_test_generator.py --pattern layers --dimensions 32,32,32 --output simple_test_data/layered_data.txt

echo "  • Mixed patterns (realistic data)..."
python3 tools/simple_test_generator.py --pattern mixed --dimensions 32,32,32 --output simple_test_data/mixed_data.txt

echo "  • Data similar to your original test..."
python3 tools/simple_test_generator.py --like test_input.txt --pattern blocks --output simple_test_data/like_original.txt

echo ""
echo "⚡ Step 2: Testing compression performance..."

# Test each pattern
for file in simple_test_data/*.txt; do
    basename=$(basename "$file" .txt)
    echo "  • Testing $basename..."
    
    # Run compression and measure time
    time_output=$(time ( ./myapp < "$file" > "simple_results/${basename}_compressed.txt" ) 2>&1)
    
    # Quick analysis
    input_lines=$(wc -l < "$file")
    output_lines=$(wc -l < "simple_results/${basename}_compressed.txt")
    
    echo "    Input: $input_lines lines → Output: $output_lines blocks"
done

echo ""
echo "📊 Step 3: Analyzing results..."

# Analyze each result (save to JSON but don't spam console)
for compressed_file in simple_results/*_compressed.txt; do
    basename=$(basename "$compressed_file" _compressed.txt)
    original_file="simple_test_data/${basename}.txt"
    
    echo "  • Analyzing $basename..."
    python3 tools/simple_analyzer.py --compressed "$compressed_file" --original "$original_file" --output "simple_results/${basename}_analysis.json" --quiet
done

echo ""
echo "🏃 Step 4: Running comprehensive benchmark..."

# Run full benchmark
python3 tools/simple_benchmark.py --existing-tests "$(ls simple_test_data/*.txt | tr '\n' ',' | sed 's/,$//')" --iterations 3 --output-dir simple_results

echo ""
echo "✅ Demo Complete!"
echo "=================="
echo ""
echo "📁 Results saved in simple_results/:"
echo "   • Individual analysis files (*_analysis.json)"
echo "   • Comprehensive benchmark (benchmark_report.json)"
echo "   • Compressed outputs (*_compressed.txt)"
echo ""
echo "📊 Key files to check:"
echo "   • simple_results/benchmark_report.json - Full performance analysis"
echo "   • simple_test_data/ - Generated test datasets"
echo ""
echo "🎯 Quick comparison with your original data:"
if [ -f "test_input.txt" ]; then
    echo "Original test_input.txt compression analysis:"
    ./myapp < test_input.txt > simple_results/original_test_compressed.txt
    python3 tools/simple_analyzer.py --compressed simple_results/original_test_compressed.txt --original test_input.txt
    echo ""
fi

echo "💡 Next steps:"
echo "   • Review benchmark_report.json for detailed metrics"
echo "   • Try different patterns with simple_test_generator.py"
echo "   • Use simple_analyzer.py for custom analysis"
echo "   • Run simple_benchmark.py for performance testing"