#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <sstream>
#include <chrono>
#include <algorithm> // For std::min
#include <tuple>     // For std::tie

// A macro for 1D array indexing improves cache performance over nested vectors.
// Corrected to use size_t for all multiplications to prevent integer overflow with large dimensions.
#define GET_INDEX(x, y, z, x_count, y_count) ((size_t)(z) * (size_t)(x_count) * (size_t)(y_count) + (size_t)(y) * (size_t)(x_count) + (size_t)(x))

struct BlockInfo {
    int x_count, y_count, z_count;
    int parent_x, parent_y, parent_z;
    std::unordered_map<char, std::string> tag_to_label;
};

// Parses the header and tag table from the input stream.
BlockInfo parse_header(std::istream& input) {
    BlockInfo info;
    std::string header_line;
    std::getline(input, header_line);

    std::stringstream ss(header_line);
    std::string token;
    std::getline(ss, token, ','); info.x_count = std::stoi(token);
    std::getline(ss, token, ','); info.y_count = std::stoi(token);
    std::getline(ss, token, ','); info.z_count = std::stoi(token);
    std::getline(ss, token, ','); info.parent_x = std::stoi(token);
    std::getline(ss, token, ','); info.parent_y = std::stoi(token);
    std::getline(ss, token, ','); info.parent_z = std::stoi(token);

    std::string tag_line;
    while (std::getline(input, tag_line) && !tag_line.empty()) {
        if(tag_line.length() > 1 && tag_line[1] == ',') {
            info.tag_to_label[tag_line[0]] = tag_line.substr(2);
        }
    }
    return info;
}

void process_model_greedy(const BlockInfo& info, std::istream& input) {
    int slab_start_z = 0;

    // Process the model in horizontal slabs to keep memory usage low.
    while (slab_start_z < info.z_count) {
        int slab_height = std::min(info.parent_z, info.z_count - slab_start_z);
        if (slab_height <= 0) break;

        std::vector<char> slab_data((size_t)info.x_count * info.y_count * slab_height);
        std::vector<bool> visited(slab_data.size(), false);
        
        // Read the raw character data for the current slab.
        for (int z = 0; z < slab_height; ++z) {
            // Skips the blank line separating slices.
            if (!(slab_start_z == 0 && z == 0)) {
                 std::string dummy;
                 std::getline(input, dummy);
            }
            for (int y = 0; y < info.y_count; ++y) {
                std::string line;
                if (!std::getline(input, line)) break;
                for (int x = 0; x < info.x_count; ++x) {
                    if (x < line.length()) {
                       slab_data[GET_INDEX(x, y, z, info.x_count, info.y_count)] = line[x];
                    }
                }
            }
        }
        
        for (int py = 0; py < info.y_count; py += info.parent_y) {
            for (int px = 0; px < info.x_count; px += info.parent_x) {
                const int parent_x_end = std::min(px + info.parent_x, info.x_count);
                const int parent_y_end = std::min(py + info.parent_y, info.y_count);
                const int parent_z_end = slab_height;
                
                for (int z = 0; z < parent_z_end; ++z) {
                    for (int y = py; y < parent_y_end; ++y) {
                        for (int x = px; x < parent_x_end; ++x) {
                            size_t current_idx = GET_INDEX(x, y, z, info.x_count, info.y_count);
                            if (visited[current_idx])
                                continue;

                            char current_tag = slab_data[current_idx];

                            // Determine max size in X
                            int maxX = x + 1; // 0 1 7
                            while (maxX < parent_z_end) {
                                if(visited[GET_INDEX(maxX, y, z, info.x_count, info.y_count)] || slab_data[GET_INDEX(maxX, y, z, info.x_count, info.y_count)] != current_tag) break;
                                maxX++; // 1 7
                            }

                            // Determine max size in Y
                            int maxY = y + 1;         // 0
                            while (maxY < parent_y_end)
                            {
                                bool uniformY = true;
                                // x = 0 -> maxX = 1; 1 < 7
                                for (int xi = x; xi < maxX; xi++)
                                {
                                    // x1 = 1
                                    if (visited[GET_INDEX(xi, maxY, z, info.x_count, info.y_count)] || slab_data[GET_INDEX(xi, maxY, z, info.x_count, info.y_count)] != current_tag)
                                    {
                                        uniformY = false;
                                        break;
                                    }
                                }
                                if (!uniformY)
                                    break;
                                maxY++;
                            }

                            // Determine max size in Z
                            int maxZ = z + 1;
                            // checks the subblocks, are they uniform and did we alreadly visit them.
                            while (maxZ < parent_z_end)
                            {
                                bool uniformZ = true;
                                // y = 0 maxY = 1
                                for (int yi = y; yi < maxY; yi++)
                                {
                                    // x = 0 MaxX = 1
                                    for (int xi = x; xi < maxX; xi++)
                                    {
                                        if (visited[GET_INDEX(xi, yi, maxZ, info.x_count, info.y_count)] || slab_data[GET_INDEX(xi, yi, maxZ, info.x_count, info.y_count)] != current_tag)
                                        {
                                            uniformZ = false;
                                            break;
                                        }
                                    }
                                    if (!uniformZ)
                                        break;
                                }
                                if (!uniformZ)
                                    break;
                                maxZ++;
                            }

                            // Mark all as visited
                            for (int zz = z; zz < maxZ; zz++)
                                for (int yy = y; yy < maxY; yy++)
                                    for (int xx = x; xx < maxX; xx++)
                                        visited[GET_INDEX(xx,yy,zz,info.x_count,info.y_count)] = true;

                            // Output the packed block
                            
                            /* std::cerr << current_tag << std::endl;
                            std::cerr << info.tag_to_label.at(current_tag) << std::endl; */
                            std::cout << x << "," << y << "," << z + slab_start_z << ","
                                      << maxX - x << "," << maxY - y << "," << maxZ - z << ","
                                      << info.tag_to_label.at(current_tag) << "\n";
                            //fprintf(stderr, "Compressor: %d,%d,%d,%s\n", sub_block->x, sub_block->y, sub_block->z, (*tag_table)[target].c_str());
                            //output_stream->push((void **)&parent_block);
                        }
                        // x += 1; x = 1; x = 2
                    }
                    // y = 1
                }
            }
        }
        slab_start_z += slab_height;
    }
}

