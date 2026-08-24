/**
 * @file timer.c
 * @brief Implementación de las funciones de temporización del sistema.
 *
 * Este módulo proporciona funciones para obtener el tiempo transcurrido
 * desde el inicio del sistema y generar retardos utilizando la
 * implementación de temporización de la HAL de STM32.
 */
#include "timer.h"
#include "stm32f4xx_hal.h"


/**
 * @brief Obtiene el tiempo transcurrido desde el inicio del sistema.
 *
 * Retorna el valor del contador de tiempo del sistema proporcionado
 * por la HAL de STM32. El valor se expresa en milisegundos y se
 * incrementa periódicamente mediante la interrupción del sistema.
 *
 * @return Tiempo transcurrido desde el inicio del sistema, expresado
 *         en milisegundos.
 */
uint32_t Timer_Millis(void)
{
    return HAL_GetTick();
}

/**
 * @brief Genera un retardo bloqueante.
 *
 * Suspende la ejecución del programa durante la cantidad de
 * milisegundos especificada utilizando la función de retardo
 * proporcionada por la HAL de STM32.
 *
 * @param[in] ms Tiempo de retardo en milisegundos.
 */
void Timer_Delay(uint32_t ms)
{
    HAL_Delay(ms);
}
