#include "control_system.h"  // Librería específica del sistema de control
#include "string.h"          // Librería estándar para manejo de cadenas
#include "ring_buffer.h"     // Librería para manejo de buffers circulares
#include "main.h"            // Configuraciones principales del microcontrolador
#include "ssd1306.h"
#include "ssd1306_conf.h"
#include "ssd1306_fonts.h"
#include "closed_padlock.h"
#include "closed_text.h"
#include "open_padlock.h"
#include "open_text.h"
// Definición de constantes
const char OPEN[] = "#*A*#";   // Comando para abrir
const char CLOSE[] = "#*C*#";  // Comando para cerrar
const char STATUS[] = "#*1*#"; // Comando para consultar el estado
const char CLEAR[] = "#*0*#";  // Comando para limpiar el buffer
// Variables globales
volatile uint8_t button_press_count = 0;
volatile uint32_t last_button_press_time = 0;

// Definición de buffers circulares y variables
ring_buffer_t keypad;         // Buffer circular para el teclado
uint8_t keypad_mem[64];       // Memoria reservada para el buffer del teclado
ring_buffer_t terminal;       // Buffer circular para la terminal
uint8_t terminal_mem[64];    // Memoria reservada para el buffer de la terminal
ring_buffer_t esp01_buffer;
uint8_t esp01_tx_byte_mem[64];
uint8_t esp01_rx_byte;            // Byte recibido por UART3
char current_cmd[LENGTH];     // Almacena el comando actual que se está procesando
uint8_t cmd_index = 0;        // Índice para rastrear la posición en el comando actual
extern UART_HandleTypeDef huart2; // Manejador de UART2 (definido externamente)
extern uint8_t rx_byte;       // Byte recibido por UART (definido externamente)
extern UART_HandleTypeDef huart3; // Manejador de UART3 (definido externamente)
// Función para inicializar el sistema de control
void control_system_init(void) {
    // Inicializa el buffer circular del teclado
    ring_buffer_init(&keypad, keypad_mem, sizeof(keypad_mem));
    // Limpia el buffer de comandos actual
    memset(current_cmd, 0, LENGTH);
    // Inicializa el buffer circular de la terminal
    ring_buffer_init(&terminal, terminal_mem, sizeof(terminal_mem));
    // Inicializa el buffer circular para la comunicación con ESP01
    ring_buffer_init(&esp01_buffer, esp01_tx_byte_mem, sizeof(esp01_tx_byte_mem));
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
    // Verifica si la interrupción es de UART3 (ESP01)
    else if (huart->Instance == USART3) {
        ring_buffer_write(&esp01_buffer, esp01_rx_byte);
        // Reinicia la recepción en UART3 para el siguiente byte
        HAL_UART_Receive_IT(&huart3, &esp01_rx_byte, 1);
        // Envía el byte recibido por UART3 (ESP01) de vuelta a través de UART2, como un "eco" para fines de depuración,
        HAL_UART_Transmit(&huart2, &esp01_rx_byte, 1, 100); 
    }
}

// Función para enviar una cadena de caracteres por UART
void uart_send_string(const char *str) {
    // Transmite la cadena a través de UART2
    HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), 100);
    HAL_UART_Transmit(&huart3, (uint8_t *)str, strlen(str), 100);
}

