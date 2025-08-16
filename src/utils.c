#include "utils.h"

// Helper function to print indentation
void print_indent(int level) {
    for (int i = 0; i < level; i++) {
        printf("  ");
    }
}

// Print the AST recursively for visualization
void print_ast(ASTNode* node, int level) {

    if (!node) return;

    print_indent(level);
    
    switch (node->type) {
        case NODE_PROGRAM:
            printf("NODE_PROGRAM\n");
            break;
        case NODE_FUNCTION_DECL:
            printf("NODE_FUNCTION_DECL: %s (return type: %s)\n",
                   node->value ? node->value : "(null)",
                   node->token.value);
            break;
        case NODE_VAR_DECL:
            printf("NODE_VAR_DECL: %s %s\n",
                   node->token.value,
                   node->value ? node->value : "(null)");
            break;
        case NODE_STATEMENT:
            printf("NODE_STATEMENT\n");
            break;
        case NODE_EXPRESSION:
            printf("NODE_EXPRESSION: %s\n", node->value ? node->value : "(null)");
            break;
        case NODE_BINARY_EXPR:
            printf("NODE_BINARY_EXPR: %s\n", node->value ? node->value : "(op)");
            break;
        case NODE_ASSIGNMENT:
            printf("NODE_ASSIGNMENT\n");
            break;
        case NODE_BINARY_OP:
            printf("NODE_UNARY_OP: %s\n", node->value ? node->value : "(unary)");
            break;
        case NODE_IF_STATEMENT:
            printf("NODE_IF_STATEMENT\n");
            break;
        case NODE_ELSE_IF_STATEMENT:
            printf("NODE_ELSE_IF_STATEMENT\n");
            break;
        case NODE_ELSE_STATEMENT:
            printf("NODE_ELSE_STATEMENT\n");
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