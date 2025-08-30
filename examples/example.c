int add(int a, int b) {
    return a+b;
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

// Simple function call test exercising call parsing
int test_call() {
    int r = add(2, 3);
    return r;
}


// Complex arithmetic calculation function for VHDL translation
int complex_operation(int a, int b, int c) {
    int res;
    res = (a * b) + (b / (c + 1)) - (a ^ c) + ((a + b + c) * 2);
    res = res + ((a & b) << 2) - ((b | c) >> 1);
    return res;
}

// Test function exercising:
//  - variable names ending with _v and _v1 (collision guard & numbering)
//  - multiple sequential assignments to same variable (process variable semantics)
//  - array whose name ends with _v (left as signal, expect note comment)
//  - repeated function calls producing instantiations and chained assignments
int variable_collision_demo(int a, int b) {
    int foo_v = a;          // name already ends with _v -> internal will become foo_v0
    int foo_v1 = foo_v + 1; // ends with _v1 -> internal will append 0 (likely foo_v10)
    int accum = 0;
    int arr_v[2] = { foo_v, foo_v1 }; // array name ends with _v (kept as signal)

    accum = add(foo_v, b);      // first call
    accum = accum + add(a, foo_v1); // second call (multiple assignments)
    accum = accum + arr_v[0] + arr_v[1];
    accum = accum * 2;          // another sequential assignment
    return accum + foo_v + foo_v1; // use declared identifiers only
}

// Even more complex stress test combining loops, bitwise ops, logical ops, function calls,
// multiple sequential assignments, early loop exits, continues, and name collision cases.
int mega_stress(int a, int b, int n) {
    int acc_v = 0;          // triggers _v0 variable name internally
    int acc_v1 = a ^ b;     // triggers _v10 variable name internally
    int data_v[4] = {1,2,3,4}; // array name ends with _v (kept as signal)
    int i = 0;
    while (i < n) {
        int j = 0;
        while (j < 4) {
            acc_v = acc_v + data_v[j]; // sequential accumulation
            if ((i & 1) && (j == 2)) {
                acc_v1 = acc_v1 + add(i, j); // function call inside condition body
            }
            if (i + j > 10) {
                break; // break inner loop
            }
            j = j + 1;
        }
        if (i == 3) {
            i = i + 1;
            continue; // skip rest of outer body for this iteration
        }
        acc_v = acc_v + ((a & b) << 1) - ((a | b) >> 1); // bitwise mix
        i = i + 1;
    }
    // for-loop accumulation with shadowed k
    for (int k = 0; k < 4; k++) {
        acc_v1 = acc_v1 + data_v[k];
    }
    // Combine with complex_operation result
    acc_v = acc_v + complex_operation(a, b, n);
    return acc_v + acc_v1;
}