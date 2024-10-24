//
// Created by ahmat on 24.10.2024.
//

/**
 * @file fifo.c
 * @brief Реализация FIFO-буфера.
 *
 * Этот файл содержит реализацию функций для работы с FIFO-буфером.
 */
#include "fifo.h"
// Инициализация буфера
void init_fifo(FIFO_Buffer *fifo) {
    fifo->head = 0;
    fifo->tail = 0;
    fifo->size = 0;
    memset(fifo->buffer, 0, MAX_FIFO_SIZE);
}

// Запись значения в буфер
int write_fifo(FIFO_Buffer *fifo, const unsigned char *data, int length) {
    if (length > (MAX_FIFO_SIZE - fifo->size)) {
        // Если длина новых данных length превышает доступное место, функция возвращает -1
        return -1;
    }
    for (int i = 0; i < length; i++) {
        fifo->buffer[fifo->tail] = data[i];
        fifo->tail = (fifo->tail + 1) % MAX_FIFO_SIZE;
    }
    fifo->size += length; // Изменение количесвта данных в буфере
    return 0;
}

// Извличение одного байта из буфера
int read_fifo(FIFO_Buffer *fifo, unsigned char *byte) {
    if (fifo->size == 0) {
        return -1; // Если буфер пуст то возвращаем -1
    }
    *byte = fifo->buffer[fifo->head];
    fifo->head = (fifo->head + 1) % MAX_FIFO_SIZE;
    fifo->size--;
    return 0;
}

// Чтение любого элемента в буфере
int peek_fifo(FIFO_Buffer *fifo, int index, unsigned char *byte) {
    if (index >= fifo->size) {
        return -1; //  Если index больше или равен `fifo->size`, это означает, что запрашиваемый элемент отсутствует в буфере, и функция возвращает -1
    }
    int pos = (fifo->head + index) % MAX_FIFO_SIZE;
    *byte = fifo->buffer[pos];
    return 0;
}
