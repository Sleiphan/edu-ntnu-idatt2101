


#include <iostream>
#include <vector>
#include <chrono>
#include <string>



template <class T>
class que {
private:
    typedef struct _node {
        T val;
        _node* next;
    } node;

    node* head = 0;
    node* tail = 0;

public:
    que() {
        head = new node;
        head->next = head;
        tail = head;
    }

    ~que() {
        node* n = head;
        node* end = head;

        do {
            node* temp = n;
            n = n->next;
            delete temp;
        } while (n != end);
    }

    inline void put(T val) {
        if (head->next == tail) { // Check if we have no more space
            head->next = new node;
            head->next->next = tail;
        }
        
        head = head->next; // Move head forward.
        head->val = val; // Set value in this node.
    }

    inline T next() {
        if (tail != head)
            tail = tail->next;
        return tail->val;
    }

    inline bool empty() { return head == tail; }
};





template <class T>
class stack {
private:
    typedef struct node {
        T val;
        node* previous;

        node(T v) : node(v, 0) {}
        node(T v, node* p) : val(v), previous(p) {}
    } node;

    node*  head  = 0;
    size_t _size = 0;

public:
    ~stack() {
        while (head) {
            node* n = head;
            head = head->previous;
            delete n;
        }
    }

    void push(T val) {
        // Create a new node that points back to the previous head
        head = new node(val, head);

        // Increment the size counter
        _size++;
    }

    T pop() {
        // Copy the head node's value
        T pop_val = head->val;
        
        // Copy the pionter to the head node
        node* current = head;

        // Move the head back one position
        head = head->previous;

        // Delete the current head node
        delete current;

        // Decrement the size counter
        _size--;

        // Return the value of the deleted head
        return pop_val;
    }

    T peek() const { return head->val; }

    size_t size() const { return _size; }
};





typedef struct edge {
    int dst = 0; // The destination node this edge connects to.
    edge* next = 0; // This struct is meant to be stored as a linked list.
} edge;

typedef struct node {
    edge* edges = 0; // The first edge in the linked list of this node's edges.

    ~node() {
        while (edges != 0) {
            edge* temp = edges;
            edges = edges->next;
            delete temp;
        }
    }
} node;

typedef struct bfs_result {
    int precursor = -1;
    int distance = -1;
} bfs_result;

class directed_graph {
private:
    node* nodes;
    int _size;

public:
    directed_graph(FILE* f) {
        int edge_count;
        fscanf(f, "%d %d\n", &_size, &edge_count);

        nodes = new node[_size];

        int from, to;

        while (edge_count-- && fscanf(f, "%d %d\n", &from, &to) == 2)
            nodes[from].edges = new edge { to, nodes[from].edges };
    }

    ~directed_graph() { delete[] nodes; }

    const int& size() const { return _size; }

    void bfs(int start_node, bfs_result* r) {
        que<int> que;

        for (int i = 0; i < _size; i++) r[i].distance = -1; // Set all distances to 'unassigned' status value.

        que.put(start_node); // Add our start node to the que.
        r[start_node].distance = 0;
        int nodes_left = 1; // Keeps track of how many nodes are left in the que to process. We start with only one, aka. the start node.

        int current_distance = 0; // Tracks the distance from the starting node.
        
        while (nodes_left) {
            int nodes_added = 0; // Keeps track of how many additional nodes we add to the que for further processing.
            current_distance++; // The distance from the starting node increases each interation

            while (nodes_left--) { // Iterate through the nodes in the que.
                int node_idx = que.next(); // The index (or ID) of the current node.

                edge* edge_it = nodes[node_idx].edges; // The iterator we use to iterate over all the edges of the current node.

                while (edge_it != nullptr) { // Iterate over every edge of the current node.
                    int dst = edge_it->dst; // The index of the node the current edge points towards.
                    edge_it = edge_it->next; // Step to the next edge

                    if (r[dst].distance != -1)
                        continue;

                    // Add all unset nodes to the que
                    que.put(dst);
                    nodes_added++;
                    r[dst].distance = current_distance;
                    r[dst].precursor = node_idx;
                }
            }
            
            nodes_left = nodes_added; // The amount of nodes left is equal the amount of nodes that were just added to the que.
        }
    }

    void topological_sort(int* result) {
        stack<int> depth;

        int* r = result + _size;

        bool* visited = new bool[_size] {0};

        for (int i = 0; i < _size; i++) {
            if (visited[i])
                continue;
            
            depth.push(i);

            while (depth.size()) {
                int node = depth.peek();
                edge* e = nodes[node].edges;

                while (e && visited[e->dst]) // Skip to the first unvisited connected node
                    e = e->next;

                if (e == nullptr) {
                    *(--r) = depth.pop();
                    visited[node] = true;
                }
                else
                    depth.push(e->dst);
            }
        }
        
        delete[] visited;
    }

};



const char* file_paths[] = {
    "ø6g1.txt", // 0
    "ø6g2.txt", // 1
    "ø6g5.txt", // 2 <--
    "ø6g6.txt", // 3 <--
};

bool test_bfs(const char* file_path) {
    FILE* f = fopen(file_path, "r");

    if (!f) {
        std::cout << "Error opening file: " + std::string(file_path) + "\n";
        return false;
    }

    directed_graph graph(f);
    fclose(f);

    std::vector<bfs_result> r(graph.size());

    // Perform bfs
    for (int i = 0; i < graph.size(); i++)
        graph.bfs(i, r.data());

    return true;
}

void test_all_bfs() {
    for (int i = 0; i < 4; i++)
        test_bfs(file_paths[i]);
}

void bfs_example(const char* file_path, int start_node) {
    FILE* f = fopen(file_path, "r");

    if (!f) {
        std::cout << "Error opening file: " + std::string(file_path) + "\n";
        return;
    }

    directed_graph graph(f);
    fclose(f);

    std::vector<bfs_result> results(graph.size());

    auto t_start = std::chrono::high_resolution_clock::now();
    graph.bfs(start_node, results.data());
    long double duration = (long double) (std::chrono::high_resolution_clock::now() - t_start).count() / 1'000'000'000;
    
    std::cout << "BFS on " + std::string(file_path) + " from node " + std::to_string(start_node) + ":\n";
    std::cout << std::to_string(duration) + " seconds \n";

    std::cout << " Node | Distance | Prev\n";
    for (int i = 0; i < graph.size(); i++)
        printf(" %4d |   %4d   | %4d\n", i, results[i].distance, results[i].precursor);
    
}



void topo_example(const char* file_path) {
    FILE* f = fopen(file_path, "r");

    if (!f) {
        std::cout << "Error opening file: " + std::string(file_path) + "\n";
        return;
    }

    directed_graph graph(f);
    fclose(f);

    std::vector<int> results(graph.size());

    auto t_start = std::chrono::high_resolution_clock::now();
    graph.topological_sort(results.data());
    long double duration = (long double) (std::chrono::high_resolution_clock::now() - t_start).count() / 1'000'000'000;
    
    std::cout << "Topological sort on " + std::string(file_path) + " from node 0:\n";
    std::cout << std::to_string(duration) + " seconds \n";
    std::cout << " Topological sort: ";
    for (int i = 0; i < results.size(); i++)
        printf("%4d ", results[i]);
    std::cout << std::endl;
}





int main() {
    test_all_bfs();

    bfs_example(file_paths[0], 5);
    std::cout << std::endl;

    topo_example(file_paths[2]);
    std::cout << std::endl;

    return 0;
}