// Main processing function using a boundary-constrained Greedy Meshing algorithm.
void process_model_constrained_greedy(const BlockInfo& info, std::istream& input) {
    int slab_start_z = 0;

    // Process the model in horizontal slabs to keep memory usage low.
    while (slab_start_z < info.z_count) {
        int slab_height = std::min(info.parent_z, info.z_count - slab_start_z);
        if (slab_height <= 0) break;

        std::vector<char> slab_data((size_t)info.x_count * info.y_count * slab_height);
        std::vector<bool> visited(slab_data.size(), false);

        // Read the raw character data for the current slab.
        for (int z = 0; z < slab_height; ++z) {
            // Skips the blank line separating slices.
            if (!(slab_start_z == 0 && z == 0)) {
                 std::string dummy;
                 std::getline(input, dummy);
            }
            for (int y = 0; y < info.y_count; ++y) {
                std::string line;
                if (!std::getline(input, line)) break;
                for (int x = 0; x < info.x_count; ++x) {
                    if (x < line.length()) {
                       slab_data[GET_INDEX(x, y, z, info.x_count, info.y_count)] = line[x];
                    }
                }
            }
        }
        
        // --- Constrained Greedy Meshing ---
        // Iterate through each PARENT BLOCK region in the slab.
        for (int py = 0; py < info.y_count; py += info.parent_y) {
            for (int px = 0; px < info.x_count; px += info.parent_x) {
                
                // Define the absolute boundaries for this parent block.
                const int parent_x_end = std::min(px + info.parent_x, info.x_count);
                const int parent_y_end = std::min(py + info.parent_y, info.y_count);
                const int parent_z_end = slab_height;

                // Now run greedy meshing *inside* these boundaries.
                for (int z = 0; z < parent_z_end; ++z) {
                    for (int y = py; y < parent_y_end; ++y) {
                        for (int x = px; x < parent_x_end; ++x) {
                            size_t current_idx = GET_INDEX(x, y, z, info.x_count, info.y_count);
                            if (visited[current_idx]) continue;

                            char current_tag = slab_data[current_idx];
                            int best_w = 1, best_h = 1, best_d = 1; // Default to 1x1x1
                            size_t max_volume = 0;

                            // --- OPTIMIZATION: Try all 6 expansion orders ---
                            for (int order = 0; order < 6; ++order) {
                                int w = 1, h = 1, d = 1;

                                // The primary axis of expansion
                                int u_end, v_end, w_coord_end;
                                int u_start, v_start, w_coord_start;

                                // Remap axes based on current order
                                switch(order) {
                                    case 0: u_start=x; v_start=y; w_coord_start=z; u_end=parent_x_end; v_end=parent_y_end; w_coord_end=parent_z_end; break; // x,y,z
                                    case 1: u_start=x; v_start=z; w_coord_start=y; u_end=parent_x_end; v_end=parent_z_end; w_coord_end=parent_y_end; break; // x,z,y
                                    case 2: u_start=y; v_start=x; w_coord_start=z; u_end=parent_y_end; v_end=parent_x_end; w_coord_end=parent_z_end; break; // y,x,z
                                    case 3: u_start=y; v_start=z; w_coord_start=x; u_end=parent_y_end; v_end=parent_z_end; w_coord_end=parent_x_end; break; // y,z,x
                                    case 4: u_start=z; v_start=x; w_coord_start=y; u_end=parent_z_end; v_end=parent_x_end; w_coord_end=parent_y_end; break; // z,x,y
                                    case 5: u_start=z; v_start=y; w_coord_start=x; u_end=parent_z_end; v_end=parent_y_end; w_coord_end=parent_x_end; break; // z,y,x
                                }

                                // Expand in the first dimension
                                while(u_start + w < u_end) {
                                    int cur_x, cur_y, cur_z;
                                    switch(order){
                                        case 0: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(u_start + w, v_start, w_coord_start); break;
                                        case 1: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(u_start + w, w_coord_start, v_start); break;
                                        case 2: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(v_start, u_start + w, w_coord_start); break;
                                        case 3: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(w_coord_start, u_start + w, v_start); break;
                                        case 4: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(v_start, w_coord_start, u_start + w); break;
                                        case 5: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(w_coord_start, v_start, u_start + w); break;
                                    }
                                    if(visited[GET_INDEX(cur_x, cur_y, cur_z, info.x_count, info.y_count)] || slab_data[GET_INDEX(cur_x, cur_y, cur_z, info.x_count, info.y_count)] != current_tag) break;
                                    w++;
                                }

                                // Expand in the second dimension
                                bool can_expand_v = true;
                                while(v_start + h < v_end && can_expand_v) {
                                    for(int i = 0; i < w; ++i) {
                                        int cur_x, cur_y, cur_z;
                                        switch(order){
                                            case 0: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(u_start + i, v_start + h, w_coord_start); break;
                                            case 1: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(u_start + i, w_coord_start, v_start + h); break;
                                            case 2: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(v_start + h, u_start + i, w_coord_start); break;
                                            case 3: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(w_coord_start, u_start + i, v_start + h); break;
                                            case 4: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(v_start + h, w_coord_start, u_start + i); break;
                                            case 5: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(w_coord_start, v_start + h, u_start + i); break;
                                        }
                                        if(visited[GET_INDEX(cur_x, cur_y, cur_z, info.x_count, info.y_count)] || slab_data[GET_INDEX(cur_x, cur_y, cur_z, info.x_count, info.y_count)] != current_tag) { can_expand_v = false; break; }
                                    }
                                    if(can_expand_v) h++;
                                }
                                
                                // Expand in the third dimension
                                bool can_expand_w = true;
                                while(w_coord_start + d < w_coord_end && can_expand_w) {
                                    for(int j = 0; j < h; ++j) {
                                        for(int i = 0; i < w; ++i) {
                                           int cur_x, cur_y, cur_z;
                                           switch(order){
                                                case 0: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(u_start + i, v_start + j, w_coord_start + d); break;
                                                case 1: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(u_start + i, w_coord_start + d, v_start + j); break;
                                                case 2: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(v_start + j, u_start + i, w_coord_start + d); break;
                                                case 3: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(w_coord_start + d, u_start + i, v_start + j); break;
                                                case 4: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(v_start + j, w_coord_start + d, u_start + i); break;
                                                case 5: std::tie(cur_x, cur_y, cur_z) = std::make_tuple(w_coord_start + d, v_start + j, u_start + i); break;
                                            }
                                            if(visited[GET_INDEX(cur_x, cur_y, cur_z, info.x_count, info.y_count)] || slab_data[GET_INDEX(cur_x, cur_y, cur_z, info.x_count, info.y_count)] != current_tag) { can_expand_w = false; break; }
                                        }
                                        if(!can_expand_w) break;
                                    }
                                    if(can_expand_w) d++;
                                }
                                
                                size_t volume = (size_t)w * h * d;
                                if (volume > max_volume) {
                                    max_volume = volume;
                                    switch(order) {
                                        case 0: case 1: std::tie(best_w, best_h, best_d) = std::make_tuple(w,h,d); break;
                                        case 2: case 3: std::tie(best_w, best_h, best_d) = std::make_tuple(h,w,d); break;
                                        case 4: case 5: std::tie(best_w, best_h, best_d) = std::make_tuple(h,d,w); break;
                                    }
                                     switch(order) {
                                        case 0: std::tie(best_w, best_h, best_d) = std::make_tuple(w,h,d); break;
                                        case 1: std::tie(best_w, best_h, best_d) = std::make_tuple(w,d,h); break;
                                        case 2: std::tie(best_w, best_h, best_d) = std::make_tuple(h,w,d); break;
                                        case 3: std::tie(best_w, best_h, best_d) = std::make_tuple(d,w,h); break;
                                        case 4: std::tie(best_w, best_h, best_d) = std::make_tuple(h,d,w); break;
                                        case 5: std::tie(best_w, best_h, best_d) = std::make_tuple(d,h,w); break;
                                    }
                                }
                            }

                            // Mark all voxels in the new best block as visited.
                            for (int dz = 0; dz < best_d; ++dz) {
                                for (int dy = 0; dy < best_h; ++dy) {
                                    for (int dx = 0; dx < best_w; ++dx) {
                                        visited[GET_INDEX(x + dx, y + dy, z + dz, info.x_count, info.y_count)] = true;
                                    }
                                }
                            }
                            
                            /* std::cerr << current_tag << std::endl;
                            std::cerr << info.tag_to_label.at(current_tag) << std::endl; */
                            std::cout << x << "," << y << "," << z + slab_start_z << ","
                                      << best_w << "," << best_h << "," << best_d << ","
                                      << info.tag_to_label.at(current_tag) << "\n";
                        }
                    }
                }
            }
        }
        //std::cerr << "chunk: " << slab_start_z << std::endl;
        slab_start_z += slab_height;
    }
}

