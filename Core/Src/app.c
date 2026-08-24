/**
 ******************************************************************************
 * @file    app.c
 * @brief   Implementación de la clase App. Aca vive toda la lógica del
 *          programa: la definición real de la estructura (privada a este
 *          archivo, siguiendo el patron de "puntero a estructura anonima"
 *          visto en clase 7 para representar objetos en C) y la máquina
 *          de estados del super-loop.
 ******************************************************************************
 */
#include "app.h"
#include "aht20.h"
#include "timer.h"
#include "serial_display.h"

#include "String.h"

/**
 * @brief Estados del ciclo de medición (ver diagrama de estados del TP).
 *        Corresponden al patron "Observar y reaccionar" con Super Loop
 *        no bloqueante (RNF-01): WAITING solo consulta Timer_Millis(),
 *        nunca usa HAL_Delay() en ese estado.
 */
typedef enum {
    APP_STATE_INIT = 0,
    APP_STATE_WAITING,
    APP_STATE_MEASURING,
    APP_STATE_DISPLAYING,
    APP_STATE_ERROR
} AppState;

/**
 * @brief Definición real de App. Es privada a app.c: fuera de este archivo
 *        solo se conoce como "struct App" (tipo incompleto), por lo que
 *        main.c no puede acceder a estos campos ni le importa como estan
 *        organizados (encapsulamiento).
 */
struct App {
    AHT20Sensor    sensor_;
    SerialDisplay  display_;
    AppState       state_;
    uint32_t       lastReadTime_;
    uint32_t 	   errorTime_;
};

HAL_StatusTypeDef status;
uint32_t error;

#define APP_SAMPLE_PERIOD_MS 5000U /* RF-01/RF-02: periodo de muestreo */

/* Única instancia del programa (RNF-05: sin heap, todo estatico). */
static App s_app;

App *App_GetInstance(void)
{
    return &s_app;
}

void App_Init(App *self, I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart)
{
    SerialDisplay_Init(&self->display_, huart);
    self->state_ = APP_STATE_INIT;

    if (AHT20_Init(&self->sensor_, hi2c)) {
		self->state_ = APP_STATE_WAITING;
		self->lastReadTime_ = Timer_Millis();
	} else {
		self->state_ = APP_STATE_ERROR;
		self->errorTime_ = Timer_Millis();
		SerialDisplay_PrintError(&self->display_, "AHT20 no responde en el bus I2C");
	}
}

void App_Run(App *self)
{
    switch (self->state_) {

    case APP_STATE_WAITING:
        if ((Timer_Millis() - self->lastReadTime_) >= APP_SAMPLE_PERIOD_MS) {
            self->state_ = APP_STATE_MEASURING;
        }
        break;

    case APP_STATE_MEASURING:
        if (AHT20_ReadMeasurement(&self->sensor_)) {
            self->state_ = APP_STATE_DISPLAYING;
        } else {
            self->state_ = APP_STATE_ERROR;
        }
        break;

    case APP_STATE_DISPLAYING:
        SerialDisplay_PrintMeasurement(&self->display_,
                                        AHT20_GetTemperature(&self->sensor_),
                                        AHT20_GetHumidity(&self->sensor_));
        self->lastReadTime_ = Timer_Millis();
        self->state_ = APP_STATE_WAITING;
        break;

    case APP_STATE_ERROR:
    	if (Timer_Millis() - self->errorTime_ >= 2000) {
			SerialDisplay_PrintError(&self->display_, "Lectura AHT20 fallida, presione RESET para reiniciar");
			self->lastReadTime_ = Timer_Millis();
			self->state_ = APP_STATE_INIT;
    	}
        break;

    case APP_STATE_INIT:

    	break;
    default:
        /* No deberia llegar aca: App_Init() siempre deja WAITING o ERROR */
        self->state_ = APP_STATE_ERROR;
        break;
    }
}
