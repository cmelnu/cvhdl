// simple.c - Example for compi compiler

// A simple function that can be converted to VHDL
int add(int a, int b) {
    return a + b;
}
// Another function with void return type
void process_data(int data[], int length) {
    int i;
    for (i = 0; i < length; i++) {
        data[i] = data[i] * 2;
    }
}

// Function with multiple parameters
int calculate_sum(int a, int b, int c) {
    int sum = a + b + c;
    return sum;
}