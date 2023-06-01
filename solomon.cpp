#include <iostream>
#include <vector>
#include <cstring>

using namespace std;

const int MAXN = 100; // максимальное количество символов в сообщении
int K; // количество символов, используемых для кодирования
int N; // общее количество символов (K + M), где M - количество символов для коррекции ошибок

int gf_add(int a, int b) {
    return a ^ b;
}

int gf_mul(int a, int b) {
    int p = 0;
    while (b) {
        if (b & 1) p ^= a;
        if (a & 0x80) a = (a << 1) ^ 0x11d;
        else a <<= 1;
        b >>= 1;
    }
    return p;
}

void encode(vector<int>& message, vector<int>& code) {
    for (int i = 0; i < K; ++i) {
        code[i] = message[i];
        for (int j = 0; j < N - K; ++j) {
            code[i] ^= gf_mul(code[K + j], gf_add(1, gf_mul(i + 1, j + 1)));
        }
        
    }

}

bool decode(vector<int>& received, vector<int>& decoded) {
    vector<vector<int>> matrix(N - K, vector<int>(N - K));
    vector<int> syndrome(N - K);
    vector<int> error_locator(N - K + 2);
    vector<int> error_evaluator(N - K + 2);
    vector<int> error_positions(N - K + 2);
    int num_errors = 0;

    // формирование матрицы Слейтера
    for (int i = 0; i < N - K; ++i) {
        for (int j = 0; j < N - K; ++j) {
            matrix[i][j] = gf_mul(gf_add(1, i + j), gf_add(1, gf_mul(i + 1, j + 1)));
        }
    }

    // вычисление синдрома
    for (int i = 0; i < N - K; ++i) {
        syndrome[i] = received[K + i];
        for (int j = 0; j < K; ++j) {
            syndrome[i] ^= gf_mul(received[j], gf_add(1, gf_mul(i + 1, j + 1)));
        }
    }

    // поиск ошибок
    for (int i = 0; i < N - K; ++i) {
        error_locator[i + 1] = error_evaluator[i + 1] = 0;
        for (int j = 0; j < N - K; ++j) {
            error_locator[i + 1] ^= gf_mul(matrix[i][j], error_evaluator[j + 1]);
        }
        error_locator[i + 1] = gf_mul(error_locator[i + 1], syndrome[i]);
        if (error_locator[i + 1]) num_errors++;
    }

    if (!num_errors) {
        // нет ошибок
        for (int i = 0; i < K; ++i) {
            decoded[i] = received[i];
        }
        return true;
    }

    // поиск позиций ошибок
    int j = 0;
    for (int i = 1; i <= N - K && j < num_errors; ++i) {
        int q = 1;
        for (int k = 1; k <= j; ++k) {
            q = gf_mul(q, error_positions[k] ^ i);
        }
        if (!error_locator[i]) {
            error_positions[++j] = i;
            error_evaluator[j] = 0;
        } else {
            error_evaluator[j + 1] = 0;
            for (int k = 1; k <= j; ++k) {
                error_evaluator[j + 1] ^= gf_mul(error_evaluator[k], gf_add(error_positions[k], i));
            }
            error_evaluator[j + 1] = gf_add(error_evaluator[j + 1], error_locator[i]);
        }
    }

    // исправление ошибок
    for (int i = 0; i < K; ++i) {
        decoded[i] = received[i];
        for (int j = 1; j <= num_errors; ++j) {
            decoded[i] ^= gf_mul(error_evaluator[j], gf_add(1, gf_mul(i + 1, error_positions[j])));
        }
    }

    return true;
}

int main() {
    vector<int> message(MAXN);
    vector<int> decoded(MAXN);

    // ввод сообщения
    cout << "Enter message: ";
    string s;
    getline(cin, s);
    K = s.length();
    N = K+2;
    for (int i = 0; i < K; ++i) {
        message[i] = s[i];
    }
    vector<int> code(N);
    vector<int> received(N);
    // кодирование
    encode(message, code);
    
    // передача по каналу связи
    for (int i = 0; i < N; ++i) {
        received[i] = code[i];
        cout<<received[i];
    }
    //received[2]=5;
    //received[7]=3;
    // восстановление сообщения
    if (decode(received, decoded)) {
        cout << "\nDecoded message: ";
        for (int i = 0; i < K; ++i) {
            cout << char(decoded[i]);
        }
        cout << endl;
    } else {
        cout << "Failed to decode message" << endl;
    }

    return 0;
}