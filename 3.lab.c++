//Shabarov Vladimir 090304-RPIa-024

#include <iostream>
#include <vector>
#include <queue>
#include <utility>
#include <list>
#include <chrono>
#include <random>
#include <cmath>
using namespace std;
using namespace std::chrono;

class PerformanceMeasure {
private:
    high_resolution_clock::time_point start_time;
    long long operations_count = 0;
    double flops_per_operation = 10.0;

public:
    void start() {
        start_time = high_resolution_clock::now();
        operations_count = 0;
    }

    void incrementOperations(long long count = 1) {
        operations_count += count;
    }

    double getGFlops() const {
        auto end_time = high_resolution_clock::now();
        double duration = duration_cast<nanoseconds>(end_time - start_time).count() / 1e9;
        if (duration == 0) return 0;
        return (operations_count * flops_per_operation) / (duration * 1e9);
    }

    double getTimeMs() const {
        auto end_time = high_resolution_clock::now();
        return duration_cast<microseconds>(end_time - start_time).count() / 1000.0;
    }
};

class Maze {
private:
    int N, M;
    vector<vector<int>> grid;

public:
    void initFromInput() {
        cout << "Enter maze dimensions N and M: ";
        cin >> N >> M;

        if (N <= 0 || M <= 0) throw runtime_error("Maze dimensions must be positive");
        
        grid.resize(N, vector<int>(M));
        
        cout << "Enter maze (0 for path, 1 for wall), row by row:\n";
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < M; ++j) {
                cin >> grid[i][j];
            }
        }
    }

    void generateRandom(int n, int m, double wall_probability = 0.3) {
        N = n;
        M = m;
        grid.resize(N, vector<int>(M));

        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<> dis(0, 1);

        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < M; ++j) {
                grid[i][j] = (dis(gen) < wall_probability) ? 1 : 0;
            }
        }

        int entries = min(N, M);
        for (int i = 0; i < entries; ++i) {
            grid[0][i] = 0;
            grid[N-1][i] = 0;
        }
    }

    const vector<vector<int>>& getGrid() const { return grid; }
    int getN() const { return N; }
    int getM() const { return M; }

    void print() const {
        cout << "Maze layout (" << N << "x" << M << "):\n";
        for (const auto& row : grid) {
            for (int cell : row) {
                cout << cell << " ";
            }
            cout << endl;
        }
    }
};

class MazeSolver {
protected:
    const Maze& maze;
    vector<vector<bool>> global_used;
    mutable PerformanceMeasure perf;

    bool is_valid(int x, int y, const vector<vector<bool>>& visited) const {
        perf.incrementOperations(6);
        return x >= 0 && x < maze.getN() && y >= 0 && y < maze.getM() 
               && maze.getGrid()[x][y] == 0 && !visited[x][y] && !global_used[x][y];
    }

public:
    MazeSolver(const Maze& m) : maze(m) {
        global_used.resize(maze.getN(), vector<bool>(maze.getM(), false));
    }

    virtual bool solve_a() = 0;
    virtual bool solve_b() = 0;
    virtual string implementationName() const = 0;

    void resetGlobalUsed() {
        global_used.assign(maze.getN(), vector<bool>(maze.getM(), false));
    }

    void performanceTest() {
        cout << implementationName() << " Results:\n";
        
        perf.start();
        bool a_result = solve_a();
        double a_time = perf.getTimeMs();
        double a_gflops = perf.getGFlops();
        
        perf.start();
        bool b_result = solve_b();
        double b_time = perf.getTimeMs();
        double b_gflops = perf.getGFlops();

        cout << "  Solution A: " << (a_result ? "Possible" : "Impossible") 
             << " (" << a_time << " ms, " << a_gflops << " GFLOPs)\n";
        cout << "  Solution B: " << (b_result ? "Possible" : "Impossible") 
             << " (" << b_time << " ms, " << b_gflops << " GFLOPs)\n";
    }
};

class ArraySolver : public MazeSolver {
private:
    static const int MAX_N = 100;
    static const int MAX_M = 100;

