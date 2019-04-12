#include "YMF262.h"

YMF262::YMF262(PinName p_cs, PinName p_rd, PinName p_wr, PinName p_a1, PinName p_a0, PinName p_rst, PinName p_dataBus[8]):
CS(p_cs), RD(p_rd), WR(p_wr), A0(p_a0), A1(p_a1), RST(p_rst),
DataBus(p_dataBus[0], p_dataBus[1], p_dataBus[2], p_dataBus[3], p_dataBus[4], p_dataBus[5], p_dataBus[6], p_dataBus[7])
{
	reset();
}

YMF262::~YMF262()
{
}

void YMF262::reset()
{
	CS = 0;
	RST = 1;
	wait_ms(5);
	RST = 0;
	CS = 1;
	wait_ms(5);
	RST = 1;
	wait_ms(500);
}

void YMF262::write_reg(uint8_t array, uint8_t reg, uint8_t val)
{
	A0 = 0;
	A1 = array;
	WR = 0;
	RD = 1;
	DataBus.write(reg);
	CS = 0;
	wait_us(10);
	WR = 1;
	CS = 1;
	wait_us(10);
	
	A0 = 1;
	WR = 0;
	DataBus.write(val);
	CS = 0;
	wait_us(10);
	WR = 1;
	CS = 1;
	wait_us(10);
}

uint8_t YMF262::read_status()
{
	uint8_t status = 0;
	A0 = 0;
	A1 = 0;
	WR = 1;
	RD = 0;
	
	CS = 0;
	wait_us(10);
	status = DataBus.read();
	CS = 1;
	wait_us(10);
	return status;
}