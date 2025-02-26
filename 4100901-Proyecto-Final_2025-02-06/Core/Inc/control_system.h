#ifndef CONTROL_SYSTEM_H
#define CONTROL_SYSTEM_H

#include "ring_buffer.h"
#include "main.h"

// Define el tamaño del comando
#define LENGTH 5    // Longitud de los comandos

#define BUTTON_DEBOUNCE_DELAY   50    // 50 ms para el rebote
#define BUTTON_PRESS_TIMEOUT   500   // 500 ms para considerar fin de la secuencia
extern volatile uint8_t button_press_count;
extern volatile uint32_t last_button_press_time;
// Buffer circular para almacenar los datos recibidos
extern ring_buffer_t keypad;
extern uint8_t keypad_mem[64];  // Memoria para el buffer circular
extern ring_buffer_t terminal;
extern uint8_t terminal_mem[64];  // Memoria para el buffer circular
extern char current_cmd[LENGTH];  // Almacena el comando actual
extern uint8_t cmd_index;  // Índice para realizar el seguimiento del comando actual
extern UART_HandleTypeDef huart2;  // Declaración de huart2
extern uint8_t rx_byte;
extern ring_buffer_t esp01_buffer;
extern uint8_t esp01_tx_byte_mem[64];
extern uint8_t esp01_rx_byte;  

// Función que inicializa el sistema de control
void control_system_init(void);

// Callback para manejar la recepción de datos UART
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

// Función que procesa los comandos recibidos
void process_commands(void);

// Función auxiliar para enviar una cadena por UART
void uart_send_string(const char *str);
void process_buffer_commands(ring_buffer_t *rb);
void ssd1306_On_Led(void);
void ssd1306_Off_Led(void);
void process_button(void);
#endif /* CONTROL_SYSTEM_H */