    bool bfs(const pair<int, int>& start, const pair<int, int>& end, bool mark_used) {
        vector<vector<bool>> visited(maze.getN(), vector<bool>(maze.getM(), false));
        queue<pair<int, int>> q;
        q.push(start);
        visited[start.first][start.second] = true;
        perf.incrementOperations(5);

        int dx[] = {-1, 1, 0, 0};
        int dy[] = {0, 0, -1, 1};

        while (!q.empty()) {
            auto [x, y] = q.front();
            q.pop();
            perf.incrementOperations(3);

            if (x == end.first && y == end.second) {
                if (mark_used) {
                    for (int i = 0; i < maze.getN(); ++i) {
                        for (int j = 0; j < maze.getM(); ++j) {
                            if (visited[i][j]) global_used[i][j] = true;
                            perf.incrementOperations(3);
                        }
                    }
                }
                perf.incrementOperations(2);
                return true;
            }

            for (int i = 0; i < 4; ++i) {
                int nx = x + dx[i];
                int ny = y + dy[i];
                perf.incrementOperations(2);

                if (is_valid(nx, ny, visited)) {
                    visited[nx][ny] = true;
                    q.push({nx, ny});
                    perf.incrementOperations(3);
                }
            }
        }
        return false;
    }

public:
    ArraySolver(const Maze& m) : MazeSolver(m) {
        if (maze.getN() > MAX_N || maze.getM() > MAX_M) 
            throw runtime_error("Size too large for array implementation");
    }

    bool solve_a() override {
        resetGlobalUsed();
        int entries = min(maze.getN(), maze.getM());
        for (int i = 0; i < entries; ++i) {
            if (maze.getGrid()[0][i] != 0 || maze.getGrid()[maze.getN()-1][i] != 0) 
                return false;
            
            if (!bfs({0, i}, {maze.getN()-1, i}, false))
                return false;
        }
        return true;
    }

