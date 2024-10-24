/**
 * @file parser.h
 * @brief Заголовочный файл для парсера UART-пакетов.
 *
 * Этот файл содержит объявление структур и функций, используемых для парсинга UART-пакетов.
 */
#ifndef PARSER_H
#define PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include "fifo.h"


#define MAX_FIFO_SIZE 2048           /**< Максимальный размер FIFO буфера */
#define MAX_PACKET_SIZE 1000         /**< Максимальный размер пакета данных */
#define SYNC_SEQUENCE_LENGTH 3       /**< Длина последовательности синхронизации */
#define MAX_HEADER_SIZE 7            /**< Максимальный размер заголовка пакета */

/**
 * @brief Тип функции обратного вызова при приеме пакета.
 *
 * @param type Тип пакета.
 * @param data Указатель на данные пакета.
 * @param size Размер данных пакета.
 */
typedef void (*PacketCallback)(unsigned int type, unsigned char *data, unsigned int size);

/**
 * @enum ParserState
 * @brief Перечисление состояний парсера.
 */
typedef enum {
    STATE_SYNC,           /**< Начальное состояние парсера (поиск синхронизации) */
    STATE_HEADER_SIZE,    /**< Получение размера заголовка пакета */
    STATE_HEADER_TYPE,    /**< Получение типа пакета */
    STATE_HEADER_CHECKSUM,/**< Получение контрольной суммы заголовка */
    STATE_BODY            /**< Получение тела пакета */
} ParserState;

/**
 * @struct Parser
 * @brief Структура, представляющая парсер данных.
 */
typedef struct {
    ParserState state;                    /**< Текущее состояние парсера */
    FIFO_Buffer *fifo;                    /**< Указатель на FIFO буфер */
    PacketCallback callback;              /**< Функция обратного вызова при приеме пакета */

    int sync_pos;                         /**< Позиция поиска синхронизации */

    // Поля Заголовка
    unsigned int data_size;               /**< Размер данных в пакете */
    unsigned int type;                    /**< Тип пакета */
    unsigned char header_checksum;        /**< Контрольная сумма заголовка */
    unsigned char calculated_header_checksum; /**< Вычисленная контрольная сумма заголовка */
    int size_bytes_read;                  /**< Количество байтов, прочитанных для размера данных */
    int type_bytes_read;                  /**< Количество байтов, прочитанных для типа пакета */

    // Поля Тела
    unsigned char body[MAX_PACKET_SIZE];  /**< Массив для хранения данных тела пакета */
    unsigned int body_bytes_read;         /**< Количество байтов, прочитанных для тела пакета */
} Parser;

/**
 * @brief Инициализирует парсер.
 *
 * Устанавливает начальные значения для структуры парсера и устанавливает FIFO буфер и функцию обратного вызова.
 *
 * @param parser Указатель на структуру парсера.
 * @param fifo Указатель на FIFO буфер.
 * @param callback Функция обратного вызова при приеме пакета.
 */
void init_parser(Parser *parser, FIFO_Buffer *fifo, PacketCallback callback);

/**
 * @brief Декодирует переменную длину из FIFO буфера.
 *
 * Извлекает значение переменной длины из FIFO буфера и сохраняет его в переданной переменной.
 *
 * @param fifo Указатель на FIFO буфер.
 * @param parser Указатель на структуру парсера.
 * @param value Указатель на переменную, где будет сохранено декодированное значение.
 * @return Возвращает 0 при успешном декодировании, -1 при ошибке.
 */
int decode_variable_length(FIFO_Buffer *fifo, Parser *parser, unsigned int *value);

/**
 * @brief Вычисляет контрольную сумму данных.
 *
 * Использует простой алгоритм суммирования байтов для вычисления контрольной суммы.
 *
 * @param data Указатель на данные, для которых вычисляется контрольная сумма.
 * @param length Длина данных в байтах.
 * @return Вычисленная контрольная сумма.
 */
unsigned char calculate_checksum(unsigned char *data, int length);

/**
 * @brief Парсит входящие данные из UART.
 *
 * Обрабатывает данные из FIFO буфера в соответствии с текущим состоянием парсера.
 *
 * @param parser Указатель на структуру парсера.
 */
void parse_uart(Parser *parser);

/**
 * @brief Кодирует значение переменной длины в байты.
 *
 * Преобразует целочисленное значение в переменную длину и сохраняет результат в выходной буфер.
 *
 * @param value Значение для кодирования.
 * @param output Указатель на буфер для сохранения закодированных байтов.
 * @param length Указатель на переменную, где будет сохранена длина закодированных байтов.
 * @return Возвращает 0 при успешном кодировании, -1 при ошибке.
 */
int encode_variable_length(unsigned int value, unsigned char *output, int *length);

/**
 * @brief Составляет пакет данных.
 *
 * Формирует пакет с заданным типом и данными, заполняя переданный буфер.
 *
 * @param packet Указатель на буфер для хранения сформированного пакета.
 * @param packet_length Указатель на переменную, где будет сохранена длина сформированного пакета.
 * @param data_size Размер данных в пакете.
 * @param type Тип пакета.
 * @param data Указатель на данные, которые будут включены в пакет.
 * @return Возвращает 0 при успешном построении пакета, -1 при ошибке.
 */
int build_packet(unsigned char *packet, unsigned int *packet_length, unsigned int data_size, unsigned int type, unsigned char *data);

/**
 * @brief Обратная функция при приеме пакета.
 *
 * Функция вызывается, когда весь пакет успешно принят и обработан.
 *
 * @param type Тип принятого пакета.
 * @param data Указатель на данные принятого пакета.
 * @param size Размер данных принятого пакета.
 */
void packet_received_callback(unsigned int type, unsigned char *data, unsigned int size);

#ifdef __cplusplus
}
#endif

#endif // PARSER_H