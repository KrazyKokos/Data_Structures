#include <iostream>
#include <queue>
#include <vector>
#include <memory>
#include <cstring>
#include <stdexcept>
#include <chrono>
#include <random>
#include <functional>

using namespace std;
using namespace std::chrono;

constexpr int WALL_PROBABILITY = 25;

class MazeGenerator {
public:
    static void generateMaze(vector<vector<int>>& maze) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, 99);

        for (auto& row : maze) {
            for (auto& cell : row) {
                cell = (dis(gen) < WALL_PROBABILITY) ? 1 : 0;
            }
        }
        
        maze[0][0] = 0;
        maze.back().back() = 0;
    }
};

class IMazeSolver {
public:
    virtual ~IMazeSolver() = default;
    virtual void setMaze(const vector<vector<int>>& input) = 0;
    virtual bool solve(pair<int, int> start, pair<int, int> end) = 0;
    virtual string getName() const = 0;
};

class ArrayMazeSolver : public IMazeSolver {
private:
    static const int MAX_SIZE = 1000;
    int maze[MAX_SIZE][MAX_SIZE];
    bool visited[MAX_SIZE][MAX_SIZE];
    int N, M;

    void validateCoordinates(int x, int y) const {
        if (x < 0 || x >= N || y < 0 || y >= M) {
            throw out_of_range("Coordinates out of maze bounds");
        }
    }

public:
    ArrayMazeSolver(int n, int m) : N(n), M(m) {
        if (n <= 0 || m <= 0 || n > MAX_SIZE || m > MAX_SIZE) {
            throw invalid_argument("Invalid maze dimensions");
        }
        memset(maze, 0, sizeof(maze));
        memset(visited, false, sizeof(visited));
    }

    void setMaze(const vector<vector<int>>& input) override {
        if (input.size() != static_cast<size_t>(N) || input[0].size() != static_cast<size_t>(M)) {
            throw invalid_argument("Input maze dimensions don't match solver dimensions");
        }
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < M; ++j) {
                maze[i][j] = input[i][j];
            }
        }
    }

    bool solve(pair<int, int> start, pair<int, int> end) override {
        validateCoordinates(start.first, start.second);
        validateCoordinates(end.first, end.second);

        if (maze[start.first][start.second] == 1 || maze[end.first][end.second] == 1) {
            return false;
        }

        bool temp_visited[MAX_SIZE][MAX_SIZE];
        memcpy(temp_visited, visited, sizeof(visited));

        queue<pair<int, int>> q;
        q.push(start);
        visited[start.first][start.second] = true;

        constexpr int dx[] = {-1, 1, 0, 0};
        constexpr int dy[] = {0, 0, -1, 1};

        while (!q.empty()) {
            auto [x, y] = q.front();
            q.pop();

            if (x == end.first && y == end.second) {
                memcpy(visited, temp_visited, sizeof(visited));
                return true;
            }

            for (int i = 0; i < 4; ++i) {
                int nx = x + dx[i];
                int ny = y + dy[i];

                if (nx >= 0 && nx < N && ny >= 0 && ny < M && 
                    maze[nx][ny] == 0 && !visited[nx][ny]) {
                    visited[nx][ny] = true;
                    q.push({nx, ny});
                }
            }
        }

        memcpy(visited, temp_visited, sizeof(visited));
        return false;
    }

    string getName() const override { return "Array implementation"; }
};

class LinkedListMazeSolver : public IMazeSolver {
private:
    struct Node {
        int x, y;
        shared_ptr<Node> next;
        Node(int x, int y) : x(x), y(y), next(nullptr) {}
    };

    vector<vector<shared_ptr<Node>>> grid;
    vector<vector<bool>> visited;
    int N, M;

    void validateCoordinates(int x, int y) const {
        if (x < 0 || x >= N || y < 0 || y >= M) {
            throw out_of_range("Coordinates out of maze bounds");
        }
    }

public:
    LinkedListMazeSolver(int n, int m) : N(n), M(m), 
        grid(n, vector<shared_ptr<Node>>(m, nullptr)), 
        visited(n, vector<bool>(m, false)) {
        if (n <= 0 || m <= 0) {
            throw invalid_argument("Invalid maze dimensions");
        }
    }

