


#include <iostream>
#include <vector>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <cstring>
#include <limits>
#include <functional>
#include <algorithm>
#include <queue>
#include <math.h>
#include <thread>

// ----------- //





// ----------- //

#pragma region Structs and typedefs

const char* node_file_paths[] = { "noder.txt", "kanter.txt", "interessepkt.txt" };
const std::vector<const char*> landmark_file_paths = {
    "Aabenraa0.txt",
    "Ålesund0.txt",
    "Ilomantsi0.txt",
    "Trinntorp0.txt",
    "Nordkapp0.txt"
};

typedef int distance_t;
typedef int node_idx_t;
typedef struct landmark_t {
    node_idx_t node;
    std::vector<distance_t> to_nodes;
    std::vector<distance_t> from_nodes;
} landmark_t;

typedef struct _edge {
    node_idx_t to_node_idx;
    distance_t kjøretid;
    distance_t lengde;
    distance_t fartsgrense;
} edge_t;

typedef struct _point_of_interest {
    char type = 0;
    std::string name;
} point_of_interest;

typedef struct _node {
    node_idx_t idx;

    double lon;
    double lat;

    std::vector<point_of_interest> points_of_interest;
    std::vector<edge_t> edges;

    std::vector<distance_t> distance_from_landmarks;
    std::vector<distance_t> distance_to_landmarks;
} node_t;

typedef struct search_result_t {
    node_idx_t previous_node;
    distance_t priority;
    distance_t distance;
    node_idx_t node_idx;
    int update_id;

    friend bool operator<(const search_result_t& lhs, const search_result_t& rhs) { return lhs.priority < rhs.priority; }
    friend bool operator>(const search_result_t& lhs, const search_result_t& rhs) { return lhs.priority > rhs.priority; }
} search_result_t;

#pragma endregion

// ----------- //





// ----------- //

#pragma region Reading nodes into memory

void read_line_info(char*& str_it, int* node_idx, int* type, char* name, int max_name_len) {
    *node_idx = atoi(str_it);

    while (*str_it != '\t') ++str_it;
    while (*str_it == '\t') ++str_it;
    *type = atoi(str_it);

    while (*str_it != '\t') ++str_it;
    while (*str_it == '\t') ++str_it;
    
    char* mark = str_it;

    while ((*str_it != '\n') & (*str_it != '\0')) ++str_it;

    int read_len = std::min((int)(str_it - mark), (int)max_name_len);
    memcpy(name, mark, read_len * sizeof(char));
    name[read_len] = '\0';
    
    if (*str_it == '\0')
        return;
    
    while (*str_it == '\n') ++str_it;
}

void read_nodes_info(node_t* nodes, FILE* f_info) {
    fseek(f_info, 0, SEEK_END);
    size_t f_size = ftell(f_info);
    fseek(f_info, 0, SEEK_SET);
    
    int count;
    fscanf(f_info, "%d\n", &count);
    f_size -= ftell(f_info);

    char* content = new char[f_size + 1];
    fread(content, sizeof(char), f_size, f_info);
    content[f_size] = '\0';

    int node_idx;
    int type;
    char name[129] {0};

    char* str_it = content;

    while (count--) {
        read_line_info(str_it, &node_idx, &type, name, 128);

        point_of_interest point_of_interest;

        // Store the type
        point_of_interest.type = type;

        // Find the length of the name
        int name_len = std::min((int) strlen(name) - 1, 128);

        // Make sure the string has an end
        name[name_len] = '\0';
        
        // Give the name to the node
        point_of_interest.name = std::string(name + 1);

        // Add point of interest to the node
        nodes[node_idx].points_of_interest.push_back(point_of_interest);

        continue;
    }

    delete[] content;

    printf("Done reading node attributes\n");
}

void read_line_edges(char*& str_it, int* from, int* to, int* kjøretid, int* lengde, int* fartsgrense) {
    *from = atoi(str_it);

    while (*str_it != '\t') ++str_it;
    while (*str_it == '\t') ++str_it;
    *to = atoi(str_it);
    
    while (*str_it != '\t') ++str_it;
    while (*str_it == '\t') ++str_it;
    *kjøretid = atoi(str_it);
    
    while (*str_it != '\t') ++str_it;
    while (*str_it == '\t') ++str_it;
    *lengde = atoi(str_it);
    
    while (*str_it != '\t') ++str_it;
    while (*str_it == '\t') ++str_it;
    *fartsgrense = atoi(str_it);

    while ((*str_it != '\n') & (*str_it != '\0')) ++str_it;

    if (*str_it == '\0')
        return;
    
    while (*str_it == '\n') ++str_it;
}

