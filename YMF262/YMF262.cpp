#include "YMF262.h"

YMF262::YMF262(PinName p_cs, PinName p_rd, PinName p_wr, PinName p_a1, PinName p_a0, PinName p_ic, PinName p_dataBus[8]):
CS(p_cs), RD(p_rd), WR(p_wr), A0(p_a0), A1(p_a1), IC(p_ic),
DataBus(p_dataBus[0], p_dataBus[1], p_dataBus[2], p_dataBus[3], p_dataBus[4], p_dataBus[5], p_dataBus[6], p_dataBus[7])
{
	clear();
}

YMF262::~YMF262()
{
}

void YMF262::tst_all_high()
{
	DataBus.output();
	DataBus.write(0xFF);
	CS = 1;
	RD = 1;
	WR = 1;
	A0 = 1;
	A1 = 1;
	rst_high();
}

void YMF262::clear()
{
	CS = 0;
	rst_high();
	wait_ms(5);
	rst_low();
	CS = 1;
	wait_ms(5);
	rst_high();
	wait_ms(5);
}

void YMF262::write_reg(uint8_t array, uint8_t reg, uint8_t val)
{
	DataBus.output();
	A0 = 0;
	A1 = array;
	WR = 0;
	RD = 1;
	DataBus.write(reg);
	CS = 0;
	wait_us(1);
	WR = 1;
	CS = 1;
	wait_us(1);
	
	A0 = 1;
	WR = 0;
	DataBus.write(val);
	CS = 0;
	wait_us(1);
	WR = 1;
	CS = 1;
	wait_us(1);
}

uint8_t YMF262::read_status()
{
	uint8_t status = 0;
	DataBus.input();
	A0 = 0;
	A1 = 0;
	WR = 1;
	RD = 0;
	
	CS = 0;
	wait_us(1);
	status = DataBus.read();
	CS = 1;
	wait_us(1);
	return status;
}

void YMF262::rst_high()
{
	IC.input();
}

void YMF262::rst_low()
{
	IC.output();
	IC = 0;
}