    void setMaze(const vector<vector<int>>& input) override {
        if (input.size() != static_cast<size_t>(N) || input[0].size() != static_cast<size_t>(M)) {
            throw invalid_argument("Input maze dimensions don't match solver dimensions");
        }
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < M; ++j) {
                if (input[i][j] == 0) {
                    grid[i][j] = make_shared<Node>(i, j);
                } else {
                    grid[i][j] = nullptr;
                }
            }
        }
    }

    bool solve(pair<int, int> start, pair<int, int> end) override {
        validateCoordinates(start.first, start.second);
        validateCoordinates(end.first, end.second);

        if (grid[start.first][start.second] == nullptr || grid[end.first][end.second] == nullptr) {
            return false;
        }

        auto temp_visited = visited;

        queue<pair<int, int>> q;
        q.push(start);
        visited[start.first][start.second] = true;

        constexpr int dx[] = {-1, 1, 0, 0};
        constexpr int dy[] = {0, 0, -1, 1};

        while (!q.empty()) {
            auto [x, y] = q.front();
            q.pop();

            if (x == end.first && y == end.second) {
                visited = temp_visited;
                return true;
            }

            for (int i = 0; i < 4; ++i) {
                int nx = x + dx[i];
                int ny = y + dy[i];

                if (nx >= 0 && nx < N && ny >= 0 && ny < M && 
                    grid[nx][ny] != nullptr && !visited[nx][ny]) {
                    visited[nx][ny] = true;
                    q.push({nx, ny});
                }
            }
        }

        visited = temp_visited;
        return false;
    }

    string getName() const override { return "Linked list implementation"; }
};

class STLMazeSolver : public IMazeSolver {
private:
    vector<vector<int>> maze;
    vector<vector<bool>> visited;
    int N, M;

    void validateCoordinates(int x, int y) const {
        if (x < 0 || x >= N || y < 0 || y >= M) {
            throw out_of_range("Coordinates out of maze bounds");
        }
    }

public:
    STLMazeSolver(int n, int m) : N(n), M(m), 
        maze(n, vector<int>(m, 0)), 
        visited(n, vector<bool>(m, false)) {
        if (n <= 0 || m <= 0) {
            throw invalid_argument("Invalid maze dimensions");
        }
    }

    void setMaze(const vector<vector<int>>& input) override {
        if (input.size() != static_cast<size_t>(N) || input[0].size() != static_cast<size_t>(M)) {
            throw invalid_argument("Input maze dimensions don't match solver dimensions");
        }
        maze = input;
    }

    bool solve(pair<int, int> start, pair<int, int> end) override {
        validateCoordinates(start.first, start.second);
        validateCoordinates(end.first, end.second);

        if (maze[start.first][start.second] == 1 || maze[end.first][end.second] == 1) {
            return false;
        }

        auto temp_visited = visited;

        queue<pair<int, int>> q;
        q.push(start);
        visited[start.first][start.second] = true;

        constexpr int dx[] = {-1, 1, 0, 0};
        constexpr int dy[] = {0, 0, -1, 1};

        while (!q.empty()) {
            auto [x, y] = q.front();
            q.pop();

            if (x == end.first && y == end.second) {
                visited = temp_visited;
                return true;
            }

            for (int i = 0; i < 4; ++i) {
                int nx = x + dx[i];
                int ny = y + dy[i];

                if (nx >= 0 && nx < N && ny >= 0 && ny < M && 
                    maze[nx][ny] == 0 && !visited[nx][ny]) {
                    visited[nx][ny] = true;
                    q.push({nx, ny});
                }
            }
        }

        visited = temp_visited;
        return false;
    }

    string getName() const override { return "STL vector implementation"; }
};

class Benchmark {
public:
    static void run(int N, int M) {
        vector<vector<int>> maze(N, vector<int>(M));
        MazeGenerator::generateMaze(maze);
        pair<int, int> start = {0, 0};
        pair<int, int> end = {N-1, M-1};

        cout << "Benchmark results for " << N << "x" << M << " maze:\n";

        vector<unique_ptr<IMazeSolver>> solvers;
        solvers.push_back(make_unique<ArrayMazeSolver>(N, M));
        solvers.push_back(make_unique<LinkedListMazeSolver>(N, M));
        solvers.push_back(make_unique<STLMazeSolver>(N, M));

        for (auto& solver : solvers) {
            try {
                solver->setMaze(maze);
                
                auto start_time = high_resolution_clock::now();
                bool result = solver->solve(start, end);
                auto duration = duration_cast<microseconds>(high_resolution_clock::now() - start_time);
                
                cout << solver->getName() << ": " << duration.count() << " μs, "
                     << (result ? "Path found" : "No path") << endl;
            } catch (const exception& e) {
                cerr << solver->getName() << " error: " << e.what() << endl;
            }
        }
    }
};

int main() {
    try {
        int N, M;
        cout << "Enter maze dimensions (N M): ";
        cin >> N >> M;

        if (N <= 0 || M <= 0) {
            throw invalid_argument("Dimensions must be positive integers");
        }

        Benchmark::run(N, M);

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
