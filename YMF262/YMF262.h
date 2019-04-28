#ifndef __YMF262_H__
#define __YMF262_H__

#include "mbed.h"
#include <stdint.h>

class YMF262
{
public:
	YMF262(PinName p_cs, PinName p_rd, PinName p_wr, PinName p_a1, PinName p_a0, PinName p_ic, PinName p_dataBus[8]);
	~YMF262();
	
	// write val to reg on array (0 or 1) 
	void write_reg(uint8_t array, uint8_t reg, uint8_t val);
	uint8_t read_status();
	void clear();
	void tst_all_high();
	
private:
	DigitalOut CS;	// when CS=H, bus in high impedance
	DigitalOut RD;	// L for read, H for write
	DigitalOut WR;	// L for write, H for read
	DigitalOut A0;	// L for address write, H for data write
	DigitalOut A1;	// L for register array 0, H for array 1
	DigitalInOut IC;	// briefly move low to clear all registers
	BusInOut DataBus;	// 8-bit data bus
	
	void ic_high();
	void ic_low();
	
};

#endif