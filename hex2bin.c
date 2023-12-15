#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "readhex.h"

const char * usage = "usage: %s [{-r | -c IDLOC_COUNT} BYTECOUNT OFFSET] in.hex out.bin\n";

void write_config(FILE* outf, unsigned char* b, int size, int idloc_count){
int i;

for(i=0;i<size;i++)fprintf(outf, "conf_byte%d = 0x%02hhx\n", i, (unsigned int)b[i]);
for(i=0;i<idloc_count;i++)fprintf(outf, "user_id%d = 0xff\n", i);

}

// Convert a .hex file to a .bin file.
int main(int argc, char *argv[])
{
    char **argvp = argv + 1;
    int argcp = argc - 1;
    int size = 16384;   // Assume the ROM we're writing is 16K
    int offset = 0;     // At offset 0 within the SRECORDs
    int idloc_count;
    unsigned char conf_flag = 0xff;

    if (argc < 2) {
        fprintf(stderr, usage , argv[0]);
        return 1;
    }

    if(strcmp(argvp[0], "-r") == 0) {
        size = strtol(argvp[1], NULL, 0);
        offset = strtol(argvp[2], NULL, 0);
        argvp += 3;
        argcp -= 3;
    }

    if(strcmp(argvp[0], "-c") == 0) {
        idloc_count = strtol(argvp[1], NULL, 0);
        size = strtol(argvp[2], NULL, 0);
        offset = strtol(argvp[3], NULL, 0);
        argvp += 4;
        argcp -= 4;
        conf_flag=0;
    }

    if(argcp < 2) {
      fprintf(stderr, usage, argv[0]);
      return 1;
    }

    char *in_filename = argvp[0];
    char *out_filename = argvp[1];

    FILE *inf = fopen(in_filename, "ra");
    if (inf == NULL) {
        perror(in_filename);
        return 1;
    }

    unsigned char *b = malloc(size);
    if(b == NULL) {
        fprintf(stderr, "couldn't allocated %d bytes\n", size);
        exit(EXIT_FAILURE);
    }
    memset(b, conf_flag, size);

    struct memory_desc md;
    memory_desc_init(&md, b, offset, size);
    int success = read_hex(inf, memory_desc_store, &md, 1);
    fclose(inf);

    if (success) {
        FILE *outf = fopen(out_filename, "wb");
        if (outf == NULL) {
            perror(out_filename);
            return 1;
        }

        if( conf_flag ) fwrite(b, 1, size, outf);
        else write_config(outf, b, size, idloc_count);
        fclose(outf);
    } else {
        // The read_hex() routine will print the error.
        return 1;
    }

    return 0;
}
