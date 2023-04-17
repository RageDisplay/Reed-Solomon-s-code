import random

def generate_polynomial(degree, prime):
    #Генерирует полином заданной степени в конечном поле
    coefficients = [random.randint(0, prime-1) for _ in range(degree)]
    coefficients.append(1) # коэффициент при x^degree
    return coefficients

def evaluate_polynomial(polynomial, x, prime):
    #Вычисляет значение полинома в точке x в конечном поле
    result = 0
    for coefficient in reversed(polynomial):
        result *= x
        result += coefficient
        result %= prime
    return result

def encode_message(msg, n, k, prime):
    #Кодирует сообщение в код Рида-Соломона
    message = [ord(char) for char in msg]
    encoded = []
    polynomial = generate_polynomial(k-1, prime)
    for i in range(n):
        if i < k:
            encoded.append(message[i])
        else:
            value = evaluate_polynomial(polynomial, i+1-k, prime)
            encoded.append(value ^ message[i])
    return encoded

def find_error_locator(syndromes, prime):
    #Находит локатор ошибок
    n = len(syndromes) + 1
    b = [1] + [0] * (n-1)
    c = [1] + [0] * (n-1)
    for i in range(n-1):
        b_next = [0] * n
        for j in range(i+1):
            b_next[j] = (b[j] - c[i-j] * syndromes[i]) % prime
        if 2 * i < n-1:
            c_next = c[:i+2] + [0] * (n-i-2)
        else:
            c_next = b[:n-i-1] + [0] * (i+1-n%2)
        for j in range(i+2):
            c_next[j] = (c_next[j] + b[i+1-j]) % prime
        b, c = b_next, c_next
    return b

def find_errors(error_locator, codeword, prime):
    #Находит позиции ошибок
    n = len(codeword)
    k = n - len(error_locator) + 1
    errors = []
    for i in range(n):
        if evaluate_polynomial(error_locator, pow(i+1, prime-2, prime), prime) == 0:
            continue
        value = 0
        for j in range(len(error_locator)):
            value += error_locator[j] * evaluate_polynomial([1, pow(i+1,j,prime)], 1, prime)
            value %= prime
        if value == 0:
            errors.append(i)
    return errors

def decode_message(codeword, n, k, prime):
    #Декодирует код Рида-Соломона
    syndromes = []
    polynomial = generate_polynomial(n-k, prime)
    for i in range(n-k):
        value = evaluate_polynomial(polynomial, i+1, prime)
        syndrome = 0
        for j in range(n):
            syndrome += codeword[j] * evaluate_polynomial([1, pow(i+1,j,prime)], 1, prime)
            syndrome %= prime
        syndromes.append(syndrome)
    error_locator = find_error_locator(syndromes, prime)
    errors = find_errors(error_locator, codeword, prime)
    for i in range(n):
        if i < k:
            continue
        else:
            for j in range(len(errors)):
                if i == errors[j]:
                    codeword[i] ^= evaluate_polynomial(error_locator, pow(j+1, prime-2, prime), prime)
                    break
    message = ''.join([chr(char) for char in codeword[:k]])
    return message


# Пример использования
prime = 999
msg = "Hello world"
n = len(msg) #размер кодировки
k = len(msg) #количество символов на декод
codeword = encode_message(msg, n, k, prime)
print(codeword)
decoded_msg = decode_message(codeword, n, k, prime)
print(decoded_msg) # выводит "Hello world"