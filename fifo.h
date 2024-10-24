//
// Created by ahmat on 24.10.2024.
//

/**
 * @file fifo.h
 * @brief Заголовочный файл для FIFO-буфера.
 *
 * Этот файл содержит объявление структуры FIFO-буфера и функций для работы с ним.
 */
#ifndef FIFO_H
#define FIFO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/**
 * @file fifo.h
 * @brief Заголовочный файл для реализации FIFO-буфера.
 *
 * Этот файл содержит определения структур, макросов и функций,
 * необходимых для инициализации и работы с FIFO-буфером.
 */

/// Максимальный размер FIFO-буфера.
#define MAX_FIFO_SIZE 2048           /**< Максимальный размер FIFO буфера */

/**
 * @struct FIFO_Buffer
 * @brief Структура, представляющая FIFO-буфер.
 *
 * @var FIFO_Buffer::buffer
 * Массив байтов для хранения данных буфера.
 *
 * @var FIFO_Buffer::head
 * Индекс головы буфера (позиция для записи).
 *
 * @var FIFO_Buffer::tail
 * Индекс хвоста буфера (позиция для чтения).
 *
 * @var FIFO_Buffer::size
 * Текущее количество элементов в буфере.
 */
typedef struct {
    unsigned char buffer[MAX_FIFO_SIZE]; /**< Массив для хранения данных буфера */
    int head;                            /**< Индекс головы буфера */
    int tail;                            /**< Индекс хвоста буфера */
    int size;                            /**< Текущее количество элементов в буфере */
} FIFO_Buffer;

/**
 * @brief Инициализирует FIFO-буфер.
 *
 * Устанавливает начальные значения для головы, хвоста и размера буфера.
 *
 * @param fifo Указатель на структуру FIFO_Buffer.
 */
void init_fifo(FIFO_Buffer *fifo);

/**
 * @brief Записывает данные в FIFO-буфер.
 *
 * Добавляет указанное количество байтов из массива данных в буфер.
 *
 * @param fifo Указатель на структуру FIFO_Buffer.
 * @param data Указатель на массив данных для записи.
 * @param length Количество байтов для записи.
 * @return Возвращает количество успешно записанных байтов, или -1 при ошибке.
 */
int write_fifo(FIFO_Buffer *fifo, const unsigned char *data, int length);

/**
 * @brief Читает один байт из FIFO-буфера.
 *
 * Извлекает один байт из буфера и сохраняет его по указанному адресу.
 *
 * @param fifo Указатель на структуру FIFO_Buffer.
 * @param byte Указатель на переменную, где будет сохранён прочитанный байт.
 * @return Возвращает 1 при успешном чтении, 0 если буфер пуст, или -1 при ошибке.
 */
int read_fifo(FIFO_Buffer *fifo, unsigned char *byte);

/**
 * @brief Просматривает байт в FIFO-буфере по индексу.
 *
 * Позволяет просмотреть байт, находящийся на указанном индексе от текущего хвоста буфера, не извлекая его.
 *
 * @param fifo Указатель на структуру FIFO_Buffer.
 * @param index Индекс байта от текущего хвоста буфера.
 * @param byte Указатель на переменную, где будет сохранён просматриваемый байт.
 * @return Возвращает 1 при успешном просмотре, 0 если индекс вне диапазона, или -1 при ошибке.
 */
int peek_fifo(FIFO_Buffer *fifo, int index, unsigned char *byte);

#ifdef __cplusplus
}
#endif

#endif // FIFO_H

