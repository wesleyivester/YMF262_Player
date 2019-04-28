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
#include "USBMIDI.h"
#include "YMF262MIDI.h"

Serial pc(USBTX, USBRX);
DigitalOut leds[4] = { DigitalOut(LED1), DigitalOut(LED2), DigitalOut(LED3), DigitalOut(LED4) };

PinName dataBus[8] = {p16, p15, p14, p13, p12, p11, p10, p9};
YMF262 ymf262(p21, p22, p23, p24, p25, p29, dataBus);

uLCD_4DGL lcd(p28, p27, p30);

YMF262MIDI ymfmidi(&ymf262, &lcd);

void rad_write_reg_cb(void* userp, uint16_t reg, uint8_t val)
{
	YMF262* ymf262 = (YMF262*)userp;
	uint8_t array = (reg & 0x100) >> 8;
	ymf262->write_reg(array, reg & 0xFF, val);
	//pc.printf("%d, %d\r\n", reg, val);
}

void process_midi_msg(MIDIMessage msg)
{
	switch (msg.type())
	{
		case MIDIMessage::NoteOnType:
			ymfmidi.noteOn(msg.channel(), msg.key());
			leds[0] = 1;
			break;
		case MIDIMessage::NoteOffType:
			ymfmidi.noteOff(msg.channel(), msg.key());
			leds[0] = 0;
			break;
	}    
}

int main()
{
	lcd.baudrate(3000000);
	lcd.background_color(0x000000);
	lcd.cls();
	
	SDFileSystem sd(p5, p6, p7, p8, "sd");
	
	DirBrowser dir("/sd", &lcd);
	
	DigitalIn up(p18, PullUp);
	DigitalIn down(p19, PullUp);
	DigitalIn action(p20, PullUp);
	DigitalIn action2(p26, PullUp);
	bool up_press(false), down_press(false), action_press(false), action2_press(false);
	
	RADPlayer radplayer;
	Ticker updateTicker;
	bool playing = false;
	
	pc.printf("Start\r\n");
	
	//FastPWM clock(p26);
	//clock.period_ticks(6);
	//clock.period(.001);
	//clock.write(.5);
	
	bool midiMode = false;
	
	USBMIDI midi;
	
	ymfmidi.init();
	
    while(1)
	{		
		wait_ms(5);
		if(!action)
		{
			if(!action_press)
			{
				if(!midiMode)
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
				else
				{
					ymfmidi.actionPress();
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
				if(!midiMode)
					dir.selectPrev();
				else
					ymfmidi.upPress();
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
				if(!midiMode)
					dir.selectNext();
				else
					ymfmidi.downPress();
			}
			down_press = true;
		}
		else
		{
			down_press = false;
		}
		
		if(!action2)
		{
			if(!action2_press)
			{
				if(!midiMode)
				{
					if(playing)
					{
						updateTicker.detach();
						radplayer.Stop();
						playing = false;
					}
					
					midiMode = true;
					ymfmidi.init();
					midi.attach(process_midi_msg);
					ymfmidi.redraw();
				}
				else
				{
					midiMode = false;
					ymfmidi.init();
					midi.attach(NULL);
					dir.redraw();
				}
			}
			action2_press = true;
		}
		else
		{
			action2_press = false;
		}
    }
}
