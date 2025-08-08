#include "StreamProcessor.h"

// Constructor and Destructor
StreamProcessor::StreamProcessor(/* args */)
{
    // Constructor implementation
    count = 0;
    index_x = 0;
}

StreamProcessor::~StreamProcessor()
{
    // Destructor implementation
}

/*
// Function to split a string by a delimiter
vector<string> StreamProcessor::split(const string &s, char delimiter)
{
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}
*/

void StreamProcessor::startProcessing(FILE *in)
{
    getHeaderLine(in);
}

/*
// Function to read the header line and initialize dimensions
void StreamProcessor::get_headerLine()
{
    getline(cin, header_line);
    vector<string> dims = split(header_line, ',');

    x_count = stoi(dims[0]);
    y_count = stoi(dims[1]);
    z_count = stoi(dims[2]);
    parent_x = stoi(dims[3]);
    parent_y = stoi(dims[4]);
    parent_z = stoi(dims[5]);
    get_tagTable();
}
*/

void StreamProcessor::getHeaderLine(FILE *in)
{
    getCommaSeparatedValuesFromStream(in, &x_count, &y_count, &z_count, &parent_x, &parent_y, &parent_z);
    getLegendFromStream(in, &tag_table);
    processSlice(in);
}

void StreamProcessor::getCommaSeparatedValuesFromStream(FILE *in) {}
template <typename T, typename... Args>
void StreamProcessor::getCommaSeparatedValuesFromStream(FILE *in, T *value, Args... args)
{
    char c;
    *value = 0;
    while ((c = getc(in)) != EOF)
    {
        if (c == ',' || c == '\n')
        {
            break;
        }
        else
        {
            *value *= 10;
            *value += (int)c - '0';
        }
    }
    getCommaSeparatedValuesFromStream(in, args...);
}

/*
// Function to read the tag table
void StreamProcessor::get_tagTable()
{
    string line;
    while (getline(cin, line))
    {
        if (line.empty())
            break;
        auto parts = split(line, ',');
        tag_table[parts[0][0]] = parts[1];
    }
    process_slice();
}
*/

void StreamProcessor::getLegendFromStream(FILE *in, std::unordered_map<char, std::string> *legend)
{
    char c;
    char key = 0;
    std::string value = "";
    int v = 0;
    int n = 0;
    while ((c = getc(in)) != EOF)
    {
        if (c == ',')
        {
            v++;
        }
        else if (c == '\n')
        {
            if (n > 0)
            {
                return;
            }
            (*legend)[key] = value;
            v = 0;
            value.clear();
            n++;
        }
        else
        {
            if (v == 0)
            {
                key = c;
            }
            else
            {
                value += c;
            }
            n = 0;
        }
    }
}

// Function to process the slice of 3D block data
void StreamProcessor::processSlice(FILE *in)
{
    // resizing the 3D block data structure
    c.resize(z_count, vector<vector<char>>(y_count, vector<char>(x_count)));
    // Main Block : z-depth
    for (int z = 0; z < z_count; z++)
    {
        // Main Block : y-height
        for (int y = 0; y < y_count; y++)
        {
            // Main Block : x-width
            for (int x = 0; x < x_count; x++)
            {
                int ch;
                do
                {
                    ch = getc(in);
                } while (isspace(ch) && ch != EOF);

                c[z][y][x] = (char)ch;
            }
        }
    }
    // Store the 3D block in a member variable
    ptr_c = &c;
    parentBlock(ptr_c);
}

// Parent Block function to set the pointer to the 3D block data
void StreamProcessor::parentBlock(vector<vector<vector<char>>> *block)
{
    int num_parent_blocks_x = (x_count / parent_x);
    // 4D block to hold the parent block data
    parent_block.resize(num_parent_blocks_x * z_count,
                        vector<vector<vector<char>>>(
                            parent_z,
                            vector<vector<char>>(
                                parent_y,
                                vector<char>(parent_x))));

    // cout << x_count / parent_x;
    while (index_z < z_count)
    {
        /* code */
        count = 0;
        index_x = 0;

        while (count < num_parent_blocks_x)
        {
            int block_index = index_z * num_parent_blocks_x + count;

            // Initialize the parent block for each count
            parent_block[block_index].resize(parent_z, vector<vector<char>>(parent_y, vector<char>(parent_x)));

            processParentBlock(block, block_index, index_x, index_z);

            count++;
            index_x += parent_x;
        }

        index_z++;
    }
}

void StreamProcessor::processParentBlock(vector<vector<vector<char>>> *block, int count, int index_x, int index_z)
{
    // Main Block : z-depth

    for (int z = 0; z < parent_z; z++)
    {
        // Main Block : y-height
        for (int y = 0; y < parent_y; y++)
        {
            // Main Block : x-width
            for (int x = 0; x < parent_x; x++)
            {
                parent_block[count][z][y][x] = block->at(index_z).at(y).at(x + index_x);
            }
        }
    }
}

// print the header information and the 3D block data
void StreamProcessor::printHeader()
{
    // print the header information
    printf("%d, %d, %d, %d, %d, %d\n", x_count, y_count, z_count, parent_x, parent_y, parent_z);
    for (const auto &e : tag_table)
    {
        printf("%c, %s\n", e.first, e.second.c_str());
    }

    // Print the 3D block data
    // Block z-depth
    for (int i = 0; i < z_count; i++)
    {
        cout << "Slice " << i << ":\n";
        for (int j = 0; j < y_count; j++)
        {
            for (int k = 0; k < x_count; k++)
            {
                cout << c[i][j][k];
            }
            cout << '\n';
        }
        cout << '\n';
    }

    // parent block data
    /*
    for (int i = 0; i < parent_z; i++)
    {
        cout << "Parent Slice " << i << ":\n";
        for (int j = 0; j < parent_y; j++)
        {
            for (int k = 0; k < parent_x; k++)
            {
                cout << parent_block[i][j][k];
            }
            cout << '\n';
        }
        cout << '\n';
    }
    */
    for (size_t block_i = 0; block_i < parent_block.size(); ++block_i)
    {
        std::cout << "Parent Block " << block_i << ":\n";
        for (int z = 0; z < parent_z; ++z)
        {
            std::cout << "Slice " << z << ":\n";
            for (int y = 0; y < parent_y; ++y)
            {
                for (int x = 0; x < parent_x; ++x)
                {
                    std::cout << parent_block[block_i][z][y][x];
                }
                std::cout << '\n';
            }
            std::cout << '\n';
        }
    }
}

// Getters for dimensions and tag table
// vector<string> StreamProcessor::getSlice() { return slice; }
unordered_map<char, string> StreamProcessor::getTagTable() { return tag_table; }
int StreamProcessor::getXCount() { return x_count; }
int StreamProcessor::getYCount() { return y_count; }
int StreamProcessor::getZCount() { return z_count; }
int StreamProcessor::getParentX() { return parent_x; }
int StreamProcessor::getParentY() { return parent_y; }
int StreamProcessor::getParentZ() { return parent_z; }
