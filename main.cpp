#include "mbed.h"
#include "SDFileSystem.h"
#include "uLCD_4DGL.h"
#include "YMF262/YMF262.h"
#include "rad/player20.cpp"
#include "rad/validate20.cpp"
#include <string>
#include <vector>
#include <stdint.h>
#include "FastPWM.h"
#include "DirBrowser.h"

Serial pc(USBTX, USBRX);

void rad_write_reg_cb(void* userp, uint16_t reg, uint8_t val)
{
	YMF262* ymf262 = (YMF262*)userp;
	uint8_t array = (reg & 0x100) >> 8;
	ymf262->write_reg(array, reg & 0xFF, val);
	//pc.printf("%d, %d\r\n", reg, val);
}

int main() {	
	uLCD_4DGL lcd(p28, p27, p30);
	lcd.baudrate(3000000);
	lcd.background_color(0x000000);
	lcd.cls();
	
	SDFileSystem sd(p5, p6, p7, p8, "sd");
	
	DirBrowser dir("/sd", &lcd);
	
	DigitalIn up(p17, PullUp);
	DigitalIn down(p19, PullUp);
	DigitalIn action(p20, PullUp);
	bool up_press(false), down_press(false), action_press(false);
	
	RADPlayer radplayer;
	Ticker updateTicker;
	bool playing = false;
	
	pc.printf("Start\r\n");
	wait_ms(1000);
	
	FastPWM clock(p26);
	clock.period_ticks(6);
	clock.write(.5);
	
	//PinName dataBus[8] =  {p9, p10, p11, p12, p13, p14, p15, p16};
	PinName dataBus[8] =  {p16, p15, p14, p13, p12, p11, p10, p9};
	YMF262 ymf262(p21, p22, p23, p24, p25, p29, dataBus);
	
	//ymf262.write_reg(0, 2, 5);
	ymf262.write_reg(0, 3, 5);
	
    while(1)
	{
		uint8_t status = ymf262.read_status();
		if(status != 0)
			pc.printf("%d\r\n", status);
		wait_ms(5);
		if(!action)
		{
			if(!action_press)
			{
				if(playing)
				{
					updateTicker.detach();
					radplayer.Stop();
					playing = false;
					lcd.locate(0, 0);
					lcd.printf("Playing Stopped\n");
				}
				else
				{
					if(!dir.enterDir())
					{
						uint8_t* filedata;
						size_t filedataSize;
						dir.getSelectedFileData(filedata, filedataSize);

						const char *err = RADValidate(filedata, filedataSize);
						if(err)
						{
							lcd.locate(0, 0);
							lcd.printf(err);
						}
						else
						{
							lcd.locate(0, 0);
							lcd.printf("Playing Started\n");
							radplayer.Init(filedata, rad_write_reg_cb, (void*)&ymf262);
							float updateInterval = 1.0 / (float)radplayer.GetHertz();
							updateTicker.attach([&radplayer](){
								radplayer.Update();
							}, updateInterval);							
							playing = true;
						}
					}
				}
			}
			action_press = true;
		}
		else
		{
			action_press = false;
		}
		
		if(!up)
		{
			if(!up_press)
			{
				dir.selectPrev();
			}
			up_press = true;
		}
		else
		{
			up_press = false;
		}
		
		if(!down)
		{
			if(!down_press)
			{
				dir.selectNext();
			}
			down_press = true;
		}
		else
		{
			down_press = false;
		}
    }
}
