//Shabarov Vladimir 090304-RPIa-024

#include <iostream>
#include <vector>
#include <queue>
#include <utility>
#include <list>
using namespace std;


class MazeBase {
protected:
    int N, M;
    vector<vector<int>> grid;
    vector<vector<bool>> global_used; 

    bool is_valid(int x, int y, const vector<vector<bool>>& visited) const {
        return x >= 0 && x < N && y >= 0 && y < M 
               && grid[x][y] == 0 && !visited[x][y] && !global_used[x][y];
    }

public:
    virtual void init(int n, int m, const vector<pair<int, int>>& entries, 
                     const vector<pair<int, int>>& exits) = 0;
    virtual bool solve_a() = 0;
    virtual bool solve_b() = 0;
};

// 1. Массив 
class MazeArray : public MazeBase {
private:
    static const int MAX_N = 100;
    static const int MAX_M = 100;
    int array_grid[MAX_N][MAX_M];
    bool array_used[MAX_N][MAX_M];

public:
    void init(int n, int m, const vector<pair<int, int>>& entries, 
             const vector<pair<int, int>>& exits) override {
        N = n;
        M = m;
        if (N > MAX_N || M > MAX_M) throw runtime_error("Size too large for array implementation");
        
        grid.resize(N, vector<int>(M));
        global_used.resize(N, vector<bool>(M, false));
        
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < M; ++j) {
                cin >> array_grid[i][j];
                grid[i][j] = array_grid[i][j];
            }
        }
    }

    bool bfs(const pair<int, int>& start, const pair<int, int>& end, bool mark_used) {
        vector<vector<bool>> visited(N, vector<bool>(M, false));
        queue<pair<int, int>> q;
        q.push(start);
        visited[start.first][start.second] = true;

        int dx[] = {-1, 1, 0, 0};
        int dy[] = {0, 0, -1, 1};

        while (!q.empty()) {
            auto [x, y] = q.front();
            q.pop();

            if (x == end.first && y == end.second) {
                if (mark_used) {
                    for (int i = 0; i < N; ++i) {
                        for (int j = 0; j < M; ++j) {
                            if (visited[i][j]) global_used[i][j] = true;
                        }
                    }
                }
                return true;
            }

            for (int i = 0; i < 4; ++i) {
                int nx = x + dx[i];
                int ny = y + dy[i];
                if (is_valid(nx, ny, visited)) {
                    visited[nx][ny] = true;
                    q.push({nx, ny});
                }
            }
        }
        return false;
    }

    bool solve_a() override {
        int k = min(N, M);
        for (int i = 0; i < k; ++i) {
            if (grid[0][i] != 0 || grid[N-1][i] != 0) return false;
            if (!bfs({0, i}, {N-1, i}, true)) return false;
        }
        return true;
    }

    bool solve_b() override {
        int k = min(N, M);
        for (int i = 0; i < k; ++i) {
            if (grid[0][i] != 0) return false;
            bool found = false;
            for (int j = 0; j < M; ++j) {
                if (grid[N-1][j] == 0 && bfs({0, i}, {N-1, j}, true)) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
        return true;
    }
};

// 2. Связанный список
class MazeLinkedList : public MazeBase {
private:
    struct Node {
        int x, y;
        Node* next;
        Node* prev;
    };

    Node* create_node(int x, int y) {
        Node* new_node = new Node{x, y, nullptr, nullptr};
        return new_node;
    }

public:
    void init(int n, int m, const vector<pair<int, int>>& entries,
             const vector<pair<int, int>>& exits) override {
        N = n;
        M = m;
        grid.resize(N, vector<int>(M));
        global_used.resize(N, vector<bool>(M, false));
        
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < M; ++j) {
                cin >> grid[i][j];
            }
        }
    }

    bool bfs(const pair<int, int>& start, const pair<int, int>& end, bool mark_used) {
        vector<vector<bool>> visited(N, vector<bool>(M, false));
        Node* front = create_node(start.first, start.second);
        Node* rear = front;
        visited[start.first][start.second] = true;

        int dx[] = {-1, 1, 0, 0};
        int dy[] = {0, 0, -1, 1};

        while (front != nullptr) {
            Node* current = front;
            front = front->next;
            
            int x = current->x;
            int y = current->y;

            if (x == end.first && y == end.second) {
                if (mark_used) {
                    for (int i = 0; i < N; ++i) {
                        for (int j = 0; j < M; ++j) {
                            if (visited[i][j]) global_used[i][j] = true;
                        }
                    }
                }
                while (front != nullptr) {
                    Node* temp = front;
                    front = front->next;
                    delete temp;
                }
                delete current;
                return true;
            }

            for (int i = 0; i < 4; ++i) {
                int nx = x + dx[i];
                int ny = y + dy[i];
                if (is_valid(nx, ny, visited)) {
                    visited[nx][ny] = true;
                    Node* new_node = create_node(nx, ny);
                    if (front == nullptr) {
                        front = rear = new_node;
                    } else {
                        rear->next = new_node;
                        new_node->prev = rear;
                        rear = new_node;
                    }
                }
            }
            delete current;
        }
        return false;
    }

    bool solve_a() override {
        int k = min(N, M);
        for (int i = 0; i < k; ++i) {
            if (grid[0][i] != 0 || grid[N-1][i] != 0) return false;
            if (!bfs({0, i}, {N-1, i}, true)) return false;
        }
        return true;
    }

    bool solve_b() override {
        int k = min(N, M);
        for (int i = 0; i < k; ++i) {
            if (grid[0][i] != 0) return false;
            bool found = false;
            for (int j = 0; j < M; ++j) {
                if (grid[N-1][j] == 0 && bfs({0, i}, {N-1, j}, true)) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
        return true;
    }
};

// 3. STL
class MazeSTL : public MazeBase {
public:
    void init(int n, int m, const vector<pair<int, int>>& entries,
             const vector<pair<int, int>>& exits) override {
        N = n;
        M = m;
        grid.resize(N, vector<int>(M));
        global_used.resize(N, vector<bool>(M, false));
        
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < M; ++j) {
                cin >> grid[i][j];
            }
        }
    }

    bool bfs(const pair<int, int>& start, const pair<int, int>& end, bool mark_used) {
        vector<vector<bool>> visited(N, vector<bool>(M, false));
        queue<pair<int, int>> q;
        q.push(start);
        visited[start.first][start.second] = true;

        int dx[] = {-1, 1, 0, 0};
        int dy[] = {0, 0, -1, 1};

        while (!q.empty()) {
            auto [x, y] = q.front();
            q.pop();

            if (x == end.first && y == end.second) {
                if (mark_used) {
                    for (int i = 0; i < N; ++i) {
                        for (int j = 0; j < M; ++j) {
                            if (visited[i][j]) global_used[i][j] = true;
                        }
                    }
                }
                return true;
            }

            for (int i = 0; i < 4; ++i) {
                int nx = x + dx[i];
                int ny = y + dy[i];
                if (is_valid(nx, ny, visited)) {
                    visited[nx][ny] = true;
                    q.push({nx, ny});
                }
            }
        }
        return false;
    }

    bool solve_a() override {
        int k = min(N, M);
        for (int i = 0; i < k; ++i) {
            if (grid[0][i] != 0 || grid[N-1][i] != 0) return false;
            if (!bfs({0, i}, {N-1, i}, true)) return false;
        }
        return true;
    }

    bool solve_b() override {
        int k = min(N, M);
        for (int i = 0; i < k; ++i) {
            if (grid[0][i] != 0) return false;
            bool found = false;
            for (int j = 0; j < M; ++j) {
                if (grid[N-1][j] == 0 && bfs({0, i}, {N-1, j}, true)) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
        return true;
    }
};

int main() {
    int n, m, k;
    cout << "Enter N, M and k: ";
    cin >> n >> m >> k;

    cout << "Enter maze (0 for path, 1 for wall):\n";
    
   
  
    vector<pair<int, int>> entries, exits;
    for (int i = 0; i < k; ++i) {
        entries.emplace_back(0, i);
        exits.emplace_back(n-1, i);
    }

  
    try {
        MazeArray mazeArray;
        mazeArray.init(n, m, entries, exits);
        cout << "Array Solution A: " << (mazeArray.solve_a() ? "Possible" : "Impossible") << endl;
        cout << "Array Solution B: " << (mazeArray.solve_b() ? "Possible" : "Impossible") << endl;
    } catch (const exception& e) {
        cout << "Array implementation error: " << e.what() << endl;
    }

    MazeLinkedList mazeLinkedList;
    mazeLinkedList.init(n, m, entries, exits);
    cout << "Linked List Solution A: " << (mazeLinkedList.solve_a() ? "Possible" : "Impossible") << endl;
    cout << "Linked List Solution B: " << (mazeLinkedList.solve_b() ? "Possible" : "Impossible") << endl;

    MazeSTL mazeSTL;
    mazeSTL.init(n, m, entries, exits);
    cout << "STL Solution A: " << (mazeSTL.solve_a() ? "Possible" : "Impossible") << endl;
    cout << "STL Solution B: " << (mazeSTL.solve_b() ? "Possible" : "Impossible") << endl;

    return 0;
}