void read_nodes_edges(node_t* nodes, FILE* f_edge) {
    // Find the total size of the file
    fseek(f_edge, 0, SEEK_END);
    size_t f_size = ftell(f_edge);
    fseek(f_edge, 0, SEEK_SET);

    // The amount of lines in the file
    size_t count;
    fscanf(f_edge, "%zd \n", &count);

    // Find the size of the content (without the metadata)
    f_size -= ftell(f_edge);

    // The content of the file
    char* content = new char[f_size + 1];
    fread(content, sizeof(char), f_size, f_edge);
    content[f_size] = '\0';

    // An iterator for the content
    char* str_it = content;

    while (count--) {
        int node_idx;
        edge_t new_edge;

        read_line_edges(str_it, &node_idx, &new_edge.to_node_idx, &new_edge.kjøretid, &new_edge.lengde, &new_edge.fartsgrense);

        nodes[node_idx].edges.push_back(new_edge);
    }
    
    delete[] content;

    printf("Done reading edges\n");
}

void read_line_lon_lat(char*& str_it, int* node_idx, double* lon, double* lat) {
    *node_idx = atoi(str_it);
    
    while (*str_it != ' ') ++str_it;
    while (*str_it == ' ') ++str_it;
    *lon = atof(str_it);
    
    while (*str_it != ' ') ++str_it;
    while (*str_it == ' ') ++str_it;
    *lat = atof(str_it);

    while ((*str_it != '\n') & (*str_it != '\0')) ++str_it;

    if (*str_it == '\0')
        return;
    
    while (*str_it == '\n') ++str_it;
}

void read_nodes_lon_lat(node_t* nodes, int* node_count, FILE* f_node) {
    size_t count = *node_count;

    // Find the size of the content
    size_t mark = ftell(f_node);
    fseek(f_node, 0, SEEK_END);
    size_t f_size = ftell(f_node) - mark;
    fseek(f_node, (long) mark - 1, SEEK_SET);

    // The content of the file, excluding metadata
    char* content = new char[f_size + 1];
    fread(content, sizeof(char), f_size, f_node);
    content[f_size] = '\0';

    // An iterator for the content
    char* str_it = content;

    int node_idx;
    double lon, lat;

    while (count--) {
        read_line_lon_lat(str_it, &node_idx, &lon, &lat);
        nodes[node_idx].idx = node_idx;
        nodes[node_idx].lon = lon;
        nodes[node_idx].lat = lat;
    }

    delete[] content;

    printf("Done reading node coordinates\n");
}

node_t* read_nodes(FILE* f_node, FILE* f_edge, FILE* f_type, int* node_count) {
    fscanf(f_node, "%d\n", node_count);

    node_t* nodes = new node_t[*node_count];
    
    ///*
    std::thread t_node(read_nodes_lon_lat, nodes, node_count, f_node);
    std::thread t_edge(read_nodes_edges, nodes, f_edge);
    std::thread t_info(read_nodes_info, nodes, f_type);

    t_node.join();
    t_edge.join();
    t_info.join();
    //*/
    
    
    // read_nodes_lon_lat(nodes, node_count, f_node);
    // read_nodes_edges(nodes, f_edge);
    // read_nodes_info(nodes, f_type);
    //*/

    return nodes;
}

node_t* read_nodes(int* node_count) {
    FILE* f_node;
    FILE* f_edge;
    FILE* f_type;

    f_node = fopen(node_file_paths[0], "r");
    f_edge = fopen(node_file_paths[1], "r");
    f_type = fopen(node_file_paths[2], "r");

    if (!f_node)
        printf("Could not open file %s", node_file_paths[0]);
    if (!f_edge)
        printf("Could not open file %s", node_file_paths[1]);
    if (!f_type)
        printf("Could not open file %s", node_file_paths[2]);
    
    if (!f_node || !f_edge || !f_type)
        return 0;

    node_t* nodes = read_nodes(f_node, f_edge, f_type, node_count);
    
    fclose(f_node);
    fclose(f_edge);
    fclose(f_type);

    return nodes;
}

node_t* read_nodes() {
    int node_count;
    return read_nodes(&node_count);
}

#pragma endregion

// ----------- //





// ----------- //

#pragma region Dijkstras algorithm

search_result_t* dijkstra(const node_t* nodes, const size_t node_count, const int start_node, const int end_node, int* processed_nodes) {
    search_result_t* result = new search_result_t[node_count];

    for (int i = 0; i < node_count; i++) {
        result[i].node_idx = i;
        result[i].priority = (std::numeric_limits<decltype(search_result_t::priority)>::max)();
        result[start_node].previous_node = -1;
        result[i].update_id = 0;
    }

    result[start_node].priority = 0;

    std::priority_queue<search_result_t, std::vector<search_result_t>, std::greater<search_result_t>> queue;
    queue.push(result[start_node]);



    while (!queue.empty()) {
        *processed_nodes += 1;

        search_result_t sn = queue.top();
        queue.pop();
        while (sn.update_id != result[sn.node_idx].update_id) {
            sn = queue.top();
            queue.pop();
        }

        if (start_node != end_node && sn.node_idx == end_node)
            break;

        const int node_idx = sn.node_idx;

        for (edge_t e : nodes[node_idx].edges) {
            search_result_t& dst_node = result[e.to_node_idx];

            search_result_t new_node;
            new_node.previous_node = node_idx;
            new_node.priority = sn.priority + e.kjøretid;
            new_node.node_idx = e.to_node_idx;
            new_node.update_id = result[new_node.node_idx].update_id + 1;

            if (new_node.priority < dst_node.priority) {
                result[e.to_node_idx] = new_node;
                queue.push(new_node);
            }
        }
    }

    return result;
}

