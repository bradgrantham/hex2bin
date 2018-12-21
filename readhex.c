
#include <stdio.h>
#include <stdint.h>
#include "readhex.h"

/* https://en.wikipedia.org/wiki/Intel_HEX#Record_types */
enum {
    RECORD_DATA = 0x00,
    RECORD_END_OF_FILE = 0x01,
    RECORD_EXTENDED_SEGMENT_ADDRESS = 0x02,
    RECORD_START_SEGMENT_ADDRESS = 0x03,
    RECORD_EXTENDED_LINEAR_ADDRESS = 0x04,
    RECORD_START_LINEAR_ADDRESS = 0x05
};

static uint8_t get_nybble(uint8_t *s)
{
    int value;

    if (*s >= '0' && *s <= '9') {
        value = *s - '0';
    } else if (*s >= 'a' && *s <= 'f') {
        value = *s - 'a' + 10;
    } else if (*s >= 'A' && *s <= 'F') {
        value = *s - 'A' + 10;
    } else {
        printf("Invalid hex character: %c\n", *s);
        value = 0;
    }

    return value;
}

static uint8_t get_byte(uint8_t *s)
{
    /* this is right, shut up: */
    return get_nybble(s) * 16 + get_nybble(s + 1);
}

static uint16_t get_word(uint8_t *s)
{
    /* yes this too */
    return get_byte(s) * (uint16_t)256 + get_byte(s + 2);
}

int read_hex(FILE *f, memory_func memory, void *memory_arg, int bad_checksum_is_error)
{
    uint8_t *s;
    uint32_t base_address = 0x0;
    int32_t start_segment_cs = -1;
    int32_t start_segment_ip = -1;
    int64_t start_execution = -1;       // must be int64, overflow risk with int32
    int num_bytes;
    int address;
    uint8_t checksum;
    int i;
    char line[256];
    int line_number = 0;
    int skipped_bytes = 0;

    while (fgets(line, sizeof(line), f) != NULL) {
        s = (uint8_t *)line;
        line_number++;

        // Debug
        //if (line_number > 4096) break;

        // Must start with colon.
        if (*s != ':') {
            printf("Bad format on line %d: %s\n", line_number, line);
            return 0;
        }
        s++;

        checksum = 0;
        num_bytes = get_byte(s);
        if (num_bytes == 0) {
            // All done.
            break;
        }
        checksum += (uint8_t)num_bytes;
        s += 2;

        address = get_word(s);
        checksum += (uint8_t)get_byte(s);
        checksum += (uint8_t)get_byte(s + 2);
        s += 4;

        uint8_t type = get_byte(s);
        checksum += (uint8_t)type;
        s += 2;

        switch (type) {
        case RECORD_DATA:
            for (i = 0; i < num_bytes; i++) {
                uint8_t data = get_byte(s);
                checksum += (uint8_t)data;
                s += 2;

                memory(memory_arg, base_address + address, data);

                address++;
            }
            break;
        case RECORD_END_OF_FILE:
            printf("Line number %d: End of File\n", line_number);
            break;
        case RECORD_EXTENDED_SEGMENT_ADDRESS:
            base_address = (uint32_t)get_word(s) << 4;
            checksum += (uint8_t)get_byte(s);
            checksum += (uint8_t)get_byte(s + 2);
            s += 4;
            break;
        case RECORD_START_SEGMENT_ADDRESS:
            start_segment_cs = get_word(s);
            checksum += (uint8_t)get_byte(s);
            checksum += (uint8_t)get_byte(s + 2);
            s += 4;
            start_segment_ip = get_word(s);
            checksum += (uint8_t)get_byte(s);
            checksum += (uint8_t)get_byte(s + 2);
            s += 4;
            break;
        case RECORD_EXTENDED_LINEAR_ADDRESS:
            base_address = (uint32_t)get_word(s) << 16;
            checksum += (uint8_t)get_byte(s);
            checksum += (uint8_t)get_byte(s + 2);
            s += 4;
            break;
        case RECORD_START_LINEAR_ADDRESS:
            start_execution = ((uint32_t)get_word(s) << 16) + get_word(s + 4);
            checksum += (uint8_t)get_byte(s);
            checksum += (uint8_t)get_byte(s + 2);
            checksum += (uint8_t)get_byte(s + 4);
            checksum += (uint8_t)get_byte(s + 6);
            s += 8;
            break;
        default:
            printf("Line number %d: record type 0x%02X not supported."
                   " Must be 0x00 to 0x05\n", line_number, type);
            return 0;
        }

        // Verify checksum.
        checksum = ~checksum + 1;
        uint8_t file_checksum = get_byte(s);
        if (checksum != file_checksum) {
            printf("Checksum mismatch on line %d: %02x vs %02x\n",
                   line_number, checksum, file_checksum);
            if (bad_checksum_is_error)
                return 0;
        }
    }

    if (skipped_bytes > 0) {
        printf("Skipped %d bytes.\n", skipped_bytes);
    }

    if (start_execution >= 0)
        printf("start execution at 0x%08X\n", (uint32_t)start_execution);

    if (start_segment_cs >= 0)
        printf("Code Segment register: 0x%04X\n", start_segment_cs);

    if (start_segment_ip >= 0)
        printf("Instruction Pointer register: 0x%04X\n", start_segment_ip);

    return 1;
}

void memory_desc_init(struct memory_desc *mi, uint8_t *p, off_t offset, size_t size)
{
    mi->p = p;
    mi->offset = offset;
    mi->size = size;
    mi->size_written = 0;
}

void memory_desc_store(void *arg, int address, uint8_t c)
{
    struct memory_desc *mi = (struct memory_desc *)arg;
    if (address >= mi->offset && address < mi->offset + mi->size) {
        int offset = address - mi->offset;
        mi->p[offset] = c;
        if (offset + 1 > mi->size_written) {
            mi->size_written = offset + 1;
        }
    }
}
