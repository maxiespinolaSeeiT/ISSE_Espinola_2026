/**
 * @file serial_display.c
 * @brief Implementación de la interfaz de salida por UART para mostrar mediciones y errores.
 *
 * Este módulo proporciona funciones para inicializar una interfaz de salida
 * serie y transmitir por UART las mediciones de temperatura y humedad, así
 * como mensajes de error.
 */
#include "serial_display.h"
#include <stdio.h>
#include <string.h>


/**
 * @brief Tiempo máximo de espera para la transmisión UART.
 *
 * Tiempo de espera utilizado por HAL_UART_Transmit() para completar
 * una transmisión.
 */
#define SERIAL_TX_TIMEOUT_MS 100U

/**
 * @brief Inicializa la interfaz de salida serie.
 *
 * Asocia la instancia de SerialDisplay con el periférico UART
 * proporcionado.
 *
 * @param[in,out] self Puntero a la instancia de SerialDisplay.
 * @param[in] huart Puntero al manejador del periférico UART utilizado
 *                  para la transmisión.
 */
void SerialDisplay_Init(SerialDisplay *self, UART_HandleTypeDef *huart)
{
    self->huart_ = huart;
}

/**
 * @brief Transmite una medición de temperatura y humedad por UART.
 *
 * Formatea los valores recibidos y los transmite mediante el periférico
 * UART asociado a la instancia. La temperatura y la humedad se muestran
 * con una precisión de un decimal.
 *
 * Formato de salida:
 * @code
 * Temp: XX.X C   Hum: XX.X %RH
 * @endcode
 *
 * @param[in] self Puntero a la instancia de SerialDisplay.
 * @param[in] temperature_c Temperatura en grados Celsius.
 * @param[in] humidity_rh Humedad relativa en porcentaje.
 */
void SerialDisplay_PrintMeasurement(SerialDisplay *self, float temperature_c, float humidity_rh)
{
    char line[64];
    int len = snprintf(line, sizeof(line), "Temp: %.1f C   Hum: %.1f %%RH\r\n",
                        temperature_c, humidity_rh);

    if (len > 0) {
        HAL_UART_Transmit(self->huart_, (uint8_t *)line, (uint16_t)len, SERIAL_TX_TIMEOUT_MS);
    }
}

/**
 * @brief Transmite un mensaje de error por UART.
 *
 * Agrega el prefijo "[ERROR]" al mensaje recibido y lo transmite
 * mediante el periférico UART asociado a la instancia.
 *
 * Formato de salida:
 * @code
 * [ERROR] mensaje
 * @endcode
 *
 * @param[in] self Puntero a la instancia de SerialDisplay.
 * @param[in] message Puntero al mensaje de error terminado en '\0'.
 */
void SerialDisplay_PrintError(SerialDisplay *self, const char *message)
{
    char line[64];
    int len = snprintf(line, sizeof(line), "[ERROR] %s\r\n", message);

    if (len > 0) {
        HAL_UART_Transmit(self->huart_, (uint8_t *)line, (uint16_t)len, SERIAL_TX_TIMEOUT_MS);
    }
}