search_result_t* dijkstra(const node_t* nodes, const size_t node_count, const int start_node, int* processed_nodes) {
    return dijkstra(nodes, node_count, start_node, start_node, processed_nodes);
}

search_result_t* dijkstra(const node_t* nodes, const size_t node_count, const int start_node) {
    int processed_nodes = 0;
    return dijkstra(nodes, node_count, start_node, start_node, &processed_nodes);
}

std::mutex print_lock;

void test_dijkstra(const node_t* nodes, const int node_count, const int start_node, const int end_node) {
    int nodes_processed = 0;

    // printf("Started dijkstra...");

    auto start = std::chrono::high_resolution_clock::now();
    search_result_t* result = dijkstra(nodes, node_count, start_node, end_node, &nodes_processed);
    double elapsed = (long double)(std::chrono::high_resolution_clock::now() - start).count() / 1'000'000'000;
    
    // printf("DONE\n");

    int nodes_in_route = 1;
    int curr_node = end_node;
    while (curr_node != start_node) {
        nodes_in_route++;
        curr_node = result[curr_node].previous_node;
    }

    int time = result[end_node].priority / 100;
    int seconds = time % 60;
    int minutes = (time / 60) % 60;
    int hours = (time / 3600);

    print_lock.lock();
    printf("[%7d -> %7d]   nodes: %4d   Time: %02d:%02d:%02d   Proc-nodes: %7d   Proc-time(s): %f\n", start_node, end_node, nodes_in_route, hours, minutes, seconds, nodes_processed, elapsed);
    print_lock.unlock();

    delete[] result;
}

void test_dijkstra_multiple_data(node_t* nodes, node_idx_t node_count) {
    std::vector<std::pair<int, int>> tests;
    tests.push_back({2800567, 7705656});
    tests.push_back({7705656, 2800567});
    tests.push_back({647826,  136530});
    tests.push_back({136530,  647826});
    tests.push_back({7826348, 2948202});
    tests.push_back({2948202, 7826348});
    tests.push_back({339910,  1853145});
    tests.push_back({1853145,  339910});
    tests.push_back({2503331, 2866570});
    tests.push_back({2866570, 2503331});
    tests.push_back({6441311, 3168086});
    tests.push_back({3168086, 6441311});

    for (int i = 0; i < tests.size(); i++)
        test_dijkstra(nodes, node_count, tests[i].first, tests[i].second);
}

#pragma endregion

// ----------- //





// ----------- //

#pragma region Pre-processing nodes

node_t* create_inverse_graph(const node_t* src, size_t node_count, int* progress) {
    node_t* inverse = new node_t[node_count];

    const node_t* current = src;

    while (node_count--) {
        for (edge_t e : current->edges) {
            // Set the source node as the edge's destination node
            int src = e.to_node_idx;

            // Set the destination of the edge to the current node
            // (Editing 'e' directly, because it is a copy)
            e.to_node_idx = current->idx;

            // Add the new edge to the destination node
            inverse[src].edges.push_back(e);
        }

        (*progress)++;
        current++;
    }

    return inverse;
}

void create_landmark_to_nodes(landmark_t* l, const node_t* nodes, const size_t* node_count, int* progress_node_count) {
    search_result_t* r_dijkstra = dijkstra(nodes, *node_count, l->node, progress_node_count);

    for (size_t i = 0; i < *node_count; i++) {
        l->to_nodes[i] = r_dijkstra[i].priority;
    }
    
    delete[] r_dijkstra;
}

void create_landmark_from_nodes(landmark_t* l, const node_t* nodes, const size_t* node_count, int* progress_node_count) {
    const node_t* inverse = create_inverse_graph(nodes, *node_count, progress_node_count);

    search_result_t* r_dijkstra = dijkstra(inverse, *node_count, l->node, progress_node_count);

    delete[] inverse;

    for (size_t i = 0; i < *node_count; i++) {
        (*progress_node_count)++;
        l->from_nodes[i] = r_dijkstra[i].priority;
    }

    delete[] r_dijkstra;
}

void create_landmark(landmark_t* l, const node_idx_t* src_node, const node_t* nodes, const size_t* node_count, int* progress) {
    l->node = *src_node;
    l->from_nodes.resize(*node_count);
    l->to_nodes.resize(*node_count);

    std::thread t_from_nodes(create_landmark_from_nodes, l, nodes, node_count, progress);
    std::thread t_to_nodes(create_landmark_to_nodes, l, nodes, node_count, progress);

    t_from_nodes.join();
    t_to_nodes.join();
}

