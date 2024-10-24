/**
 * @file parser.c
 * @brief Реализация парсера UART-пакетов.
 *
 * Этот файл содержит реализацию функций для парсинга UART-пакетов.
 */

#include "parser.h"
#include <stdio.h>
#include <string.h>
const unsigned char SYNC_SEQUENCE[SYNC_SEQUENCE_LENGTH] = {0xAA, 0xBB, 0xCC};
// Инициализация парсера
void init_parser(Parser *parser, FIFO_Buffer *fifo, PacketCallback callback) {
    parser->state = STATE_SYNC;
    parser->fifo = fifo;
    parser->callback = callback;
    parser->sync_pos = 0;
    parser->data_size = 0;
    parser->type = 0;
    parser->header_checksum = 0;
    parser->calculated_header_checksum = 0;
    parser->size_bytes_read = 0;
    parser->type_bytes_read = 0;
    parser->body_bytes_read = 0;
    memset(parser->body, 0, MAX_PACKET_SIZE);
}

// Function to decode variable length field (Size or Type)
int decode_variable_length(FIFO_Buffer *fifo, Parser *parser, unsigned int *value) {
    unsigned char byte;
    if (read_fifo(fifo, &byte) != 0) {
        return -1; // Not enough data
    }

    if (byte < 128) {
        // Single byte
        *value = byte;
    } else {
        // Two bytes
        unsigned char byte1 = byte;
        if (read_fifo(fifo, &byte) != 0) {
            // Not enough data for second byte
            // Push back the first byte
            parser->fifo->head = (parser->fifo->head - 1 + MAX_FIFO_SIZE) % MAX_FIFO_SIZE;
            parser->fifo->size++;
            return -1;
        }
        *value = (byte1 - 128) + (byte << 7);
    }
    return 0;
}

// Calculate checksum (sum of bytes modulo 256)
unsigned char calculate_checksum(unsigned char *data, int length) {
    unsigned int sum = 0;

    for(int i =0; i < length; i++) {
        sum += data[i];
    }
    return (unsigned char)(sum % 256);
}

