#include "bitbang_i2c.h"

static void bbi2c_send_one(BBI2C* bb){
  nrf_gpio_pin_clear(bb->scl_pin);
  nrf_gpio_pin_set(bb->sda_pin);
  BBI2C_DELAY();
  nrf_gpio_pin_set(bb->scl_pin);
  BBI2C_DELAY();
}

static void bbi2c_send_zero(BBI2C* bb){
  nrf_gpio_pin_clear(bb->scl_pin);
  nrf_gpio_pin_clear(bb->sda_pin);
  BBI2C_DELAY();
  nrf_gpio_pin_set(bb->scl_pin);
  BBI2C_DELAY();
}

static bool bbi2c_wait_ack(BBI2C* bb){
  nrf_gpio_pin_clear(bb->scl_pin);
  nrf_gpio_pin_set(bb->sda_pin);
  BBI2C_DELAY();
  nrf_gpio_pin_set(bb->scl_pin);
  BBI2C_DELAY();
  bool res=(nrf_gpio_pin_read(bb->sda_pin)==0);
  nrf_gpio_pin_clear(bb->scl_pin);
  return res;
}

static void bbi2c_send_ack(BBI2C* bb){
  nrf_gpio_pin_clear(bb->scl_pin);
  nrf_gpio_pin_clear(bb->sda_pin);
  BBI2C_DELAY();
  nrf_gpio_pin_set(bb->scl_pin);
  BBI2C_DELAY();
  nrf_gpio_pin_clear(bb->scl_pin);
}

static void bbi2c_send_no_ack(BBI2C* bb){
  nrf_gpio_pin_clear(bb->scl_pin);
  nrf_gpio_pin_set(bb->sda_pin);
  BBI2C_DELAY();
  nrf_gpio_pin_set(bb->scl_pin);
  BBI2C_DELAY();
  nrf_gpio_pin_clear(bb->scl_pin);
}

static bool bbi2c_read_bit(BBI2C* bb){
  nrf_gpio_pin_clear(bb->scl_pin);
  nrf_gpio_pin_set(bb->sda_pin);
  BBI2C_DELAY();
  nrf_gpio_pin_set(bb->scl_pin);
  BBI2C_DELAY();
  return (nrf_gpio_pin_read(bb->sda_pin)>0);
}

static void bbi2c_send_start(BBI2C* bb){
  BBI2C_DELAY();
  nrf_gpio_pin_set(bb->sda_pin);
  nrf_gpio_pin_set(bb->scl_pin);
  BBI2C_DELAY();
  nrf_gpio_pin_clear(bb->sda_pin);
  BBI2C_DELAY();
  nrf_gpio_pin_clear(bb->scl_pin);
}

static void bbi2c_send_stop(BBI2C* bb){
  nrf_gpio_pin_clear(bb->sda_pin);
  nrf_gpio_pin_clear(bb->scl_pin);
  BBI2C_DELAY();
  nrf_gpio_pin_set(bb->scl_pin);
  BBI2C_DELAY();
  nrf_gpio_pin_set(bb->sda_pin);
}

static bool bbi2c_send_byte(BBI2C* bb,uint8_t ba){
  for(int32_t i=0;i<8;i++){
    if((ba & 0x80)>0){
      bbi2c_send_one(bb);
    }else{
      bbi2c_send_zero(bb);
    }
    ba<<=1;
  }
  return bbi2c_wait_ack(bb);
}

static void bbi2c_read_byte(BBI2C* bb,uint8_t* ba,bool req_more){
  uint8_t tmp=0;
  for(int32_t i=0;i<8;i++){
    if(bbi2c_read_bit(bb)){
      tmp |=1;
    }tmp<<=1;
  }
  if(req_more){
    bbi2c_send_ack(bb);
  }else{
    bbi2c_send_no_ack(bb);
  }
  *ba=tmp;
}

void bbi2c_setup(BBI2C* bb,uint32_t sda,uint32_t scl){
  bb->scl_pin=scl;
  bb->sda_pin=sda;
  
  //if there are pull-up resistor in the circuit:
  //one can replace NRF_GPIO_PIN_PULLUP to NRF_GPIO_PIN_NOPULL
  nrf_gpio_cfg(
      sda,
      NRF_GPIO_PIN_DIR_OUTPUT,
      NRF_GPIO_PIN_INPUT_CONNECT,
      NRF_GPIO_PIN_PULLUP,
      NRF_GPIO_PIN_S0D1,
      NRF_GPIO_PIN_NOSENSE);
  
  nrf_gpio_cfg(
      scl,
      NRF_GPIO_PIN_DIR_OUTPUT,
      NRF_GPIO_PIN_INPUT_DISCONNECT,
      NRF_GPIO_PIN_PULLUP,
      NRF_GPIO_PIN_S0D1,
      NRF_GPIO_PIN_NOSENSE);
  
  nrf_gpio_pin_set(sda);
  nrf_gpio_pin_set(scl);
}

bool bbi2c_write(BBI2C* bb,uint8_t add,uint8_t* dat,int32_t len,bool req_no_stop){  
  bool res;
  
  bbi2c_send_start(bb);
  
  add<<=1;
  res=bbi2c_send_byte(bb,add);
  if(res==false){
    bbi2c_send_stop(bb);
    return false;
  }
  
  for(int i=0;i<len;i++){
    res=bbi2c_send_byte(bb, *dat);
    if(res==false){
      bbi2c_send_stop(bb);
      return false;
    }
    dat++;
  }
  
  if(req_no_stop==false){
    bbi2c_send_stop(bb);
  }
  return true;
}
bool bbi2c_read(BBI2C* bb,uint8_t add,uint8_t* dat,int32_t len){
  bool res;
  
  bbi2c_send_start(bb);
  
  add<<=1;
  add|=0x01;
  res=bbi2c_send_byte(bb, add);
  if(res==false){
    bbi2c_send_stop(bb);
    return false;
  }
  
  for(int i=0;i<len;i++){
    bbi2c_read_byte(bb, dat,(i<(len-1)));
    dat++;
  }
  
  bbi2c_send_stop(bb);
  return true;
}