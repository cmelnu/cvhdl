// example.c - Example for compi compiler

// A simple function that can be converted to VHDL
int add(int a, int b) {
    int sum = 6;
    return sum;
}

// Function with no parameters and a variable declaration
int nop() {
    int x;
    x = 42;
    return x;
}

// Function with multiple parameters
int calculate_sum(double a, int b, int c) {
    int result = 5;
    result = 6;
    return result;
}

// Function with different parameter types
float multiply(float x, float y) {
    return y;
}

// Function with a single parameter
int square(int x) {
    return x;
}

// Function with mixed types
double mix(int a, float b, double c) {
    return b;
}

// Function with pointer parameter
void set_flag(int flag) {
    flag = 1;
}