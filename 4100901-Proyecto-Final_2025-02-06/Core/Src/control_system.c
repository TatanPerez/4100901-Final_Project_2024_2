#include "control_system.h"
#include "string.h"
#include "ring_buffer.h"
#include "main.h"

// Definición del buffer circular y otras variables
ring_buffer_t rx_buffer;
uint8_t rx_buffer_mem[64];  // Memoria para el buffer circular
char current_cmd[COMMAND_LENGTH];  // Almacena el comando actual
uint8_t cmd_index = 0;  // Índice para realizar el seguimiento del comando actual
extern UART_HandleTypeDef huart2;
extern uint8_t rx_byte; 
// Función para inicializar el sistema de control
void control_system_init(void) {
    ring_buffer_init(&rx_buffer, rx_buffer_mem, sizeof(rx_buffer_mem));
    memset(current_cmd, 0, COMMAND_LENGTH);  // Inicializa el buffer de comandos
}

// Callback que maneja la recepción de bytes por UART
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == &huart2) {  // Verifica si la interrupción es de UART2
        ring_buffer_write(&rx_buffer, rx_byte);  // Escribe el byte recibido en el buffer circular
        HAL_UART_Transmit(&huart2, &rx_byte, 1, 100);  // Realiza un echo del byte recibido
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);  // Habilita la recepción de un nuevo byte
    }
}

// Función para enviar una cadena por UART
void uart_send_string(const char *str) {
    HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), 100);
}

// Función que procesa los comandos recibidos por UART
void process_commands(void) {
    uint8_t byte;
    while (ring_buffer_read(&rx_buffer, &byte)) {  // Lee un byte del buffer circular
        // Desplaza el buffer de comando y agrega el nuevo byte
        memmove(current_cmd, current_cmd + 1, COMMAND_LENGTH - 1);
        current_cmd[COMMAND_LENGTH - 1] = (char)byte;

        // Compara el comando actual con los comandos predefinidos
        if (memcmp(current_cmd, "#*A*#", COMMAND_LENGTH) == 0) {
            HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);  // Enciende el LED
            uart_send_string("\r\n#*A*#\r\nPuerta abierta (encendido LD2)\r\n");
            memset(current_cmd, 0, COMMAND_LENGTH);  // Limpia el buffer de comando
        } else if (memcmp(current_cmd, "#*C*#", COMMAND_LENGTH) == 0) {
            HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);  // Apaga el LED
            uart_send_string("\r\n#*C*#\r\nPuerta cerrada (apagado LD2)\r\n");
            memset(current_cmd, 0, COMMAND_LENGTH);  // Limpia el buffer de comando
        } else if (memcmp(current_cmd, "#*1*#", COMMAND_LENGTH) == 0) {
            uint8_t state = HAL_GPIO_ReadPin(LD2_GPIO_Port, LD2_Pin);  // Lee el estado del LED
            uart_send_string(state ? "\r\n#*1*#\r\nEstado de la puerta: abierta (LD2 ON)\r\n" :
                                     "#*1*#\r\nEstado de la puerta: cerrada (LD2 OFF)\r\n");
            memset(current_cmd, 0, COMMAND_LENGTH);  // Limpia el buffer de comando
        } else if (memcmp(current_cmd, "#*0*#", COMMAND_LENGTH) == 0) {
            ring_buffer_reset(&rx_buffer);  // Limpia el buffer circular
            uart_send_string("\r\n#*0*#\r\nComando de limpiar buffer: El buffer está vacío\r\n");
            memset(current_cmd, 0, COMMAND_LENGTH);  // Limpia el buffer de comando
        }
    }
}
