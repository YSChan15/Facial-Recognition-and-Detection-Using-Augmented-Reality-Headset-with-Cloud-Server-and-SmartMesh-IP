#include "system_setup.h"
#include "system_saml21.h"
#include "RTCClass.h"
#include "API.h"
#include "DMA.h"

void setup_system(void){
	NVMCTRL->CTRLB.reg |= 0x6;
	PM->STDBYCFG.reg |= 0x40;
	PM->PLCFG.reg = 0x2;// Set to high power mode(PL2)SUPC_REGS->SUPC_VREG |= 0x4;
	while(PM->INTFLAG.reg == 0);
	while((SUPC->STATUS.reg & 0x400) == 0);
	PM->STDBYCFG.reg |= 0x40;
	uint32_t *nvc = (uint32_t*)0x00806020;
	uint32_t osc32k_cal = (*nvc >> 6)&0x7F;
	
	// 32kHz and 1kHz Oscillator Setup => will output on clock source 0
	OSC32KCTRL->OSC32K.reg = 0x40E;// Enable the 32kHz internal oscillator => use 18 clock cycles for startup
	OSC32KCTRL->OSC32K.reg |= (osc32k_cal << 16);
	OSC32KCTRL->RTCCTRL.reg = 0x2;// 1kHz oscillator input source to RTC
	while((OSC32KCTRL->STATUS.reg & 0x2) == 0);
	PORT->Group[0].PINCFG[14].reg |= 0x1;
	PORT->Group[0].PMUX[7].reg = 0x7;
	
	// GCLK Setup
	GCLK->GENCTRL[1].reg = 0x104;// Setup 32kHz oscillator on clock source 1
	GCLK->GENCTRL[2].reg = 0x106;// Setup internal oscillator on clock source 2
	GCLK->PCHCTRL[1].reg = 0x41;// Enable DPLL and setup GCLK source => 32kHz oscillator
	GCLK->PCHCTRL[2].reg = 0x41;// Enable DPLL and setup GCLK source => 32kHz oscillator
	
	// 48MHz main clock setup
	OSCCTRL->DPLLCTRLB.reg |= 0x20;// Use GCLK as the source and setup clock division
	OSCCTRL->DPLLPRESC.reg = 0x0;// Divide output clock by 1
	OSCCTRL->DPLLRATIO.reg = 1400;// Current max value I can get out of PLL
	while((OSCCTRL->DPLLSYNCBUSY.reg & 0x4) != 0);
	OSCCTRL->DPLLCTRLA.reg = 0x2;// Enable PLL Output
	while((OSCCTRL->DPLLSYNCBUSY.reg & 0x2) != 0);
	while((OSCCTRL->DPLLSTATUS.reg & 0x1) == 0);// Wait for PLL to lock
	
	// 48Mhz DFLL Setup
	uint32_t bits = (*nvc >> 26);// Get the proper bits
	OSCCTRL->DFLLVAL.reg = (bits << 10)|0x0;// Fine frequency control(0x3FF max value)
	OSCCTRL->DFLLCTRL.reg = 0x2;
	
	GCLK->GENCTRL[0].reg = 0x108;// Change cpu clock from internal clock to PLL
	OSCCTRL->OSC16MCTRL.reg |= 0xC;// Setup internal oscillator speed to 16MHz
	
	SystemCoreClock = 48000000;

	DMAC_Driver::DMAC_Driver_Init();// DMA initialization

	// Turn on the LTE module
	PORT->Group[0].DIR.reg |= 0x80;// Set PA7 to output
	PORT->Group[0].OUTSET.reg |= 0x80;
	for(volatile int i = 0;i < 5000000;i++);
	PORT->Group[0].OUTCLR.reg |= 0x80;

	// Wait for LTE module to bootup
	for(volatile int i = 0;i < 40000000;i++);
}