    bool solve_b() override {
        resetGlobalUsed();
        int entries = min(maze.getN(), maze.getM());
        for (int i = 0; i < entries; ++i) {
            if (maze.getGrid()[0][i] != 0) return false;
            bool found = false;
            for (int j = 0; j < entries; ++j) {
                if (maze.getGrid()[maze.getN()-1][j] == 0 && 
                    bfs({0, i}, {maze.getN()-1, j}, false)) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
        return true;
    }

    string implementationName() const override { return "Array Implementation"; }
};

class LinkedListSolver : public MazeSolver {
private:
    struct Node {
        int x, y;
        Node* next;
        Node* prev;
    };

    Node* create_node(int x, int y) {
        perf.incrementOperations(5);
        return new Node{x, y, nullptr, nullptr};
    }

    bool bfs(const pair<int, int>& start, const pair<int, int>& end, bool mark_used) {
        vector<vector<bool>> visited(maze.getN(), vector<bool>(maze.getM(), false));
        Node* front = create_node(start.first, start.second);
        Node* rear = front;
        visited[start.first][start.second] = true;
        perf.incrementOperations(5);

        int dx[] = {-1, 1, 0, 0};
        int dy[] = {0, 0, -1, 1};

        while (front != nullptr) {
            Node* current = front;
            front = front->next;
            perf.incrementOperations(3);
            
            int x = current->x;
            int y = current->y;

            if (x == end.first && y == end.second) {
                if (mark_used) {
                    for (int i = 0; i < maze.getN(); ++i) {
                        for (int j = 0; j < maze.getM(); ++j) {
                            if (visited[i][j]) global_used[i][j] = true;
                            perf.incrementOperations(3);
                        }
                    }
                }
                while (front != nullptr) {
                    Node* temp = front;
                    front = front->next;
                    delete temp;
                    perf.incrementOperations(4);
                }
                delete current;
                perf.incrementOperations(2);
                return true;
            }

            for (int i = 0; i < 4; ++i) {
                int nx = x + dx[i];
                int ny = y + dy[i];
                perf.incrementOperations(2);

                if (is_valid(nx, ny, visited)) {
                    visited[nx][ny] = true;
                    Node* new_node = create_node(nx, ny);
                    if (front == nullptr) {
                        front = rear = new_node;
                        perf.incrementOperations(2);
                    } else {
                        rear->next = new_node;
                        new_node->prev = rear;
                        rear = new_node;
                        perf.incrementOperations(4);
                    }
                }
            }
            delete current;
            perf.incrementOperations(1);
        }
        return false;
    }

public:
    LinkedListSolver(const Maze& m) : MazeSolver(m) {}

    bool solve_a() override {
        resetGlobalUsed();
        int entries = min(maze.getN(), maze.getM());
        for (int i = 0; i < entries; ++i) {
            if (maze.getGrid()[0][i] != 0 || maze.getGrid()[maze.getN()-1][i] != 0) 
                return false;
            
            if (!bfs({0, i}, {maze.getN()-1, i}, false))
                return false;
        }
        return true;
    }

    bool solve_b() override {
        resetGlobalUsed();
        int entries = min(maze.getN(), maze.getM());
        for (int i = 0; i < entries; ++i) {
            if (maze.getGrid()[0][i] != 0) return false;
            bool found = false;
            for (int j = 0; j < entries; ++j) {
                if (maze.getGrid()[maze.getN()-1][j] == 0 && 
                    bfs({0, i}, {maze.getN()-1, j}, false)) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
        return true;
    }

    string implementationName() const override { return "Linked List Implementation"; }
};

class STLSolver : public MazeSolver {
public:
    STLSolver(const Maze& m) : MazeSolver(m) {}

    bool bfs(const pair<int, int>& start, const pair<int, int>& end, bool mark_used) {
        vector<vector<bool>> visited(maze.getN(), vector<bool>(maze.getM(), false));
        queue<pair<int, int>> q;
        q.push(start);
        visited[start.first][start.second] = true;
        perf.incrementOperations(5);

        int dx[] = {-1, 1, 0, 0};
        int dy[] = {0, 0, -1, 1};

        while (!q.empty()) {
            auto [x, y] = q.front();
            q.pop();
            perf.incrementOperations(3);

            if (x == end.first && y == end.second) {
                if (mark_used) {
                    for (int i = 0; i < maze.getN(); ++i) {
                        for (int j = 0; j < maze.getM(); ++j) {
                            if (visited[i][j]) global_used[i][j] = true;
                            perf.incrementOperations(3);
                        }
                    }
                }
                return true;
            }

            for (int i = 0; i < 4; ++i) {
                int nx = x + dx[i];
                int ny = y + dy[i];
                perf.incrementOperations(2);

                if (is_valid(nx, ny, visited)) {
                    visited[nx][ny] = true;
                    q.push({nx, ny});
                    perf.incrementOperations(3);
                }
            }
        }
        return false;
    }

    bool solve_a() override {
        resetGlobalUsed();
        int entries = min(maze.getN(), maze.getM());
        for (int i = 0; i < entries; ++i) {
            if (maze.getGrid()[0][i] != 0 || maze.getGrid()[maze.getN()-1][i] != 0) 
                return false;
            
            if (!bfs({0, i}, {maze.getN()-1, i}, false))
                return false;
        }
        return true;
    }

    bool solve_b() override {
        resetGlobalUsed();
        int entries = min(maze.getN(), maze.getM());
        for (int i = 0; i < entries; ++i) {
            if (maze.getGrid()[0][i] != 0) return false;
            bool found = false;
            for (int j = 0; j < entries; ++j) {
                if (maze.getGrid()[maze.getN()-1][j] == 0 && 
                    bfs({0, i}, {maze.getN()-1, j}, false)) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
        return true;
    }

    string implementationName() const override { return "STL Implementation"; }
};

void compareImplementations(Maze& maze) {
    cout << "\n=== Performance Comparison ===\n";
    
    try {
        ArraySolver arraySolver(maze);
        arraySolver.performanceTest();
    } catch (const exception& e) {
        cout << "Array implementation error: " << e.what() << endl;
    }

    try {
        LinkedListSolver linkedListSolver(maze);
        linkedListSolver.performanceTest();
    } catch (const exception& e) {
        cout << "Linked List implementation error: " << e.what() << endl;
    }

    try {
        STLSolver stlSolver(maze);
        stlSolver.performanceTest();
    } catch (const exception& e) {
        cout << "STL implementation error: " << e.what() << endl;
    }
}

int main() {
    int choice;
    cout << "Choose input method:\n";
    cout << "1. Manual input\n";
    cout << "2. Random generated maze (50x50)\n";
    cout << "Enter choice: ";
    cin >> choice;

    Maze maze;

    try {
        if (choice == 1) {
            maze.initFromInput();
        } else {
            maze.generateRandom(50, 50);
        }
        maze.print();
        compareImplementations(maze);
    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