int main() {
    // Fast I/O is crucial for performance.
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    BlockInfo info = parse_header(std::cin);
    auto started = std::chrono::high_resolution_clock::now();
    
    process_model_constrained_greedy(info, std::cin);
    /* int slab_start_z = 0;
    while (slab_start_z < info.z_count) {
        int slab_height = std::min(info.parent_z, info.z_count - slab_start_z);
        std::vector<char> slab_data((size_t)info.x_count * info.y_count * slab_height);
        for (int z = 0; z < slab_height; ++z) {
            // Skips the blank line separating slices.
            if (!(slab_start_z == 0 && z == 0)) {
                    std::string dummy;
                    std::getline(std::cin, dummy);
            }
            for (int y = 0; y < info.y_count; ++y) {
                std::string line;
                if (!std::getline(std::cin, line)) break;
                for (int x = 0; x < info.x_count; ++x) {
                    if (x < line.length()) {
                        slab_data[GET_INDEX(x, y, z, info.x_count, info.y_count)] = line[x];
                    }
                }
            }
        }
        slab_start_z += slab_height;
    } */
    auto end = std::chrono::high_resolution_clock::now();
    std::cerr << "InputStreamReader Runtime:\n  " << std::chrono::duration_cast<std::chrono::milliseconds>(end - started).count() << std::endl;

    return 0;
}