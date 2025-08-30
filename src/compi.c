#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parse.h"
#include "codegen_vhdl.h"


int main(int argc, char *argv[]) {

    if (argc < 3) {
        printf("Usage: %s <input.c> <output.vhdl>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *fin = fopen(argv[1], "r");
    if (!fin) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    FILE *fout = fopen(argv[2], "w");
    if (!fout) {
        perror("Error opening output file");
        fclose(fin);
        exit(EXIT_FAILURE);
    }

    printf("Parsing input file...\n");
    
    // Parse the program and build the AST
    ASTNode* program = parse_program(fin);

    #ifdef DEBUG
        print_ast(program, 0); // Print the AST for debugging if -d is passed
    #endif

    // Generate VHDL code from the AST
    if (program) {
        printf("Generating VHDL code...\n");
        generate_vhdl(program, fout);
        free_node(program);
    } else {
        fprintf(fout, "-- VHDL code generation failed\n");
        fprintf(fout, "-- AST was not generated successfully\n");
        exit(EXIT_FAILURE);
    }

    fclose(fin);
    fclose(fout);

    printf("Compilation finished.\n");
    exit(EXIT_SUCCESS);    
}