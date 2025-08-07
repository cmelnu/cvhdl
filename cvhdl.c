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

    // TODO: Parse C code from fin
    // TODO: Generate VHDL code to fout

    fprintf(fout, "-- VHDL code generated from %s\n", argv[1]);
    // Placeholder: Write a simple VHDL entity
    fprintf(fout, "entity dummy is\nend entity;\n");

    fclose(fin);
    fclose(fout);

    printf("Compilation finished.\n");
    return 0;
}