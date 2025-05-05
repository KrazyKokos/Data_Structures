//Shabarov Vladimir 090304-RPIa-024

#include <iostream>
#include <queue>
#include <vector>
#include <utility>
#include <chrono>
#include <cstring>
#include <algorithm>

using namespace std;
using namespace std::chrono;

const int MAX_N = 100;
const int MAX_M = 100;

int maze_array[MAX_N][MAX_M];
bool visited_global[MAX_N][MAX_M];

// Решение задачи a (фиксированные входы/выходы)
bool solve_part_a(int N, int M, const vector<pair<int, int>>& entries, const vector<pair<int, int>>& exits) {
    if (entries.size() != exits.size()) return false;

    bool temp_visited[MAX_N][MAX_M];
    memcpy(temp_visited, visited_global, sizeof(visited_global));

    for (size_t i = 0; i < entries.size(); ++i) {
        bool local_visited[MAX_N][MAX_M] = {false};
        queue<pair<int, int>> q;
        q.push(entries[i]);
        local_visited[entries[i].first][entries[i].second] = true;

        const int dx[] = {-1, 1, 0, 0};
        const int dy[] = {0, 0, -1, 1};

        bool found = false;
        while (!q.empty() && !found) {
            auto [x, y] = q.front();
            q.pop();

            if (x == exits[i].first && y == exits[i].second) {
                found = true;
                for (int i = 0; i < N; ++i) {
                    for (int j = 0; j < M; ++j) {
                        if (local_visited[i][j]) visited_global[i][j] = true;
                    }
                }
                break;
            }

            for (int dir = 0; dir < 4; ++dir) {
                int nx = x + dx[dir];
                int ny = y + dy[dir];

                if (nx >= 0 && nx < N && ny >= 0 && ny < M && 
                    maze_array[nx][ny] == 0 && !local_visited[nx][ny] && !visited_global[nx][ny]) {
                    local_visited[nx][ny] = true;
                    q.push({nx, ny});
                }
            }
        }
        if (!found) {
            memcpy(visited_global, temp_visited, sizeof(visited_global));
            return false;
        }
    }
    return true;
}

// Решение задачи b (любые выходы)
bool solve_part_b(int N, int M, const vector<pair<int, int>>& entries, const vector<pair<int, int>>& exits) {
    bool temp_visited[MAX_N][MAX_M];
    memcpy(temp_visited, visited_global, sizeof(visited_global));

    for (const auto& entry : entries) {
        bool local_visited[MAX_N][MAX_M] = {false};
        queue<pair<pair<int, int>, vector<pair<int, int>>>> q;
        q.push({entry, {}});
        local_visited[entry.first][entry.second] = true;

        const int dx[] = {-1, 1, 0, 0};
        const int dy[] = {0, 0, -1, 1};

        bool found = false;
        while (!q.empty() && !found) {
            auto [current, path] = q.front();
            auto [x, y] = current;
            q.pop();

            if (any_of(exits.begin(), exits.end(), [x, y](const pair<int, int>& exit) {
                return x == exit.first && y == exit.second;
            })) {
                found = true;
                for (const auto& p : path) {
                    visited_global[p.first][p.second] = true;
                }
                visited_global[x][y] = true;
                break;
            }

            for (int dir = 0; dir < 4; ++dir) {
                int nx = x + dx[dir];
                int ny = y + dy[dir];

                if (nx >= 0 && nx < N && ny >= 0 && ny < M && 
                    maze_array[nx][ny] == 0 && !local_visited[nx][ny] && !visited_global[nx][ny]) {
                    local_visited[nx][ny] = true;
                    vector<pair<int, int>> new_path = path;
                    new_path.emplace_back(x, y);
                    q.push({{nx, ny}, new_path});
                }
            }
        }
        if (!found) {
            memcpy(visited_global, temp_visited, sizeof(visited_global));
            return false;
        }
    }
    return true;
}

int main() {
    int N, M;
    cout << "Введите размеры лабиринта (N M): ";
    cin >> N >> M;

    if (N <= 0 || M <= 0 || N > MAX_N || M > MAX_M) {
        cerr << "Некорректные размеры лабиринта!\n";
        return 1;
    }

    vector<vector<int>> maze(N, vector<int>(M));
    cout << "Введите лабиринт (0 - проход, 1 - стена):\n";
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < M; ++j) {
            cin >> maze[i][j];
            maze_array[i][j] = maze[i][j];
        }
    }

  
    vector<pair<int, int>> entries, exits;
    for (int j = 0; j < M; ++j) {
        if (maze[0][j] == 0) entries.emplace_back(0, j);
        if (maze[N-1][j] == 0) exits.emplace_back(N-1, j);
    }
    for (int i = 1; i < N-1; ++i) {
        if (maze[i][0] == 0) entries.emplace_back(i, 0);
        if (maze[i][M-1] == 0) exits.emplace_back(i, M-1);
    }

    if (entries.empty() || exits.empty()) {
        cout << "Нет входов или выходов в лабиринте!\n";
        return 1;
    }

    // Задача a
    memset(visited_global, false, sizeof(visited_global));
    auto start_a = high_resolution_clock::now();
    bool result_a = solve_part_a(N, M, entries, exits);
    auto stop_a = high_resolution_clock::now();
    auto duration_a = duration_cast<microseconds>(stop_a - start_a);

    // Задача b
    memset(visited_global, false, sizeof(visited_global));
    auto start_b = high_resolution_clock::now();
    bool result_b = solve_part_b(N, M, entries, exits);
    auto stop_b = high_resolution_clock::now();
    auto duration_b = duration_cast<microseconds>(stop_b - start_b);

    cout << "\nРезультаты:\n";
    cout << "Задача a (фиксированные входы/выходы): " << (result_a ? "Yes" : "No") 
         << ", время: " << duration_a.count() << " мкс\n";
    cout << "Задача b (любые выходы): " << (result_b ? "Yes" : "No") 
         << ", время: " << duration_b.count() << " мкс\n";

    return 0;
}
