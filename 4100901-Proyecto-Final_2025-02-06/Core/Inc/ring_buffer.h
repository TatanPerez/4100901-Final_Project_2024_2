
/*
 * ring_buffer.h
 *
 *  Created on: Feb 11, 2025
 *      Author: saaci
 */
#ifndef INC_RING_BUFFER_H_
#define INC_RING_BUFFER_H_
#include <stdint.h>
typedef struct ring_buffer_ {
    uint8_t *buffer;  // Puntero a la memoria donde se almacenarán los datos
    uint8_t head;     // Índice de la cabeza (posición de escritura)
    uint8_t tail;     // Índice de la cola (posición de lectura)
    uint8_t is_full;  // Indica si el buffer está lleno (1: lleno, 0: no lleno)
    uint8_t capacity; // Capacidad máxima del buffer (cantidad de elementos que puede almacenar)
} ring_buffer_t;
void ring_buffer_init(ring_buffer_t *rb, uint8_t *mem_add, uint8_t capacity);
void ring_buffer_reset(ring_buffer_t *rb);
uint8_t ring_buffer_size(ring_buffer_t *rb);	// Devuelve el número de elementos almacenados en el buffer.
uint8_t ring_buffer_is_full(ring_buffer_t *rb);		// Retorna 0 si esta vacio  1 en caso contrario
uint8_t ring_buffer_is_empty(ring_buffer_t *rb);
void ring_buffer_write(ring_buffer_t *rb, uint8_t data);
uint8_t ring_buffer_read(ring_buffer_t *rb, uint8_t *byte);
#endif /* INC_RING_BUFFER_H_ */