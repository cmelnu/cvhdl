int add(int a, int b) {
    int sum = 6;
    return sum+6+7 == 6;
}

int nop() {
    int x;
    x = x / 42;
    return -x;
}

int calculate_sum(double a, int b, int c) {

    int i = 0;
    double arr[3] = {1,2,3};
    char word[4] = {'w', 'o', 'r', 'd'};
    arr[2] = 6;
    int f = ~5;
    if (c == 0 || b == 5 && a != 10){
        arr[2] = 78;
        return f;
    }
    else if (!c){
        f = ~67;
    }

    return arr[i];
}

int bitwise(int x, int y) {
    return x^y;
}

int square(int x) {
    double y = x + x;
    if (x ^ 0 == 8) {
        y = x;
    } else if (-y + x > -0.25) {
        y = 1;
    } else {
        y = -1;
    }
    return y;
}

double mix(int a, float b, double c) {
    return b;
}

void set_flag(int flag) {
    flag = 1;
}

int while_nested_loop(int outer_limit, int inner_limit) {
    int i = 0;
    int total = 0;
    while (i < outer_limit) {
        int j = 0;
        while (j < inner_limit) {
            if (j == 2) {
                j = j + 1;
                continue;
            }
            if (i + j > 10) {
                break;
            }
            total = total + i + j;
            j = j + 1;
        }
        i = i + 1;
    }
    return total;
}

// New function to test 'for' loop support
int for_loop_sum(int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            sum = sum + i + j;
        }
    }
    return sum;
}

