#include "control_system.h"  // Librería específica del sistema de control
#include "string.h"          // Librería estándar para manejo de cadenas
#include "ring_buffer.h"     // Librería para manejo de buffers circulares
#include "main.h"            // Configuraciones principales del microcontrolador

// Definición de constantes
// #define LENGTH 5  // Longitud de los comandos
const char OPEN[] = "#*A*#";   // Comando para abrir
const char CLOSE[] = "#*C*#";  // Comando para cerrar
const char STATUS[] = "#*1*#"; // Comando para consultar el estado
const char CLEAR[] = "#*0*#";  // Comando para limpiar el buffer

// Definición de buffers circulares y variables
ring_buffer_t keypad;         // Buffer circular para el teclado
uint8_t keypad_mem[64];       // Memoria reservada para el buffer del teclado
ring_buffer_t terminal;       // Buffer circular para la terminal
uint8_t terminal_mem[64];    // Memoria reservada para el buffer de la terminal

char current_cmd[LENGTH];     // Almacena el comando actual que se está procesando
uint8_t cmd_index = 0;        // Índice para rastrear la posición en el comando actual
extern UART_HandleTypeDef huart2; // Manejador de UART2 (definido externamente)
extern uint8_t rx_byte;       // Byte recibido por UART (definido externamente)

// Función para inicializar el sistema de control
void control_system_init(void) {
    // Inicializa el buffer circular del teclado
    ring_buffer_init(&keypad, keypad_mem, sizeof(keypad_mem));
    // Limpia el buffer de comandos actual
    memset(current_cmd, 0, LENGTH);
    // Inicializa el buffer circular de la terminal
    ring_buffer_init(&terminal, terminal_mem, sizeof(terminal_mem));
}

// Callback que maneja la recepción de bytes por UART
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    // Verifica si la interrupción es de UART2
    if (huart == &huart2) {
        // Escribe el byte recibido en el buffer circular del teclado
        ring_buffer_write(&keypad, rx_byte);
        // Envía un eco del byte recibido (para depuración)
        HAL_UART_Transmit(&huart2, &rx_byte, 1, 100);
        // Habilita la recepción del siguiente byte
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    }
}

// Función para enviar una cadena de caracteres por UART
void uart_send_string(const char *str) {
    // Transmite la cadena a través de UART2
    HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), 100);
}

// Función principal para procesar comandos
void process_commands(void) {
    // Procesa los comandos en el buffer de la terminal
    process_buffer_commands(&terminal);
    // Procesa los comandos en el buffer del teclado
    process_buffer_commands(&keypad);
}

// Función para procesar comandos en un buffer circular específico
void process_buffer_commands(ring_buffer_t *rb) {
    // Mientras haya suficientes datos en el buffer para formar un comando
    while (ring_buffer_size(rb) >= LENGTH) {
        uint8_t temp[LENGTH];  // Almacena temporalmente el comando leído

        // Lee los siguientes LENGTH bytes del buffer sin eliminarlos (peek)
        for (int i = 0; i < LENGTH; i++) {
            uint8_t pos = (rb->tail + i) % rb->capacity; // Calcula la posición circular
            temp[i] = rb->buffer[pos]; // Almacena el byte en el arreglo temporal
        }

        // Compara el comando leído con el comando OPEN
        if (memcmp(temp, OPEN, LENGTH) == 0) {
            // Enciende el LED (simula abrir la puerta)
            HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
            // Envía un mensaje indicando que la puerta está abierta
            uart_send_string("\r\nDoor OPEN (LD2 ON)\r\n");
            // Elimina el comando del buffer circular
            for (int i = 0; i < LENGTH; i++) {
                uint8_t dummy;
                ring_buffer_read(rb, &dummy);
            }
        }
        // Compara el comando leído con el comando CLOSE
        else if (memcmp(temp, CLOSE, LENGTH) == 0) {
            // Apaga el LED (simula cerrar la puerta)
            HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
            // Envía un mensaje indicando que la puerta está cerrada
            uart_send_string("\r\nDoor CLOSED (LD2 OFF)\r\n");
            // Elimina el comando del buffer circular
            for (int i = 0; i < LENGTH; i++) {
                uint8_t dummy;
                ring_buffer_read(rb, &dummy);
            }
        }
        // Compara el comando leído con el comando STATUS
        else if (memcmp(temp, STATUS, LENGTH) == 0) {
            // Lee el estado actual del LED (simula el estado de la puerta)
            uint8_t state = HAL_GPIO_ReadPin(LD2_GPIO_Port, LD2_Pin);
            // Envía un mensaje con el estado actual
            uart_send_string(state ? "\r\nStatus: OPEN (LD2 ON)\r\n" : "\r\nStatus: CLOSED (LD2 OFF)\r\n");
            // Elimina el comando del buffer circular
            for (int i = 0; i < LENGTH; i++) {
                uint8_t dummy;
                ring_buffer_read(rb, &dummy);
            }
        }
        // Compara el comando leído con el comando CLEAR
        else if (memcmp(temp, CLEAR, LENGTH) == 0) {
            // Limpia el buffer circular
            ring_buffer_reset(rb);
            // Envía un mensaje indicando que el buffer fue limpiado
            uart_send_string("\r\nBuffer cleared\r\n");
            break; // Sale del bucle, ya que el buffer fue limpiado
        }
        // Si no se encuentra un comando válido
        else {
            // Descarta un byte del buffer para continuar procesando
            uint8_t discard;
            ring_buffer_read(rb, &discard);
        }
    }
}