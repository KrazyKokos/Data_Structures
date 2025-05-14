//Shabarov Vladimir 090304-RPIa-024

#include <iostream>
#include <complex>
#include <chrono>
#include <cstdlib>
#include <algorithm>
#include <cblas.h>

using namespace std;
using namespace std::chrono;

const int SIZE = 2048;
const int BLOCK_SIZE = 64;
using Complex = complex<double>;

void generate_matrices(Complex* A, Complex* B) {
    srand(42);
    for (int i = 0; i < SIZE * SIZE; ++i) {
        A[i] = Complex((rand() % 100) - 50.0, (rand() % 100) - 50.0);
        B[i] = Complex((rand() % 100) - 50.0, (rand() % 100) - 50.0);
    }
}

// Проверка матриц на равенство 
bool are_matrices_equal(const Complex* A, const Complex* B, double epsilon = 1e-6) {
    for (int i = 0; i < SIZE * SIZE; ++i) {
        if (abs(A[i].real() - B[i].real()) > epsilon || 
            abs(A[i].imag() - B[i].imag()) > epsilon) {
            return false;
        }
    }
    return true;
}

// Простое матричное умножение
void simple_matrix_multiply(const Complex* A, const Complex* B, Complex* C) {
    for (int i = 0; i < SIZE; ++i) {
        for (int k = 0; k < SIZE; ++k) {
            Complex temp = A[i * SIZE + k];
            for (int j = 0; j < SIZE; ++j) {
                C[i * SIZE + j] += temp * B[k * SIZE + j];
            }
        }
    }
}

// Умножение с использованием OpenBLAS
void openblas_matrix_multiply(const Complex* A, const Complex* B, Complex* C) {
    const Complex one(1.0, 0.0);
    const Complex zero(0.0, 0.0);
    cblas_zgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                SIZE, SIZE, SIZE,
                &one,
                A, SIZE,
                B, SIZE,
                &zero,
                C, SIZE);
}

void transpose_matrix(const Complex* A, Complex* AT) {
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            AT[j * SIZE + i] = A[i * SIZE + j];
        }
    }
}

// Блочное умножение с транспонированием 
void blocked_transposed_multiply(const Complex* A, const Complex* B, Complex* C, int blockSize) {
    Complex* BT = new Complex[SIZE * SIZE];
    transpose_matrix(B, BT);
    
    for (int ii = 0; ii < SIZE; ii += blockSize) {
        for (int jj = 0; jj < SIZE; jj += blockSize) {
            for (int kk = 0; kk < SIZE; kk += blockSize) {
                const int i_end = min(ii + blockSize, SIZE);
                const int j_end = min(jj + blockSize, SIZE);
                const int k_end = min(kk + blockSize, SIZE);
                
                for (int i = ii; i < i_end; ++i) {
                    for (int j = jj; j < j_end; ++j) {
                        Complex sum(0.0, 0.0);
                        for (int k = kk; k < k_end; ++k) {
                            sum += A[i * SIZE + k] * BT[j * SIZE + k];
                        }
                        C[i * SIZE + j] += sum;
                    }
                }
            }
        }
    }
    
    delete[] BT;
}

int main() {
    Complex* A = new Complex[SIZE * SIZE];
    Complex* B = new Complex[SIZE * SIZE];
    Complex* C_simple = new Complex[SIZE * SIZE];
    Complex* C_openblas = new Complex[SIZE * SIZE];
    Complex* C_blocked = new Complex[SIZE * SIZE];

    generate_matrices(A, B);
    const double flops = 8.0 * SIZE * SIZE * SIZE;

    // Тестирование простого алгоритма
    fill(C_simple, C_simple + SIZE * SIZE, Complex(0.0, 0.0));
    auto start = high_resolution_clock::now();
    simple_matrix_multiply(A, B, C_simple);
    auto end = high_resolution_clock::now();
    double simple_time = duration<double>(end - start).count();
    cout << "Simple multiplication:\n";
    cout << "  Time: " << simple_time << " s\n";
    cout << "  Performance: " << (flops / simple_time) * 1e-9 << " GFlops\n\n";

    // Тестирование OpenBLAS
    fill(C_openblas, C_openblas + SIZE * SIZE, Complex(0.0, 0.0));
    start = high_resolution_clock::now();
    openblas_matrix_multiply(A, B, C_openblas);
    end = high_resolution_clock::now();
    double openblas_time = duration<double>(end - start).count();
    cout << "OpenBLAS multiplication:\n";
    cout << "  Time: " << openblas_time << " s\n";
    cout << "  Performance: " << (flops /openblas_time) * 1e-9 << " GFlops\n\n";

    if (are_matrices_equal(C_simple, C_openblas)) {
        cout << "Simple and OpenBLAS results match!\n\n";
    } else {
        cout << "WARNING: Simple and OpenBLAS results differ!\n\n";
    }

    // Тестирование блочного алгоритма с транспонированием
    fill(C_blocked, C_blocked + SIZE * SIZE, Complex(0.0, 0.0));
    start = high_resolution_clock::now();
    blocked_transposed_multiply(A, B, C_blocked, BLOCK_SIZE);
    end = high_resolution_clock::now();
    double blocked_time = duration<double>(end - start).count();
    cout << "Blocked transposed multiplication:\n";
    cout << "  Time: " << blocked_time << " s\n";
    cout << "  Performance: " << (flops / blocked_time) * 1e-9 << " GFlops\n\n";

    if (are_matrices_equal(C_blocked, C_openblas)) {
        cout << "Blocked and OpenBLAS results match!\n\n";
    } else {
        cout << "WARNING: Blocked and OpenBLAS results differ!\n\n";
    }

    delete[] A;
    delete[] B;
    delete[] C_simple;
    delete[] C_openblas;
    delete[] C_blocked;

    return 0;
}
