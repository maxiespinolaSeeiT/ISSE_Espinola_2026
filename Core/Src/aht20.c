/**
 ******************************************************************************
 * @file    aht20.c
 * @brief   Implementacón del driver AHT20 (I2C).
 *
 * Protocolo del sensor (hoja de datos AHT20):
 *   1) Al energizar, esperar >= 40 ms.
 *   2) Verificar bit de calibración (bit 3) del byte de estado; si no esta
 *      seteado, enviar comando de inicializacion 0xBE 0x08 0x00.
 *   3) Para medir: enviar 0xAC 0x33 0x00 y esperar >= 80 ms.
 *   4) Leer 6 (o 7) bytes: [status][RH 20 bits][T 20 bits (comparten 1 byte)].
 ******************************************************************************
 */
#include "aht20.h"

/* ---- Constantes privadas del modulo (no expuestas en el .h) --------- */
#define AHT20_CMD_INIT          {0xBE, 0x08, 0x00}
#define AHT20_CMD_MEASURE       {0xAC, 0x33, 0x00}
#define AHT20_STATUS_BUSY_MASK  0x80U
#define AHT20_STATUS_CAL_MASK   0x08U
#define AHT20_I2C_TIMEOUT_MS    100U

/* ---- Prototipos privados --------------------------------------------- */
static bool AHT20_ReadStatus(AHT20Sensor *self, uint8_t *status);

bool AHT20_Init(AHT20Sensor *self, I2C_HandleTypeDef *hi2c)
{
    uint8_t init_cmd[3] = AHT20_CMD_INIT;
    uint8_t status = 0;

    self->hi2c_ = hi2c;
    self->address_ = AHT20_I2C_ADDRESS;
    self->temperature_ = 0.0f;
    self->humidity_ = 0.0f;
    self->calibrated_ = false;

    HAL_Delay(40); /* tiempo minimo de power-on del AHT20 */

    if (!AHT20_ReadStatus(self, &status)) {
        return false;
    }

    if ((status & AHT20_STATUS_CAL_MASK) == 0U) {
        if (HAL_I2C_Master_Transmit(self->hi2c_, self->address_, init_cmd,
                                     sizeof(init_cmd), AHT20_I2C_TIMEOUT_MS) != HAL_OK) {
            return false;
        }
        HAL_Delay(10);
    }

    if (!AHT20_ReadStatus(self, &status)) {
        return false;
    }

    self->calibrated_ = ((status & AHT20_STATUS_CAL_MASK) != 0U);
    return self->calibrated_;
}

bool AHT20_ReadMeasurement(AHT20Sensor *self)
{
    uint8_t measure_cmd[3] = AHT20_CMD_MEASURE;
    uint8_t data[6] = {0};
    uint32_t raw_hum;
    uint32_t raw_temp;

    if (!self->calibrated_) {
        return false;
    }

    if (HAL_I2C_Master_Transmit(self->hi2c_, self->address_, measure_cmd,
                                 sizeof(measure_cmd), AHT20_I2C_TIMEOUT_MS) != HAL_OK) {
        return false;
    }

    HAL_Delay(80); /* tiempo de conversion tipico del AHT20 */

    if (HAL_I2C_Master_Receive(self->hi2c_, self->address_, data,
                                sizeof(data), AHT20_I2C_TIMEOUT_MS) != HAL_OK) {
        return false;
    }

    if ((data[0] & AHT20_STATUS_BUSY_MASK) != 0U) {
        return false; /* el sensor seguia ocupado: descartar lectura */
    }

    raw_hum  = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | (data[3] >> 4);
    raw_temp = (((uint32_t)data[3] & 0x0FU) << 16) | ((uint32_t)data[4] << 8) | data[5];

    self->humidity_    = ((float)raw_hum / 1048576.0f) * 100.0f;
    self->temperature_ = (((float)raw_temp / 1048576.0f) * 200.0f) - 50.0f;

    return true;
}

float AHT20_GetTemperature(const AHT20Sensor *self)
{
    return self->temperature_;
}

float AHT20_GetHumidity(const AHT20Sensor *self)
{
    return self->humidity_;
}

/* ---- Privados ---------------------------------------------------------- */
static bool AHT20_ReadStatus(AHT20Sensor *self, uint8_t *status)
{
    return HAL_I2C_Master_Receive(self->hi2c_, self->address_, status, 1,
                                   AHT20_I2C_TIMEOUT_MS) == HAL_OK;
}
