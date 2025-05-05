#include <iostream>
#include <complex>
#include <chrono>
#include <cstdlib>
#include <algorithm>
#include <mkl.h>

const int SIZE = 2048;
const int BLOCK_SIZE = 128;
using Complex = std::complex<double>;

void generate_matrices(Complex* A, Complex* B) {
    #pragma omp parallel for
    for (int i = 0; i < SIZE * SIZE; ++i) {
        A[i] = Complex((rand() % 100) - 50.0, (rand() % 100) - 50.0);
        B[i] = Complex((rand() % 100) - 50.0, (rand() % 100) - 50.0);
    }
}

void simple_matrix_multiply(const Complex* A, const Complex* B, Complex* C) {
    #pragma omp parallel for
    for (int i = 0; i < SIZE; ++i) {
        for (int k = 0; k < SIZE; ++k) {
            Complex temp = A[i * SIZE + k];
            for (int j = 0; j < SIZE; ++j) {
                C[i * SIZE + j] += temp * B[k * SIZE + j];
            }
        }
    }
}

void mkl_matrix_multiply(const Complex* A, const Complex* B, Complex* C) {
    cblas_zgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                SIZE, SIZE, SIZE,
                &Complex(1.0, 0.0),
                A, SIZE,
                B, SIZE,
                &Complex(0.0, 0.0),
                C, SIZE);
}

void chosen_block_multiply(const Complex* A, const Complex* B, Complex* C, int blockSize) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < SIZE; i += blockSize) {
        for (int j = 0; j < SIZE; j += blockSize) {
            for (int k = 0; k < SIZE; k += blockSize) {
               
                const int i_end = std::min(i + blockSize, SIZE);
                const int j_end = std::min(j + blockSize, SIZE);
                const int k_end = std::min(k + blockSize, SIZE);
                
                for (int ii = i; ii < i_end; ii += 4) {
                    for (int kk = k; kk < k_end; ++kk) {
                        for (int jj = j; jj < j_end; jj += 4) {
                            
                            C[ii*SIZE + jj] += A[ii*SIZE + kk] * B[kk*SIZE + jj];
                            C[ii*SIZE + jj+1] += A[ii*SIZE + kk] * B[kk*SIZE + jj+1];
                            C[ii*SIZE + jj+2] += A[ii*SIZE + kk] * B[kk*SIZE + jj+2];
                            C[ii*SIZE + jj+3] += A[ii*SIZE + kk] * B[kk*SIZE + jj+3];
                            
                            C[(ii+1)*SIZE + jj] += A[(ii+1)*SIZE + kk] * B[kk*SIZE + jj];
                            C[(ii+1)*SIZE + jj+1] += A[(ii+1)*SIZE + kk] * B[kk*SIZE + jj+1];
                            C[(ii+1)*SIZE + jj+2] += A[(ii+1)*SIZE + kk] * B[kk*SIZE + jj+2];
                            C[(ii+1)*SIZE + jj+3] += A[(ii+1)*SIZE + kk] * B[kk*SIZE + jj+3];
                            
                            C[(ii+2)*SIZE + jj] += A[(ii+2)*SIZE + kk] * B[kk*SIZE + jj];
                            C[(ii+2)*SIZE + jj+1] += A[(ii+2)*SIZE + kk] * B[kk*SIZE + jj+1];
                            C[(ii+2)*SIZE + jj+2] += A[(ii+2)*SIZE + kk] * B[kk*SIZE + jj+2];
                            C[(ii+2)*SIZE + jj+3] += A[(ii+2)*SIZE + kk] * B[kk*SIZE + jj+3];
                            
                            C[(ii+3)*SIZE + jj] += A[(ii+3)*SIZE + kk] * B[kk*SIZE + jj];
                            C[(ii+3)*SIZE + jj+1] += A[(ii+3)*SIZE + kk] * B[kk*SIZE + jj+1];
                            C[(ii+3)*SIZE + jj+2] += A[(ii+3)*SIZE + kk] * B[kk*SIZE + jj+2];
                            C[(ii+3)*SIZE + jj+3] += A[(ii+3)*SIZE + kk] * B[kk*SIZE + jj+3];
                        }
                    }
                }
            }
        }
    }
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    Complex* A = static_cast<Complex*>(mkl_malloc(SIZE * SIZE * sizeof(Complex), 64));
    Complex* B = static_cast<Complex*>(mkl_malloc(SIZE * SIZE * sizeof(Complex), 64));
    Complex* C = static_cast<Complex*>(mkl_malloc(SIZE * SIZE * sizeof(Complex), 64));

    generate_matrices(A, B);

    // Тестирование простого алгоритма
    std::fill(C, C + SIZE * SIZE, Complex(0.0, 0.0));
    auto start = std::chrono::high_resolution_clock::now();
    simple_matrix_multiply(A, B, C);
    auto end = std::chrono::high_resolution_clock::now();
    double simple_time = std::chrono::duration<double>(end - start).count();
    double flops = 8.0 * SIZE * SIZE * SIZE;
    std::cout << "Simple multiplication:\n";
    std::cout << "  Time: " << simple_time << " s\n";
    std::cout << "  Performance: " << (flops / simple_time) * 1e-9 << " GFlops\n\n";

    // Тестирование MKL
    std::fill(C, C + SIZE * SIZE, Complex(0.0, 0.0));
    start = std::chrono::high_resolution_clock::now();
    mkl_matrix_multiply(A, B, C);
    end = std::chrono::high_resolution_clock::now();
    double mkl_time = std::chrono::duration<double>(end - start).count();
    std::cout << "MKL multiplication:\n";
    std::cout << "  Time: " << mkl_time << " s\n";
    std::cout << "  Performance: " << (flops / mkl_time) * 1e-9 << " GFlops\n\n";

    // Тестирование алгоритма по выбору (блочный)
    std::fill(C, C + SIZE * SIZE, Complex(0.0, 0.0));
    start = std::chrono::high_resolution_clock::now();
    chosen_block_multiply(A, B, C, BLOCK_SIZE);
    end = std::chrono::high_resolution_clock::now();
    double chosen_time = std::chrono::duration<double>(end - start).count();
    std::cout << "Algorithm of choice (blocked):\n";
    std::cout << "  Time: " << chosen_time << " s\n";
    std::cout << "  Performance: " << (flops / chosen_time) * 1e-9 << " GFlops\n\n";

    mkl_free(A);
    mkl_free(B);
    mkl_free(C);

    return 0;
}
