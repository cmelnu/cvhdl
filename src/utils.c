#include "utils.h"

ArrayInfo g_arrays[128];
int g_array_count = 0;

// Helper function to print tree branches for AST visualization
void print_tree_prefix(int level, int is_last) {
    for (int i = 0; i < level; i++) {
        printf("%s", (i == level - 1) ? (is_last ? "└── " : "├── ") : "    ");
    }
}

int find_array_size(const char *name) {
    if (!name) return -1;
    for (int i = 0; i < g_array_count; i++) {
        if (strcmp(g_arrays[i].name, name) == 0) return g_arrays[i].size;
    }
    return -1;
}

void register_array(const char *name, int size) {
    if (!name || size <= 0) return;
    if (g_array_count >= (int)(sizeof(g_arrays) / sizeof(g_arrays[0]))) return;
    // Prevent duplicates
    for (int i = 0; i < g_array_count; i++) {
        if (strcmp(g_arrays[i].name, name) == 0) { g_arrays[i].size = size; return; }
    }
    strncpy(g_arrays[g_array_count].name, name, sizeof(g_arrays[g_array_count].name) - 1);
    g_arrays[g_array_count].name[sizeof(g_arrays[g_array_count].name) - 1] = '\0';
    g_arrays[g_array_count].size = size;
    g_array_count++;
}

int is_number_str(const char *s) {
    if (!s || !*s) return 0;
    const char *p = s;
    if (*p == '+' || *p == '-') p++;
    if (!*p) return 0;
    while (*p) { if (!isdigit((unsigned char)*p)) return 0; p++; }
    return 1;
}

// Add this helper function above generate_vhdl:
int is_negative_literal(const char* value) {
    if (!value || value[0] != '-' || strlen(value) < 2)
        return 0;
    // Accept -123, -1.23, -0.5, -123.0, -y, -var, etc.
    int i = 1;
    int has_valid = 0;
    while (value[i]) {
        if (isdigit(value[i]) || value[i] == '.' || isalpha(value[i]) || value[i] == '_') {
            has_valid = 1;
        } else {
            return 0;
        }
        i++;
    }
    return has_valid;
}

int get_precedence(const char *op) {
    // Distinctly separate unknown/null from real operators
    if (!op) return -999;
    // Higher number = higher precedence (mirrors C precedence ordering)
    if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0) return 7;
    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0) return 6;
    if (strcmp(op, "<<") == 0 || strcmp(op, ">>") == 0) return 5;
    if (strcmp(op, "<") == 0 || strcmp(op, "<=") == 0 ||
        strcmp(op, ">") == 0 || strcmp(op, ">=") == 0) return 4;
    if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0) return 3;
    if (strcmp(op, "&") == 0) return 2;
    if (strcmp(op, "^") == 0) return 1;
    if (strcmp(op, "|") == 0) return 0;
    if (strcmp(op, "&&") == 0) return -1; // logical AND
    if (strcmp(op, "||") == 0) return -2; // logical OR (lowest)
    return -1000; // unknown
}


// Print the AST recursively in a readable tree format
void print_ast(ASTNode* node, int level) {
    if (!node) return;

    int is_last = 1;
    if (node->parent) {
        ASTNode *parent = node->parent;
        for (int i = 0; i < parent->num_children; i++) {
            if (parent->children[i] == node) {
                is_last = (i == parent->num_children - 1);
                break;
            }
        }
    }
    print_tree_prefix(level, is_last);

    switch (node->type) {
        case NODE_PROGRAM:
            printf("PROGRAM\n");
            break;
        case NODE_FUNCTION_DECL:
            printf("FUNCTION: %s (returns: %s)\n",
                   node->value ? node->value : "(null)",
                   node->token.value);
            break;
        case NODE_VAR_DECL:
            printf("VAR: %s %s\n",
                   node->token.value,
                   node->value ? node->value : "(null)");
            break;
        case NODE_STATEMENT:
            printf("STATEMENT\n");
            break;
        case NODE_EXPRESSION:
            printf("EXPR: %s\n", node->value ? node->value : "(null)");
            break;
        case NODE_BINARY_EXPR:
            printf("BINARY: %s\n", node->value ? node->value : "(op)");
            break;
        case NODE_ASSIGNMENT:
            printf("ASSIGN\n");
            break;
        case NODE_BINARY_OP:
            printf("UNARY: %s\n", node->value ? node->value : "(unary)");
            break;
        case NODE_IF_STATEMENT:
            printf("IF\n");
            break;
        case NODE_ELSE_IF_STATEMENT:
            printf("ELSE IF\n");
            break;
        case NODE_ELSE_STATEMENT:
            printf("ELSE\n");
            break;
        default:
            printf("NODE_TYPE_%d\n", node->type);
            break;
    }

    for (int i = 0; i < node->num_children; i++) {
        print_ast(node->children[i], level + 1);
    }
}

// Helper function to map C types to VHDL types
char* ctype_to_vhdl(const char* ctype) {
    if (strcmp(ctype, "int") == 0) {
        return "std_logic_vector(31 downto 0)";
    } else if (strcmp(ctype, "float") == 0) {
        return "std_logic_vector(31 downto 0)"; // You may want to use 'real' for advanced VHDL
    } else if (strcmp(ctype, "double") == 0) {
        return "std_logic_vector(63 downto 0)"; // Or 'real'
    } else if (strcmp(ctype, "char") == 0) {
        return "std_logic_vector(7 downto 0)";
    }
    // Default fallback
    return "std_logic_vector(31 downto 0)";
}