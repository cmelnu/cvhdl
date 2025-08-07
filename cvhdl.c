#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <input.c> <output.vhdl>\n", argv[0]);
        return 1;
    }

    FILE *fin = fopen(argv[1], "r");
    if (!fin) {
        perror("Error opening input file");
        return 1;
    }

    FILE *fout = fopen(argv[2], "w");
    if (!fout) {
        perror("Error opening output file");
        fclose(fin);
        return 1;
    }

    // Step 1: Read the input C file line by line and print each line (basic parsing)
    char line[1024];
    printf("Parsing input file:\n");
    while (fgets(line, sizeof(line), fin)) {
        // For now, just print each line (later, you can tokenize and analyze)
        printf("%s", line);
        // TODO: Tokenize and process each line for code generation
    }

    // TODO: Generate VHDL code to fout

    fprintf(fout, "-- VHDL code generated from %s\n", argv[1]);
    // Placeholder: Write a simple VHDL entity
    fprintf(fout, "entity dummy is\nend entity;\n");

    fclose(fin);
    fclose(fout);

    printf("Compilation finished.\n");
    return 0;
}