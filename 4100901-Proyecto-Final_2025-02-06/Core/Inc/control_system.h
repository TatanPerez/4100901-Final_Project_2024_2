#ifndef CONTROL_SYSTEM_H
#define CONTROL_SYSTEM_H

#include "ring_buffer.h"
#include "main.h"

// Define el tamaño del comando
#define COMMAND_LENGTH 5

// Buffer circular para almacenar los datos recibidos
extern ring_buffer_t rx_buffer;
extern uint8_t rx_buffer_mem[64];  // Memoria para el buffer circular
extern char current_cmd[COMMAND_LENGTH];  // Almacena el comando actual
extern uint8_t cmd_index;  // Índice para realizar el seguimiento del comando actual
extern UART_HandleTypeDef huart2;  // Declaración de huart2
extern uint8_t rx_byte;
// Función que inicializa el sistema de control
void control_system_init(void);

// Callback para manejar la recepción de datos UART
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

// Función que procesa los comandos recibidos
void process_commands(void);

// Función auxiliar para enviar una cadena por UART
void uart_send_string(const char *str);

#endif /* CONTROL_SYSTEM_H */
