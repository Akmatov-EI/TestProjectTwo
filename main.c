#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"


// Main Function with Test Cases
int main() {
    // Initialize FIFO Buffer
    FIFO_Buffer fifo;
    init_fifo(&fifo);

    // Initialize Parser
    Parser parser;
    init_parser(&parser, &fifo, packet_received_callback);

    // Example Data to Send
    unsigned char data1[] = {0x10, 0x20, 0x30, 0x40};
    unsigned char data2[] = {0x50, 0x60};
    unsigned char data3[] = {0x70, 0x80, 0x90};

    // Build a valid packet

    unsigned char packet1[SYNC_SEQUENCE_LENGTH + MAX_HEADER_SIZE + MAX_PACKET_SIZE];
    unsigned int packet1_length;
    build_packet(packet1, &packet1_length, sizeof(data1), 5, data1);

    // Build another valid packet with larger size
    unsigned char packet2[SYNC_SEQUENCE_LENGTH + MAX_HEADER_SIZE + MAX_PACKET_SIZE];
    unsigned int packet2_length;
    build_packet(packet2, &packet2_length, sizeof(data2), 130, data2); // Type requires two bytes

    // Build a packet with no body
    unsigned char packet3[SYNC_SEQUENCE_LENGTH + MAX_HEADER_SIZE];
    unsigned int packet3_length;
    build_packet(packet3, &packet3_length, 0, 7, NULL);

    // Combine all packets into a single stream
    unsigned char stream[packet1_length + packet2_length + packet3_length];
    int stream_pos = 0;
    memcpy(&stream[stream_pos], packet1, packet1_length);
    stream_pos += packet1_length;
    memcpy(&stream[stream_pos], packet2, packet2_length);
    stream_pos += packet2_length;
    memcpy(&stream[stream_pos], packet3, packet3_length);
    stream_pos += packet3_length;

    // Simulate receiving data in chunks
    int chunk_sizes[] = {5, 10, 15, stream_pos - 30};
    int num_chunks = sizeof(chunk_sizes)/sizeof(chunk_sizes[0]);
    int current_pos = 0;

    for(int i =0; i < num_chunks; i++) {
        int chunk_size = chunk_sizes[i];
        if (current_pos + chunk_size > stream_pos) {
            chunk_size = stream_pos - current_pos;
        }
        printf("Writing %d bytes to FIFO...\n", chunk_size);
        write_fifo(&fifo, &stream[current_pos], chunk_size);
        current_pos += chunk_size;

        // Parse the current FIFO content
        parse_uart(&parser);
    }

    // Write any remaining data
    if (current_pos < stream_pos) {
        int remaining = stream_pos - current_pos;
        printf("Writing remaining %d bytes to FIFO...\n", remaining);
        write_fifo(&fifo, &stream[current_pos], remaining);
        current_pos += remaining;

        // Final parse
        parse_uart(&parser);
    }

    return 0;
}