// Parse Function
void parse_uart(Parser *parser) {
    unsigned char byte;
    while (parser->fifo->size > 0) {
        switch (parser->state) {
            case STATE_SYNC:
                // Look for sync sequence
                if (peek_fifo(parser->fifo, 0, &byte) == 0) {
                    if (byte == SYNC_SEQUENCE[parser->sync_pos]) {
                        parser->sync_pos++;
                        // Remove the byte from FIFO
                        read_fifo(parser->fifo, &byte);
                        if (parser->sync_pos == SYNC_SEQUENCE_LENGTH) {
                            parser->state = STATE_HEADER_SIZE;
                            parser->sync_pos = 0;
                            // Reset header fields
                            parser->data_size = 0;
                            parser->type = 0;
                            parser->header_checksum = 0;
                            parser->calculated_header_checksum = 0;
                            parser->size_bytes_read = 0;
                            parser->type_bytes_read = 0;
                        }
                    } else {
                        // Mismatch, reset sync position
                        parser->sync_pos = 0;
                        // Remove the byte to prevent infinite loop
                        read_fifo(parser->fifo, &byte);
                    }
                }
                break;

            case STATE_HEADER_SIZE:
            {
                unsigned int size;
                int result = decode_variable_length(parser->fifo, parser, &size);
                if (result == 0) {
                    parser->data_size = size;
                    parser->calculated_header_checksum += (size & 0xFF);
                    parser->state = STATE_HEADER_TYPE;
                } else {
                    // Wait for more data
                    return;
                }
            }
                break;

            case STATE_HEADER_TYPE:
            {
                unsigned int type;
                int result = decode_variable_length(parser->fifo, parser, &type);
                if (result == 0) {
                    parser->type = type;
                    parser->calculated_header_checksum += (type & 0xFF);
                    parser->state = STATE_HEADER_CHECKSUM;
                } else {
                    // Wait for more data
                    return;
                }
            }
                break;

            case STATE_HEADER_CHECKSUM:
                if (read_fifo(parser->fifo, &byte) == 0) {
                    parser->header_checksum = byte;
                    unsigned char computed_checksum = calculate_checksum((unsigned char*)&parser->data_size, sizeof(unsigned int));
                    // Alternatively, use the accumulated checksum
                    computed_checksum = parser->calculated_header_checksum;
                    if (computed_checksum == parser->header_checksum) {
                        // Check data size limits
                        if (parser->data_size > MAX_PACKET_SIZE) {
                            printf("Error: Data size exceeds maximum limit.\n");
                            parser->state = STATE_SYNC;
                            break;
                        }
                        // Initialize body reading
                        parser->body_bytes_read = 0;
                        if (parser->data_size == 0) {
                            // No body, packet complete
                            parser->callback(parser->type, parser->body, parser->body_bytes_read);
                            parser->state = STATE_SYNC;
                        } else {
                            parser->state = STATE_BODY;
                        }
                    } else {

                        printf("Error: Header checksum mismatch. Expected: %02X, Received: %02X\n", computed_checksum, parser->header_checksum);
                        parser->state = STATE_SYNC;
                    }
                } else {
                    // Wait for more data
                    return;
                }
                break;

            case STATE_BODY:
            {
                int bytes_available = parser->fifo->size;
                int bytes_to_read = parser->data_size - parser->body_bytes_read;
                if (bytes_available >= bytes_to_read) {
                    // Read all remaining body bytes
                    read_fifo(parser->fifo, &byte); // Just to consume the first byte
                    for(int i =0; i < bytes_to_read; i++) {
                        read_fifo(parser->fifo, &byte);
                        parser->body[i] = byte;
                    }
                    parser->body_bytes_read += bytes_to_read;
                    // Packet complete
                    parser->callback(parser->type, parser->body, parser->body_bytes_read);
                    parser->state = STATE_SYNC;
                } else {
                    // Read available bytes
                    for(int i =0; i < bytes_available; i++) {
                        read_fifo(parser->fifo, &byte);
                        if (parser->body_bytes_read < MAX_PACKET_SIZE) {
                            parser->body[parser->body_bytes_read++] = byte;
                        } else {
                            printf("Error: Body buffer overflow.\n");
                            parser->state = STATE_SYNC;
                            break;
                        }
                    }
                    // Wait for more data
                    return;
                }
            }
                break;

            default:
                // Invalid state, reset
                parser->state = STATE_SYNC;
                break;
        }
    }
}

// Helper Function to Encode Variable Length Field
int encode_variable_length(unsigned int value, unsigned char *output, int *length) {
    if (value < 128) {
        output[0] = (unsigned char)(value & 0x7F);
        *length = 1;
    } else {
        output[0] = (unsigned char)((value - 128) & 0x7F);
        output[1] = (unsigned char)((value >> 7) & 0xFF);
        *length = 2;
    }
    return 0;
}

// Helper Function to Build a Packet
int build_packet(unsigned char *packet, unsigned int *packet_length, unsigned int data_size, unsigned int type, unsigned char *data) {
    int pos = 0;

    // Add Sync Sequence
    memcpy(&packet[pos], SYNC_SEQUENCE, SYNC_SEQUENCE_LENGTH);
    pos += SYNC_SEQUENCE_LENGTH;

    // Encode Data Size
    unsigned char size_encoded[2];
    int size_len;
    encode_variable_length(data_size, size_encoded, &size_len);
    memcpy(&packet[pos], size_encoded, size_len);
    pos += size_len;

    // Encode Type
    unsigned char type_encoded[2];
    int type_len;
    encode_variable_length(type, type_encoded, &type_len);
    memcpy(&packet[pos], type_encoded, type_len);
    pos += type_len;

    // Calculate Header Checksum
    unsigned char checksum = 0;
    for(int i = SYNC_SEQUENCE_LENGTH; i < pos; i++) {
        checksum += packet[i];
    }
    packet[pos++] = checksum;

    // Add Body
    if (data_size > 0 && data != NULL) {
        memcpy(&packet[pos], data, data_size);
        pos += data_size;
    }

    *packet_length = pos;
    return 0;
}

// Callback Function Prototype


// Example Callback Implementation
void packet_received_callback(unsigned int type, unsigned char *data, unsigned int size) {
    printf("Packet Received:\n");
    printf("Type: %u\n", type);
    printf("Size: %u bytes\n", size);
    printf("Data: ");
    for (unsigned int i = 0; i < size; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n\n");
}