// Función principal para procesar comandos
void process_commands(void) {
    process_buffer_commands(&terminal);   // Procesa los comandos del buffer de la terminal
    process_buffer_commands(&keypad);     // Procesa los comandos del buffer del teclado
    process_buffer_commands(&esp01_buffer); // Procesa los comandos del buffer de ESP01
}
void ssd1306_On_Led(void){
    ssd1306_Fill(Black); // Limpiar pantalla
    ssd1306_DrawBitmap(0, 0, open_padlock, 128, 64, White);
    ssd1306_UpdateScreen();
    HAL_Delay(500);
    ssd1306_Fill(Black); // Limpiar pantalla
    ssd1306_DrawBitmap(0, 0, open_text, 128, 64, White);
    ssd1306_UpdateScreen();
    HAL_Delay(500);
}
void ssd1306_Off_Led(void){
    ssd1306_Fill(Black); // Limpiar pantalla
    ssd1306_DrawBitmap(0, 0, closed_padlock, 128, 64, White);
    ssd1306_UpdateScreen();
    HAL_Delay(500);
    ssd1306_Fill(Black); // Limpiar pantalla
    ssd1306_DrawBitmap(0, 0, closed_text, 128, 64, White);
    ssd1306_UpdateScreen();
    HAL_Delay(500);
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
            HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_RESET);
            // Envía un mensaje indicando que la puerta está abierta
            uart_send_string("\r\nDoor OPEN (LD2 ON)\r\n");
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
            ssd1306_On_Led();
            // Elimina el comando del buffer circular
            for (int i = 0; i < LENGTH; i++) {
                uint8_t dummy;
                ring_buffer_read(rb, &dummy);
            }
        }
        // Compara el comando leído con el comando CLOSE
        else if (memcmp(temp, CLOSE, LENGTH) == 0) {
            // Apaga el LED (simula cerrar la puerta)
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
            // Envía un mensaje indicando que la puerta está cerrada
            uart_send_string("\r\nDoor CLOSED (LD2 OFF)\r\n");
            HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_SET); 
            ssd1306_Off_Led();         
            // Elimina el comando del buffer circular
            for (int i = 0; i < LENGTH; i++) {
                uint8_t dummy;
                ring_buffer_read(rb, &dummy);
            }
        }
        // Compara el comando leído con el comando STATUS
        else if (memcmp(temp, STATUS, LENGTH) == 0) {
            // Lee el estado actual del LED (simula el estado de la puerta)
            GPIO_PinState estado_LED1 = HAL_GPIO_ReadPin(LED1_GPIO_Port, LED1_Pin);
            GPIO_PinState estado_LED4 = HAL_GPIO_ReadPin(LED4_GPIO_Port, LED4_Pin);

            if (estado_LED1 == GPIO_PIN_SET){
                HAL_UART_Transmit(&huart2,(uint8_t*)"Estado: Abierta\r\n",17,100);
                HAL_UART_Transmit(&huart2,(uint8_t*)"Estado: Abierta\r\n",17,100);
                ssd1306_On_Led();
            }
            else if (estado_LED4 == GPIO_PIN_SET){
                HAL_UART_Transmit(&huart2,(uint8_t*)"Estado: Abierta\r\n",17,100);
                HAL_UART_Transmit(&huart2,(uint8_t*)"Estado: Abierta\r\n",17,100);
                ssd1306_Off_Led();
            }
            else{
                HAL_UART_Transmit(&huart2,(uint8_t*)"Estado: Reiniciando\r\n",21,100);
                HAL_UART_Transmit(&huart3,(uint8_t*)"Estado: Reiniciando\r\n",21,100);
                ssd1306_Fill(Black); // Limpiar pantalla
                ssd1306_SetCursor(20,20);
                ssd1306_WriteString("Buffer Cleared", Font_7x10, White); // Muestra "Buffer Cleared"
                ssd1306_UpdateScreen();
                HAL_Delay(1000); // Muestra el mensaje por 1 segundo

            }
            // uint8_t state = HAL_GPIO_ReadPin(LED1_GPIO_Port, LED1_Pin);
            // uint8_t state2 = HAL_GPIO_ReadPin(LED4_GPIO_Port, LED4_Pin);
            // // Envía un mensaje con el estado actual
            // uart_send_string(state ? "\r\nStatus: OPEN (LD2 ON)\r\n" : "\r\nStatus: CLOSED (LD2 OFF)\r\n");
            // // Envía un mensaje con el estado actual
            // if (state) {
            //     // Si la puerta está abierta, muestra el icono y texto de "abierto"
            //     ssd1306_On_Led();
            // } else {
            //     // Si la puerta está cerrada, muestra el icono y texto de "cerrado"
            //     ssd1306_Off_Led();
            // }
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
            // Reinicia el sistema
            HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin,GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin,GPIO_PIN_RESET);
            // Muestra un mensaje en la pantalla SSD1306 indicando que el buffer ha sido limpiado
            ssd1306_Fill(Black); // Limpiar pantalla
            ssd1306_SetCursor(20,20);
            ssd1306_WriteString("Buffer Cleared", Font_7x10, White); // Muestra "Buffer Cleared"
            ssd1306_UpdateScreen();
            HAL_Delay(1000); // Muestra el mensaje por 1 segundo
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

void process_button(void)
{
    // Si ha pasado el tiempo de espera sin nuevos presionados y hubo al menos uno
    if ((HAL_GetTick() - last_button_press_time) > BUTTON_PRESS_TIMEOUT && button_press_count > 0)
    {
        if (button_press_count == 1)
        {
            // Acción para una sola presión: enciende el LED (abre la puerta)
            HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
            uart_send_string("\r\nDoor OPEN (LD2 ON) via button\r\n");
            ssd1306_On_Led();
            button_press_count = 0;
        }
        else if (button_press_count == 2)
        {
            // Acción para dos presiones: apaga el LED (cierra la puerta)
            HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
            uart_send_string("\r\nDoor CLOSED (LD2 OFF) via button\r\n");
            ssd1306_Off_Led();
            button_press_count = 0;
        }
        // Si se presionó más de dos veces, no se realiza acción

        // Reinicia el contador para la siguiente secuencia
        button_press_count = 0;
    }
}