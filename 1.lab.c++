#include <iostream>
using namespace std;

#include <iostream>
using namespace std;

int main() {
    int n;
    cin >> n; 
    int i = 2;
    bool k = false; 
    
    if (n % i != 0) {
        int j = 3;
        while (j * j < n + 1) {
            if (n % j == 0) {
                k = true; 
                i = j; 
                break; 
            }
            j += 2; 
        }
    } else {
        k = true; 
    }

    
    if (k) {
        cout << (i - 1) * (n / i) << " " << n / i << endl;
    } else {
        cout << 1 << " " << n - 1 << endl;
    }

    return 0;
}