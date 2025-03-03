# Sistema de Control de Acceso y Seguridad

Este proyecto implementa un sistema de control de acceso que integra múltiples interfaces para gestionar la apertura y cierre de una "cerradura" (simulada mediante LEDs) y la notificación de eventos. Se utiliza un microcontrolador (por ejemplo, la placa Nucleo_L476RG) junto con periféricos como pantalla OLED, teclado matricial, módulos de comunicación UART y sensores, permitiendo la interacción a través de comandos, botones y eventos externos.

---

## 1. Descripción General

El sistema está diseñado para:

- **Procesar comandos:** Se reciben instrucciones desde distintas interfaces (teclado, terminal vía UART2, módulo ESP01 vía UART3) para abrir o cerrar la puerta.
- **Manejo de eventos:** Se utilizan interrupciones para detectar pulsaciones en el teclado, activación del sensor infrarrojo de proximidad (para notificar si hay alguien frente a la puerta) y la activación del timbre.
- **Retroalimentación visual:** Una pantalla OLED (SSD1306) muestra mensajes e imágenes (candado abierto/cerrado, mensajes de bienvenida, notificaciones, etc.).
- **Gestión de datos:** Se emplean buffers circulares para almacenar y procesar los comandos recibidos de forma eficiente.

---

## 2. Componentes de Hardware

Los principales componentes de hardware utilizados en el proyecto son:

- **Placa Microcontrolador:** Nucleo_L476RG (STM32L476RG)
- **Pantalla OLED:** SSD1306 junto con su configuración y fuentes (ssd1306_conf, ssd1306_fonts)
- **Teclado Matricial:** Utilizado para la entrada manual de comandos y claves
- **LEDs:**
  - LED de apertura (simula el candado abierto)
  - LED de cierre (simula el candado cerrado)
  - LED de "heartbeat" (indicador de funcionamiento del sistema)
- **Sensor Infrarrojo de Proximidad:** Detecta la presencia de una persona frente a la puerta
- **Módulo WiFi:** ESP01, que se comunica a través de UART3 para la recepción de comandos remotos
- **Botones Físicos:**
  - Botón interior (para activar apertura o cierre mediante pulsaciones simples o dobles)
  - Botón externo (timbre)

---

## 3. Módulos Implementados

El firmware se organiza en módulos para mantener una arquitectura modular y escalable. Los módulos implementados incluyen:

1. **Gestión de Periféricos:**
   - Inicialización y configuración de GPIO, UART e I2C.
   - Configuración de la pantalla OLED, incluyendo drivers y fuentes.
   - Implementación de buffers circulares para manejo de datos (teclado, terminal, ESP01).

2. **Drivers de Componentes:**
   - **Driver SSD1306:** Control y actualización de la pantalla OLED.
   - **Driver del Teclado Matricial:** Escaneo y gestión de pulsaciones.
   - **Control de LEDs:** Indicadores visuales para apertura y cierre.
   - **Manejo del Sensor Infrarrojo:** Detección de presencia mediante sensor de proximidad.
   - **Módulo ESP01:** Comunicación vía UART para la integración WiFi.

3. **Comunicación y Procesamiento de Comandos:**
   - Recepción de datos vía UART2 y UART3.
   - Uso de funciones de "eco" para depuración.
   - Procesamiento de comandos mediante buffers circulares (función `process_buffer_commands()`).

4. **Lógica de Control y Manejo de Estados:**
   - Gestión de comandos para abrir, cerrar, consultar estado y limpiar buffers.
   - Procesamiento de pulsaciones del botón (pulsación única para abrir y doble pulsación para cerrar).
   - Notificación y respuesta ante la detección del sensor infrarrojo.
   - Actualización de la pantalla OLED según el estado actual del sistema.

5. **Interrupciones y Eventos Externos:**
   - Manejador de interrupciones (GPIO y UART) para gestionar entradas del teclado, sensor y timbre.
   - Funciones específicas para el procesamiento de eventos (como `process_sensor()` y `process_timbre()`).

---

## 4. Diagrama de Conexiones

El siguiente diagrama ilustra las conexiones físicas entre la placa, los periféricos y los módulos de comunicación:

![Diagrama de conexiones](Assests\Diagrama_Conexiones.jpg)

---

## 5. Máquina de Estados del Sistema

El sistema gestiona el estado de la cerradura mediante comandos y pulsaciones del botón. Además, el sensor infrarrojo de proximidad notifica la presencia de una persona frente a la puerta sin alterar el estado de la cerradura. El diagrama a continuación resume las transiciones principales:

![Maquina de Estados del Sistema](Assests\Diagrama_de_estados.png)

**Notas adicionales:**

