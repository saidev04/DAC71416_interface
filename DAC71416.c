#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"
#include "delay.h"
#include "lcd16x2.h"

#define SPIx_RCC				RCC_APB2Periph_SPI1
#define SPIx						SPI1
#define SPI_GPIO_RCC		RCC_APB2Periph_GPIOA
#define SPI_GPIO				GPIOA
#define SPI_PIN_MOSI		GPIO_Pin_7
#define SPI_PIN_MISO		GPIO_Pin_6
#define SPI_PIN_SCK			GPIO_Pin_5
#define SPI_PIN_SS			GPIO_Pin_4

void SPIx_Init(void);
uint8_t SPIx_Transfer(uint8_t data);
void SPIx_EnableSlave(void);
void SPIx_DisableSlave(void);
void set_dac(uint32_t data);
void set_dac_16(uint16_t data);


	
uint8_t receivedByte;
uint16_t i=0;
uint16_t A = 0x37FF;

int main(void)
{
SystemInit();
SPIx_Init();	
while(1)
{
SPIx_EnableSlave();	
SPIx_Transfer(0x01);//enable STATUS0
SPIx_Transfer(0x05);//Read new data
SPIx_DisableSlave();
		
SPIx_EnableSlave();
SPIx_Transfer(0x04);// enable MODE2 
SPIx_Transfer(0x11);//GPIO pin connected, directed as an input
SPIx_DisableSlave();

SPIx_EnableSlave();
SPIx_Transfer(0x05);//enable MODE3
SPIx_Transfer(0x41);//enable STATUS0 , Read or write the GPIO data on the REFP1/GPIO0 pin.
SPIx_DisableSlave();
	
SPIx_EnableSlave();
SPIx_Transfer(0x06);//enable Reference configuration
SPIx_Transfer(0x10);//set Internal REF
SPIx_DisableSlave();

SPIx_EnableSlave();
SPIx_Transfer(0x10);//enable MODE4
SPIx_Transfer(0x10);//AIN0-AIN1, PGA gain 
SPIx_DisableSlave();

SPIx_EnableSlave();
SPIx_Transfer(0x11);//disable PGA ALARAM
SPIx_Transfer(0x00);
SPIx_DisableSlave();

SPIx_EnableSlave();
SPIx_Transfer(0x12);//disable status2
SPIx_Transfer(0x00);
SPIx_DisableSlave();

}
}

void set_dac_16(uint16_t data)
{
	 uint8_t msb_low, lsb_high,lsb_low;
            
            lsb_low  =  (uint8_t)(0x000000ff & data);
            
            lsb_high =  (uint8_t)( (0x0000ff00 & data) >> 8 );
	
						msb_low  =  (uint8_t)( (0x00ff0000 & data) >> 16);
            
            
            
		SPIx_EnableSlave();
		SPIx_Transfer(lsb_high);
		DelayUs(10);
		SPIx_Transfer(lsb_low);
		DelayUs(10);
		SPIx_DisableSlave();
		DelayUs(10);
	

}

void set_dac(uint32_t data)
{
	 uint8_t msb_low,lsb_high,lsb_low;
            
            lsb_low  =  (uint8_t)(0x000000ff & data);
            
            lsb_high =  (uint8_t)( (0x0000ff00 & data) >> 8 );
            
            msb_low  =  (uint8_t)( (0x00ff0000 & data) >> 16);
            
            
		SPIx_EnableSlave();
		SPIx_Transfer(msb_low);
		DelayUs(10);
		SPIx_Transfer(lsb_high);
		DelayUs(10);
		SPIx_Transfer(lsb_low);
		DelayUs(10);
		SPIx_DisableSlave();
		DelayUs(10);
	

}

void SPIx_Init()
{
	// Initialization struct

	SPI_InitTypeDef SPI_InitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
	
	// Step 1: Initialize SPI
	RCC_APB2PeriphClockCmd(SPIx_RCC, ENABLE);
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft | SPI_NSSInternalSoft_Set;
	SPI_Init(SPIx, &SPI_InitStruct); 
	SPI_Cmd(SPIx, ENABLE);
	
	// Step 2: Initialize GPIO
	RCC_APB2PeriphClockCmd(SPI_GPIO_RCC, ENABLE);
	// GPIO pins for MOSI, MISO, and SCK
	GPIO_InitStruct.GPIO_Pin = SPI_PIN_MOSI | SPI_PIN_MISO | SPI_PIN_SCK;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI_GPIO, &GPIO_InitStruct);
	// GPIO pin for SS
	GPIO_InitStruct.GPIO_Pin = SPI_PIN_SS;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI_GPIO, &GPIO_InitStruct);
	
	// Disable SPI slave device
	SPIx_DisableSlave();
}

uint8_t SPIx_Transfer(uint8_t data)
{
	// Write data to be transmitted to the SPI data register
	uint8_t dat;
	SPIx->DR = data;
	// Wait until transmit complete
	while (!(SPIx->SR & (SPI_I2S_FLAG_TXE)));
	// Wait until receive complete
	while (!(SPIx->SR & (SPI_I2S_FLAG_RXNE)));
	// Wait until SPI is not busy anymore
	while (SPIx->SR & (SPI_I2S_FLAG_BSY));
	// Return received data from SPI data register
		//return SPIx->DR;
	dat = SPIx->DR;
	return dat;
}

void SPIx_EnableSlave()
{
	// Set slave SS pin low
	SPI_GPIO->BRR = SPI_PIN_SS;
}

void SPIx_DisableSlave()
{
	// Set slave SS pin high
	SPI_GPIO->BSRR = SPI_PIN_SS;
}