void print_progress_process(std::vector<int>* progresses, const size_t* divider, const bool* running) {
    while (*running) {

        for (int i : *progresses)
            printf(" %4.1f |", (float) i / *divider * 100);

        printf("\n");

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void store_landmark(FILE* f, const landmark_t* l, int* progress) {
    node_idx_t node_count = (node_idx_t) l->from_nodes.size();
    fprintf(f, "%d %d\n", l->node, node_count);

    for (node_idx_t i = 0; i < l->from_nodes.size(); i++) {
        fprintf(f, "%d %d %d\n", i, l->from_nodes[i], l->to_nodes[i]);
        (*progress)++;
    }
}

void store_landmark(FILE* f, const landmark_t* l) {
    int progress;
    store_landmark(f, l, &progress);
}

void read_lm_line(const char*& str_it, int* node_idx, int* from, int* to) {
    *node_idx = atoi(str_it);

    while (*str_it != ' ') ++str_it;
    while (*str_it == ' ') ++str_it;
    *from = atoi(str_it);

    while (*str_it != ' ') ++str_it;
    while (*str_it == ' ') ++str_it;
    *to = atoi(str_it);
    
    while ((*str_it != '\n') & (*str_it != '\0')) ++str_it;
    
    if (*str_it == '\0')
        return;
    
    while (*str_it == '\n') ++str_it;
}

void read_landmark(FILE* f, landmark_t* dst) {
    node_idx_t node_count;
    
    fscanf(f, "%d %d\n", &dst->node, &node_count);

    dst->from_nodes.resize(node_count);
    dst->to_nodes.resize(node_count);

    long mark = ftell(f);
    fseek(f, 0, SEEK_END);
    long content_size = ftell(f) - mark;
    fseek(f, mark, SEEK_SET);

    char* content = new char[content_size + 1];
    fread(content, sizeof(char), content_size, f);
    content[content_size] = '\0';

    const char* content_it = content;

    while (node_count--) {
        node_idx_t node;
        distance_t dist_from;
        distance_t dist_to;
        
        read_lm_line(content_it, &node, &dist_from, &dist_to);

        dst->from_nodes[node] = dist_from;
        dst->to_nodes[node] = dist_to;
    }

    delete[] content;
}

void read_landmark_into_nodes(FILE* f, node_t* nodes, int lm_idx) {
    node_idx_t node_count;
    node_idx_t src_node;
    fscanf(f, "%d %d\n", &src_node, &node_count);

    long mark = ftell(f);
    fseek(f, 0, SEEK_END);
    long content_size = ftell(f) - mark;
    fseek(f, mark, SEEK_SET);

    char* content = new char[content_size + 1];
    fread(content, sizeof(char), content_size, f);
    content[content_size] = '\0';

    const char* content_it = content;

    while (node_count--) {
        node_idx_t node;
        distance_t dist_from;
        distance_t dist_to;
        
        read_lm_line(content_it, &node, &dist_from, &dist_to);

        nodes[node].distance_from_landmarks[lm_idx] = dist_from;
        nodes[node].distance_to_landmarks[lm_idx] = dist_to;

        // Break if we have reached the end of the file
        // if (*content_it == '\0') break;
    }

    print_lock.lock();
    printf("Done reading landmark %d\n", lm_idx);
    print_lock.unlock();

    delete[] content;
}

void create_and_store_landmark(FILE* f, const node_idx_t* src_node, const node_t* nodes, const size_t* node_count, node_idx_t* progress) {
    landmark_t l;

    create_landmark(&l, src_node, nodes, node_count, progress);
    store_landmark(f, &l, progress);
}

void create_and_store_landmarks(const std::vector<node_idx_t> lm_locations, const char* filename_prefix, const node_t* nodes, const size_t node_count) {
    std::vector<FILE*> files;
    for (int i = 0; i < lm_locations.size(); i++) {
        char buffer[128];
        sprintf(buffer, "%s%d.txt", filename_prefix, i);

        FILE* f = fopen(buffer, "w");

        files.push_back(f);
    }

    std::vector<int> progresses(lm_locations.size());
    for (int& i : progresses)
        i = 0;

    bool running = true;
    const size_t divider = node_count * 5;
    std::thread print_thread(print_progress_process, &progresses, &divider, &running);
    
    
    
    std::vector<std::thread> threads;

    // Create and store each landmark concurrently
    for (int i = 0; i < lm_locations.size(); i++)
        threads.push_back(std::thread(create_and_store_landmark, files[i], &lm_locations[i], nodes, &node_count, &progresses[i]));

    for (std::thread& t : threads)
        t.join();
    
    running = false;
    print_thread.join();

    for (FILE* f : files)
        fclose(f);

    printf("All landmarks successully created and stored\n");
}

void generate_standard_landmarks() {
    printf("Generating 5 landmarks. This process can take a while.\n");

    node_idx_t aabenraa = 5253888; // Sør
    node_idx_t nordkapp = 2531818; // Nord
    node_idx_t ilomantsi = 7021334; // Øst 1
    node_idx_t trinntorp = 4481242; // Øst 2
    node_idx_t ålesund = 2053550; // Vest

    node_idx_t node_count;
    node_t* nodes = read_nodes(&node_count);

    std::vector<node_idx_t> lm_nodes = { aabenraa };
    create_and_store_landmarks(lm_nodes, "Aabenraa", nodes, node_count);
    lm_nodes = { nordkapp };
    create_and_store_landmarks(lm_nodes, "Nordkapp", nodes, node_count);
    lm_nodes = { ilomantsi };
    create_and_store_landmarks(lm_nodes, "Ilomantsi", nodes, node_count);
    lm_nodes = { trinntorp };
    create_and_store_landmarks(lm_nodes, "Trinntorp", nodes, node_count);
    lm_nodes = { ålesund };
    create_and_store_landmarks(lm_nodes, "Ålesund", nodes, node_count);

    delete[] nodes;
}

node_t* read_nodes(FILE* f_node, FILE* f_edge, FILE* f_type, std::vector<FILE*> landmarks, int* node_count) {
    fscanf(f_node, "%d\n", node_count);

    printf("Allocating memory for node data... ");
    node_t* nodes = new node_t[*node_count];
    printf("DONE\n");

    // Set size of landmark vectors
    printf("Allocating memory for landmarks in nodes... ");

    for (node_t* n = nodes; n != nodes + *node_count; n++) {
        n->distance_from_landmarks.resize(landmarks.size());
        n->distance_to_landmarks.resize(landmarks.size());
    }

    printf("DONE\n");
    
    // Start all threads

    std::thread t_node(read_nodes_lon_lat, nodes, node_count, f_node);
    std::thread t_edge(read_nodes_edges, nodes, f_edge);
    std::thread t_info(read_nodes_info, nodes, f_type);

    std::vector<std::thread> t_landmarks;

    for (int i = 0; i < landmarks.size(); i++)
        t_landmarks.push_back(std::thread(read_landmark_into_nodes, landmarks[i], nodes, i));


    // Join on all threads

    print_lock.lock();
    printf("Reading all node data...\n");
    print_lock.unlock();

    for (std::thread& t : t_landmarks)
        t.join();

    t_node.join();
    t_edge.join();
    t_info.join();
    
    printf("Successfully read node data\n");

    // Return result

    return nodes;
}

node_t* read_nodes_with_default_landmarks(node_idx_t* node_count) {
    // Open node files
    FILE* f_node = fopen(node_file_paths[0], "r");
    FILE* f_edge = fopen(node_file_paths[1], "r");
    FILE* f_type = fopen(node_file_paths[2], "r");

    if (!f_node)
        printf("Could not open file %s", node_file_paths[0]);
    if (!f_edge)
        printf("Could not open file %s", node_file_paths[1]);
    if (!f_type)
        printf("Could not open file %s", node_file_paths[2]);
    
    if (!f_node || !f_edge || !f_type)
        return 0;

    // Open landmark files

    std::vector<FILE*> lm_files(landmark_file_paths.size());

    for (int i = 0; i < landmark_file_paths.size(); i++)
        lm_files[i] = fopen(landmark_file_paths[i], "r");
    
    // Perform read on all files concurrently

    node_t* nodes = read_nodes(f_node, f_edge, f_type, lm_files, node_count);
    
    fclose(f_node);
    fclose(f_edge);
    fclose(f_type);

    return nodes;
}

#pragma endregion

// ----------- //





// ----------- //

#pragma region ALT-search

distance_t find_highest_estimate(const node_t& current_node, const node_t& end_node) {
    distance_t estimate = 0;
    const int num_lm = (int) end_node.distance_from_landmarks.size();

    for (int i = 0; i < num_lm; i++) {
        // estimate = delta(L, n) - delta(L, m)
        // int est_from_front  = current_node.distance_from_landmarks[i] - end_node.distance_from_landmarks[i];
        // int est_to_front    = current_node.distance_to_landmarks[i]   - end_node.distance_to_landmarks[i];
        // estimate = delta(L, m) - delta(L, n)
        int est_from_back = end_node.distance_from_landmarks[i] - current_node.distance_from_landmarks[i];
        int est_to_back   = end_node.distance_to_landmarks[i]   - current_node.distance_to_landmarks[i];

        // estimate = std::max(estimate, est_from_front);
        // estimate = std::max(estimate, est_to_front);
        estimate = std::max(estimate, est_from_back);
        estimate = std::max(estimate, est_to_back);
    }
    
    return estimate;
}

search_result_t* ALT(const node_t* nodes, const size_t node_count, const int start_node, const int end_node, int* processed_nodes) {
    search_result_t* result = new search_result_t[node_count];

    for (int i = 0; i < node_count; i++) {
        result[i].node_idx = i;
        result[i].priority = (std::numeric_limits<decltype(search_result_t::priority)>::max)();
        result[i].distance = (std::numeric_limits<decltype(search_result_t::priority)>::max)();
        result[start_node].previous_node = -1;
        result[i].update_id = 0;
    }

    result[start_node].priority = 0;
    result[start_node].distance = 0;

    std::priority_queue<search_result_t, std::vector<search_result_t>, std::greater<search_result_t>> queue;
    queue.push(result[start_node]);

    

    while (!queue.empty()) {
        *processed_nodes += 1;

        search_result_t pop_node = queue.top();
        queue.pop();
        while (pop_node.update_id != result[pop_node.node_idx].update_id) {
            pop_node = queue.top();
            queue.pop();
        }

        if (start_node != end_node && pop_node.node_idx == end_node)
            break;

        const node_idx_t& pop_node_idx = pop_node.node_idx;
        

        for (edge_t e : nodes[pop_node_idx].edges) {
            const search_result_t& dst_node = result[e.to_node_idx];

            const distance_t estimate = find_highest_estimate(nodes[e.to_node_idx], nodes[end_node]);

            search_result_t new_node;
            new_node.previous_node = pop_node_idx;
            new_node.distance = pop_node.distance + e.kjøretid;
            new_node.priority = estimate + new_node.distance;
            new_node.node_idx = e.to_node_idx;
            new_node.update_id = result[new_node.node_idx].update_id + 1;

            if (new_node.distance < result[e.to_node_idx].distance) {
                result[e.to_node_idx] = new_node;
                queue.push(new_node);
            }
        }
    }

    return result;
}

void test_ALT(const node_t* nodes, const int node_count, const int start_node, const int end_node) {
    node_idx_t nodes_processed = 0;

    // printf("Started ALT...");

    auto start = std::chrono::high_resolution_clock::now();
    search_result_t* result = ALT(nodes, node_count, start_node, end_node, &nodes_processed);
    double elapsed = (long double)(std::chrono::high_resolution_clock::now() - start).count() / 1'000'000'000;
    
    // printf("DONE\n");

    int nodes_in_route = 1;
    int curr_node = end_node;
    while (curr_node != start_node) {
        nodes_in_route++;
        curr_node = result[curr_node].previous_node;
    }

    int time = result[end_node].priority / 100;
    int seconds = time % 60;
    int minutes = (time / 60) % 60;
    int hours = (time / 3600);

    print_lock.lock();
    printf("[%7d -> %7d]   nodes: %4d   Time: %02d:%02d:%02d   Proc-nodes: %7d   Proc-time(s): %f\n", start_node, end_node, nodes_in_route, hours, minutes, seconds, nodes_processed, elapsed);
    print_lock.unlock();

    delete[] result;
}

void test_ALT_multiple_data(node_t* nodes, node_idx_t node_count) {
    std::vector<std::pair<int, int>> tests;
    tests.push_back({2800567, 7705656});
    tests.push_back({7705656, 2800567});
    tests.push_back({647826,  136530});
    tests.push_back({136530,  647826});
    tests.push_back({7826348, 2948202});
    tests.push_back({2948202, 7826348});
    tests.push_back({339910,  1853145});
    tests.push_back({1853145,  339910});
    tests.push_back({2503331, 2866570});
    tests.push_back({2866570, 2503331});
    tests.push_back({6441311, 3168086});
    tests.push_back({3168086, 6441311});

    printf("Running ALT-tests...\n");
    for (int i = 0; i < tests.size(); i++)
        test_ALT(nodes, node_count, tests[i].first, tests[i].second);
    printf("DONE\n");
    
    printf("TEST COMPLETE\n");
}

#pragma endregion

// ----------- //





// ----------- //

#pragma region Task-specific functions

search_result_t* dijkstra_modular(const node_t* nodes, const size_t node_count, const int start_node, std::function<bool(const node_t&)> end_condition, std::function<int(const edge_t&)> weight_selector) {
    search_result_t* result = new search_result_t[node_count];

    for (int i = 0; i < node_count; i++) {
        result[i].node_idx = i;
        result[i].priority = (std::numeric_limits<decltype(search_result_t::priority)>::max)();
        result[start_node].previous_node = -1;
        result[i].update_id = 0;
    }

    result[start_node].priority = 0;

    std::priority_queue<search_result_t, std::vector<search_result_t>, std::greater<search_result_t>> queue;
    queue.push(result[start_node]);



    while (!queue.empty()) {
        search_result_t sn = queue.top();
        queue.pop();
        while (sn.update_id != result[sn.node_idx].update_id) {
            sn = queue.top();
            queue.pop();
        }

        if (end_condition(nodes[sn.node_idx]))
            break;

        const int node_idx = sn.node_idx;

        for (edge_t e : nodes[node_idx].edges) {
            search_result_t& dst_node = result[e.to_node_idx];

            search_result_t new_node;
            new_node.previous_node = node_idx;
            new_node.priority = sn.priority + weight_selector(e);
            new_node.node_idx = e.to_node_idx;
            new_node.update_id = result[new_node.node_idx].update_id + 1;

            if (new_node.priority < dst_node.priority) {
                result[e.to_node_idx] = new_node;
                queue.push(new_node);
            }
        }
    }

    return result;
}

void print_path(search_result_t* result, node_t* nodes, node_idx_t start_node, node_idx_t end_node) {
    search_result_t s = result[end_node];
    do {
        printf("%f, %f\n", nodes[s.node_idx].lon, nodes[s.node_idx].lat);
    } while ((s.previous_node >= 0) & ((s = result[s.previous_node]).node_idx != start_node));
}

#define FLAG uint8_t

const FLAG stedsnavn = 1;
const FLAG bensinstasjon = 2;
const FLAG ladestasjon = 4;
const FLAG spisested = 8; 
const FLAG drikkested = 16; 
const FLAG overnattingssted = 32;

std::vector<node_idx_t> find_points_of_interest(node_t* nodes, node_idx_t node_count, node_idx_t start_node, FLAG flag, int count, std::function<int(const edge_t&)> weight_selector) {
    std::vector<node_idx_t> poi;

    search_result_t* r = dijkstra_modular(nodes, node_count, start_node, [&poi, &count, &flag](const node_t& n){
        for (point_of_interest p : n.points_of_interest) {
            if (p.type & flag) {
                poi.push_back(n.idx);
                break;
            }
        }

        return poi.size() >= count;
    }, weight_selector);
    delete[] r;

    return poi;
}

void find_places(node_t* nodes, node_idx_t node_count) {
    const int orkanger = 2266026;
    const int trondheim_camping = 3005466;
    const int hotell_östersund = 3240367;

    std::vector<node_idx_t> orkanger_ladestasjoner = find_points_of_interest(nodes, node_count, orkanger, ladestasjon, 5, [](const edge_t& e) { return e.lengde; });
    std::vector<node_idx_t> trondheim_camping_drikkesteder = find_points_of_interest(nodes, node_count, trondheim_camping, drikkested, 5, [](const edge_t& e) { return e.lengde; });
    std::vector<node_idx_t> hotell_östersund_spisesteder = find_points_of_interest(nodes, node_count, hotell_östersund, spisested, 5, [](const edge_t& e) { return e.lengde; });

    printf("Ladstasjoner nær Orkanger(%d):\n", orkanger);
    for (node_idx_t n : orkanger_ladestasjoner)
        printf("%f, %f\n", nodes[n].lon, nodes[n].lat);
    
    printf("\nDrikkesteder nær Trondheim Camping(%d):\n", trondheim_camping);
    for (node_idx_t n : trondheim_camping_drikkesteder)
        printf("%f, %f\n", nodes[n].lon, nodes[n].lat);

    printf("\nSpisesteder nær Hotell Østersund(%d):\n", hotell_östersund);
    for (node_idx_t n : hotell_östersund_spisesteder)
        printf("%f, %f\n", nodes[n].lon, nodes[n].lat);

}

void find_path_dijkstra(node_t* nodes, node_idx_t node_count, node_idx_t start_node, node_idx_t end_node) {
    int processed_nodes = 0;

    auto start = std::chrono::high_resolution_clock::now();
    search_result_t* r = dijkstra(nodes, node_count, start_node, end_node, &processed_nodes);
    double elapsed = (long double)(std::chrono::high_resolution_clock::now() - start).count() / 1'000'000'000;

    // Send inn reisetid (tt:mm:ss), reiserute, beregningstid og antall noder hentet fra køen for
    // Orkanger–Trondheim og Selbustrand–GreenStar Hotel Lahti. Husk både Dijkstra og ALT
    // for begge turene; noe av poenget er å sammenligne ytelse og arbeidsmengde for de to
    // algoritmene.

    char buff[128]{0};
    sprintf(buff, "Dijkstra_%d_%d.txt", start_node, end_node);
    FILE* f = fopen(buff, "w");

    int nodes_in_route = 0;
    search_result_t s = r[end_node];
    while (s.node_idx != start_node && s.previous_node >= 0) {
        ++nodes_in_route;
        fprintf(f, "%f, %f\n", nodes[s.node_idx].lon, nodes[s.node_idx].lat);
        s = r[s.previous_node];
    }
    fprintf(f, "%f, %f\n", nodes[s.node_idx].lon, nodes[s.node_idx].lat);

    fclose(f);

    distance_t time = r[end_node].priority / 100;
    distance_t hour = (time / 3600);
    distance_t minute = (time / 60) % 60;
    distance_t second = time % 60;

    print_lock.lock();
    printf("[%7d -> %7d] Reisetid: %02d:%02d:%02d Noder fra køen: %7d Beregningstid(s): %f\n", start_node, end_node, hour, minute, second, processed_nodes, elapsed);
    print_lock.unlock();

    delete[] r;
}

void find_path_ALT(node_t* nodes, node_idx_t node_count, node_idx_t start_node, node_idx_t end_node) {
    int processed_nodes = 0;

    auto start = std::chrono::high_resolution_clock::now();
    search_result_t* r = ALT(nodes, node_count, start_node, end_node, &processed_nodes);
    double elapsed = (long double)(std::chrono::high_resolution_clock::now() - start).count() / 1'000'000'000;

    // Send inn reisetid (tt:mm:ss), reiserute, beregningstid og antall noder hentet fra køen for
    // Orkanger–Trondheim og Selbustrand–GreenStar Hotel Lahti. Husk både Dijkstra og ALT
    // for begge turene; noe av poenget er å sammenligne ytelse og arbeidsmengde for de to
    // algoritmene.

    char buff[128]{0};
    sprintf(buff, "ALT_%d_%d.txt", start_node, end_node);
    FILE* f = fopen(buff, "w");

    int nodes_in_route = 0;
    search_result_t s = r[end_node];
    while (s.node_idx != start_node && s.previous_node >= 0) {
        ++nodes_in_route;
        fprintf(f, "%f, %f\n", nodes[s.node_idx].lon, nodes[s.node_idx].lat);
        s = r[s.previous_node];
    }
    fprintf(f, "%f, %f\n", nodes[s.node_idx].lon, nodes[s.node_idx].lat);

    fclose(f);

    distance_t time = r[end_node].priority / 100;
    distance_t hour = (time / 3600);
    distance_t minute = (time / 60) % 60;
    distance_t second = time % 60;

    print_lock.lock();
    printf("[%7d -> %7d] Reisetid: %02d:%02d:%02d Noder fra køen: %7d Beregningstid(s): %f\n", start_node, end_node, hour, minute, second, processed_nodes, elapsed);
    print_lock.unlock();

    delete[] r;
}

void print_paths(node_t* nodes, node_idx_t node_count) {
    printf("Orkanger - Trondheim                [Dijkstra] ");
    find_path_dijkstra(nodes, node_count, 2266026, 7826348); // Orkanger - Trondheim
    
    printf("Orkanger - Trondheim                     [ALT] ");
    find_path_ALT(nodes, node_count, 2266026, 7826348); // Orkanger - Trondheim
    
    printf("Selbustrand - GreenStar Hotel Lahti [Dijkstra] ");
    find_path_dijkstra(nodes, node_count, 5009309, 999080); // Selbustrand - GreenStar Hotel Lahti
    
    printf("Selbustrand - GreenStar Hotel Lahti      [ALT] ");
    find_path_ALT(nodes, node_count, 5009309, 999080); // Selbustrand - GreenStar Hotel Lahti
}

void all_tasks_in_one_go(node_t* nodes, node_idx_t node_count) {
    printf("Denne seksjonen besvarer: \"Send inn reisetid [osv.] for Orkanger-Trondheim og Selbustrand-GreenStar Hotel Lahti\"\n");
    printf("Reiserutene printes til egne filer mens programmet kjører\n\n");
    print_paths(nodes, node_count);

    
    printf("\n\nDenne seksjonen besvarer: \"Programmet kan finne de 5 interessepunktene [...]\"\n");
    find_places(nodes, node_count);

    
    printf("\n\nDenne seksjonen tester algoritmene på rutene som er satt opp i løsningsforslaget.\nFørst Dijkstra, deretter ALT\n\n");
    test_dijkstra_multiple_data(nodes, node_count);
    printf("\n");
    test_ALT_multiple_data(nodes, node_count);
    printf("\n");
}

bool are_node_files_available() {
    FILE* f_noder = fopen(node_file_paths[0], "r");
    FILE* f_kanter = fopen(node_file_paths[1], "r");
    FILE* f_interessepunkter = fopen(node_file_paths[2], "r");

    bool all_available = f_noder && f_kanter && f_interessepunkter;

    if (f_noder)            fclose(f_noder);
    if (f_kanter)           fclose(f_kanter);
    if (f_interessepunkter) fclose(f_interessepunkter);

    return all_available;
}

bool are_all_landmark_files_available() {
    std::vector<FILE*> lm_files(landmark_file_paths.size());

    bool success = true;

    for (int i = 0; i < landmark_file_paths.size(); i++) {
        lm_files[i] = fopen(landmark_file_paths[i], "r");

        if (!lm_files[i]) success = false;
    }

    for (FILE* f : lm_files)
        if (f)
            fclose(f);

    return success;
}

int demo() {
    if (!are_node_files_available()) {
        printf("Fant ikke eller kunne ikke åpne en eller flere av nodefilene :(\nJeg leter etter disse filene:\n");
        for (const char* c : node_file_paths)
            printf("%s\n", c);

        return 1;
    }

    if (!are_all_landmark_files_available()) {
        printf("Det mangler noen landemerke-filer. Genererer dem nå før demonstrasjonen får kjøre...\n\n");
        generate_standard_landmarks();
    }

    int node_count;
    node_t* nodes = read_nodes_with_default_landmarks(&node_count);

    printf("\n\n\n\n");

    all_tasks_in_one_go(nodes, node_count);

    printf("Deallocating node data...");
    delete[] nodes;
    printf("DONE\n");
    printf("\nProcess terminated successfully\n");

    return 0;
}

#pragma endregion


int main() {
    // generate_standard_landmarks();
    // test_dijkstra_multiple_data(); // Run this to test the implementation of Dijkstra's algorithm
    // test_ALT_multiple_data(); // Run this to test the implementation of the ALT algorithm

    return demo();
}
