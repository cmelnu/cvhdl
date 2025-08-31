#include "utils.h"

// Helper function to print tree branches for AST visualization
void print_tree_prefix(int level, int is_last) {

    int i = 0;

    for (i = 0; i < level; i++) {
        printf("%s", (i == level - 1) ? (is_last ? "└── " : "├── ") : "    ");
    }
}


int is_number_str(const char *s) {

    const char *p = NULL;

    if (!s || !*s) return 0;

    p = s;

    if (*p == '+' || *p == '-') p++;
    if (!*p) return 0;

    while (*p) {
        if (!isdigit((unsigned char)*p)) return 0;
        p++;
    }

    return 1;
}

// Add this helper function above generate_vhdl:
int is_negative_literal(const char* value) {

    int i = 1;
    int has_valid = 0;

    if (!value || value[0] != '-' || strlen(value) < 2)
        return 0;

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

    // Higher number = higher precedence (mirrors C precedence ordering)
    if (!op) return -999;
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

    int is_last = 1;
    int i = 0;
    ASTNode *parent = NULL;

    if (!node) return;

    // Find if this node is the last child
    if (node->parent) {
        parent = node->parent;
        for (i = 0; i < parent->num_children; i++) {
            if (parent->children[i] == node) {
                is_last = (i == parent->num_children - 1);
                break;
            }
        }
    }

    print_tree_prefix(level, is_last);

    // Print node type
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
        case NODE_WHILE_STATEMENT:
            printf("WHILE\n");
            break;
        case NODE_BREAK_STATEMENT:
            printf("BREAK\n");
            break;
        case NODE_CONTINUE_STATEMENT:
            printf("CONTINUE\n");
            break;
        default:
            printf("NODE_TYPE_%d\n", node->type);
            break;
    }

    for (i = 0; i < node->num_children; i++) {
        print_ast(node->children[i], level + 1);
    }
}

// Helper function to map C types to VHDL types
char* ctype_to_vhdl(const char* ctype) {

    // No local variables needed
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