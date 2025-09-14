Objective:
Create a high-performance, command-line C++ application (compress.exe) that losslessly compresses 3D block model data. The program must read uncompressed data from standard input and write compressed data to standard output, adhering strictly to the formats specified below. The primary compression method will be a recursive subdivision of parent blocks, similar to an octree, to find the largest possible uniform-colored sub-blocks.

Core Algorithm:
The compression strategy is to process the model in chunks defined by a "parent block" size (parent_x, parent_y, parent_z). For each parent block:

Check if the entire parent block consists of a single color (label). If yes, output it as a single compressed block.

If the parent block is not uniform, recursively subdivide it into eight smaller sub-blocks (or four for 2D cases, two for 1D).

For each sub-block, repeat the process: check for uniformity. If a sub-block is uniform, output it. If not, subdivide it again.

Continue this recursion until you reach a block of size 1x1x1 or a uniform block is found. The goal is to represent the parent block's data with the minimum number of largest possible rectangular blocks.

Functional Requirements:

1. Input Parsing (from stdin):

Header Line: The first line contains six comma-separated integers: x_count, y_count, z_count, parent_x, parent_y, parent_z.

Tag Table: Following the header are n lines, each with a tag,label pair (e.g., o,sea). This section is terminated by a single empty line. You must parse and store this mapping.

Block Data: The main data follows. It's organized in z_count slices, each separated by a blank line. Each slice has y_count rows, and each row has x_count tag characters. The data is ordered Z (bottom-to-top), then Y (bottom-to-top), then X (left-to-right).

2. Memory Management:

The application must be memory-efficient. It must not load the entire block model into RAM at once.

It should process the model in horizontal slabs that are at most parent_z slices thick. This allows the tool to handle models that are much larger than the available system memory.

3. Compression Logic (Octree-based):

For each parent block region in the input data slab:

Implement a recursive function, compress_block(x, y, z, size_x, size_y, size_z).

The function should first check if all blocks within the given dimensions have the same label.

If they do, write a single output line for this compressed block.

If not, and if the block dimensions are all greater than 1, subdivide it into 8 children and recursively call compress_block for each child. (Handle 2D/1D subdivision accordingly if a dimension size is 1).

The base case for the recursion is a 1x1x1 block, which is always uniform.

4. Output Format (to stdout):

The output consists of one line per compressed block.

Each line must contain seven comma-separated values: x_position, y_position, z_position, x_size, y_size, z_size, label.

The label must be the string from the parsed tag table, not the character tag.

The order of output blocks does not matter.

5. Performance Goals:

Speed: The C++ implementation should be highly optimized for execution speed. Use efficient data structures, avoid unnecessary computations, and manage memory carefully.

Compression Ratio: The algorithm must effectively minimize the number of output blocks compared to input blocks. The recursive subdivision should naturally achieve good compression on models with large uniform areas.

6. Technical Specifications:

Language: C++ (latest stable standard).

Compiler: Must compile with a standard C++ compiler (like g++ or MSVC) into a single executable file.

Dependencies: No external libraries are allowed unless they are standard C++ libraries. The executable must be self-contained.

Input/Output: Strictly use stdin for input and stdout for output. Do not write to or read from any other files. No interactive prompts.

Example Walkthrough (from project PDF):

Input starts with: 64,64,1,8,8,1

Tag table follows: o,sea, W,WA, etc.

A parent block at (32, 24, 0) of size 8x8x1 might be entirely of the 's' tag, which maps to "SA".

Output for this block: 32,24,0,8,8,1,SA

Another parent block at (32, 16, 0) is mixed. Your octree algorithm should recursively subdivide it to produce the 18 output blocks as shown in the example.

Final Deliverable:

A single, runnable .exe file named compress.exe.

The complete C++ source code in a single .cpp file.