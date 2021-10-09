#ifndef __AAA_BIT_BANG_I2C_H__
#define __AAA_BIT_BANG_I2C_H__

#include <stdint.h>
#include <stdbool.h>

#include "nrf_gpio.h"
#include "nrf_delay.h"

typedef struct{
  uint32_t sda_pin;
  uint32_t scl_pin; 
}BBI2C;

#define BBI2C_DELAY()   nrf_delay_us(10);
//#define BBI2C_DELAY()   nrf_delay_ms(1)
//#define BBI2C_DELAY()   do{for(int q=0;q<100;q++){}}while(0)

void bbi2c_setup(BBI2C* bb,uint32_t sda,uint32_t scl);
bool bbi2c_write(BBI2C* bb,uint8_t add,uint8_t* dat,int32_t len,bool req_no_stop);
bool bbi2c_read(BBI2C* bb,uint8_t add,uint8_t* dat,int32_t len);

/*
declare BBI2C struct and setup:
BBI2C lepton_bbi2c;
bbi2c_setup(&lepton_bbi2c,LEPTON_SDA_PIN,LEPTON_SCL_PIN);

replace nrf_drv_twi_tx to bbi2c_write:
nrf_drv_twi_tx(&lepton_twi, LEPTON_ADDR, cmd_reg, 4,false);
bbi2c_write(&lepton_bbi2c,LEPTON_ADDR,cmd_reg, 4,false);

replace nrf_drv_twi_rx to bbi2c_read:
nrf_drv_twi_rx(&lepton_twi, LEPTON_ADDR, val_reg, 2);
bbi2c_read(&lepton_bbi2c,LEPTON_ADDR,val_reg, 2);


for example, bytes read:

original version:
nrf_drv_twi_tx(&lepton_twi, LEPTON_ADDR, cmd_reg, 2,true);
nrf_drv_twi_rx(&lepton_twi, LEPTON_ADDR, val_reg, 2);

bit bang version:
bbi2c_write(&lepton_bbi2c,LEPTON_ADDR,cmd_reg, 2,true);
bbi2c_read(&lepton_bbi2c,LEPTON_ADDR,val_reg, 2);
*/

#endif