- **De CERRADO a ABIERTO:**  
  Se produce al recibir el comando `#*A*#` o mediante una pulsación simple del botón, activando la apertura (se enciende el LED y se muestra la imagen de candado abierto).

- **De ABIERTO a CERRADO:**  
  Se produce al recibir el comando `#*C*#`, mediante una doble pulsación del botón o a través de un timeout/consulta (comando `#*1*#`), lo que cierra la puerta (se reactivan los indicadores de cerrado).

- **CLEAR (#*0*#):**  
  Este comando reinicia el buffer y fuerza la transición al estado CERRADO, independientemente del estado actual.

- **Sensor Infrarrojo de Proximidad:**  
  El sensor detecta la presencia de una persona frente a la puerta y notifica:
  - Si la puerta está cerrada, se muestra la pantalla de "Password" para solicitar el ingreso.
  - Si la puerta está abierta, se refuerza la visualización del candado abierto.
  
  Esta notificación se realiza en paralelo al manejo de estados sin modificar la lógica de transición.

---

## 6. Estructura del Software

El firmware se organiza en módulos que incluyen:

- **Inicialización y Configuración:**  
  - `control_system_init()`: Inicializa los buffers circulares para el teclado, terminal y comunicación con ESP01, y limpia el buffer de comandos.  
  - Configuración de periféricos (GPIO, UART, I2C) y de la pantalla OLED.

- **Manejo de Comunicaciones y Comandos:**  
  - `HAL_UART_RxCpltCallback()`: Procesa la recepción de datos vía UART2 (para comandos desde el PC o teclado) y UART3 (para el módulo ESP01), enviando además un "eco" para fines de depuración.  
  - `process_buffer_commands()`: Extrae y compara bloques de datos del buffer para ejecutar las acciones correspondientes según el comando recibido.  
  - `uart_send_string()`: Envía mensajes informativos a través de las interfaces UART.

- **Gestión de Eventos Externos:**  
  - **Interrupciones GPIO:** `HAL_GPIO_EXTI_Callback()` gestiona eventos del teclado, sensor de presencia y timbre, aplicando técnicas de "debounce".  
  - **Eventos por Botón:** `process_button()` detecta pulsaciones simples y dobles para abrir o cerrar la puerta respectivamente.  
  - **Procesamiento de Sensor y Timbre:**  
    - `process_sensor()` se encarga de notificar la presencia de una persona frente a la puerta (utilizando el sensor infrarrojo) y actualizar la pantalla según el estado actual de la cerradura.  
    - `process_timbre()` gestiona la activación del timbre.

- **Actualización Visual:**  
  Se utilizan funciones específicas (como `ssd1306_On_Led()`, `ssd1306_Off_Led()`, `ssd1306_Clean_Led()`, `ssd1306_Password()`, `ssd1306_Welcome()`) para actualizar la pantalla OLED en función del estado actual del sistema.

---

## 7. Pasos para Completar el Proyecto

1. **Validación y Ajustes:**  
   - Verificar la detección y procesamiento correcto de comandos y pulsaciones.  
   - Ajustar los tiempos de debounce y timeout para obtener respuestas precisas.

2. **Integración de Funcionalidades Adicionales:**  
   - Implementar mecanismos de seguridad (registro de intentos fallidos, bloqueo tras múltiples errores).  
   - Ampliar la conectividad y protocolos para el módulo ESP01.

3. **Optimización y Documentación:**  
   - Refinar la gestión de buffers circulares y la sincronización de interrupciones.  
   - Documentar y comentar el código para facilitar futuras actualizaciones.

4. **Pruebas y Validación:**  
   - Realizar pruebas integradas de hardware y software.  
   - Utilizar herramientas de depuración (UART, pantalla OLED) para ajustar el comportamiento del sistema.

---

## 8. Herramientas de Desarrollo

- **IDE y Configuración:**  
  - VS Code o STM32CubeIDE para el desarrollo del firmware.  
  - STM32CubeMX para la configuración inicial de periféricos.

- **Depuración y Comunicación:**  
  - Terminales como YAT o TeraTerm para monitorear la salida UART.  
  - Módulo ESP01 para pruebas de conectividad WiFi.

- **Documentación y Referencias:**  
  - Hojas de datos y manuales de la placa Nucleo_L476RG.  
  - Documentación de la biblioteca HAL de STM32 y drivers de los periféricos.

---


Este documento resume la arquitectura, componentes de hardware, módulos implementados, funcionalidades y la lógica de control del sistema de acceso, facilitando su comprensión, mantenimiento y futura ampliación.

*Este proyecto es parte del curso de Estructuras Computacionales en la Universidad Nacional de Colombia - Sede Manizales.*