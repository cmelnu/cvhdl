#include "utils.h"


// Helper function to print tree branches for AST visualization
static void print_tree_prefix(int level, int is_last) {
    for (int i = 0; i < level; i++) {
        printf("%s", (i == level - 1) ? (is_last ? "└── " : "├── ") : "    ");
    }
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