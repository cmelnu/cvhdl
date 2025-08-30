struct Point { int x; int y; };

int add_points(struct Point p) { return p.x + p.y; }

struct Point main_fn(int a) {
    struct Point p;
    p.x = a;
    p.y = 5;
    return p;
}
