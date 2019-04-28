#include "YMF262MIDI.h"

const uint8_t YMF262MIDI::opOffset[18] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15};
const uint8_t YMF262MIDI::op1Id[9] = {0, 1, 2, 6, 7,  8,  12, 13, 14};
const uint8_t YMF262MIDI::op2Id[9] = {3, 4, 5, 9, 10, 11, 15, 16, 17};
const uint16_t YMF262MIDI::fnum[12] = {517,547,580,615,651,690,731,774,820,869,921,975}; 

YMF262MIDI::YMF262MIDI(YMF262* ymf262, uLCD_4DGL *lcd)
{
	ymf = ymf262;
	this->lcd = lcd;
}

YMF262MIDI::~YMF262MIDI()
{
}

void YMF262MIDI::redraw()
{
	lcd->cls();
}

void YMF262MIDI::init()
{
	// Clear all registers
    for (uint16_t reg = 0x20; reg < 0xF6; reg++) {

        // Ensure envelopes decay all the way
        uint8_t val = (reg >= 0x60 && reg < 0xA0) ? 0xFF : 0;

        ymf->write_reg(0, reg, val);
        ymf->write_reg(1, reg, val);
    }
	
    // Configure OPL3
    ymf->write_reg(0, 8, 0);		// No split point
    ymf->write_reg(0, 0xbd, 0);		// No drums, etc.
    ymf->write_reg(1, 0x04, 0);		// Everything 2-op by default
    ymf->write_reg(1, 0x05, 1);		// OPL3 mode on
	
	chan_count = 18;
	for(uint8_t i(0); i < chan_count; ++i)
	{
		uint8_t arr = (i / 9);
		uint8_t chn = (i % 9);
		uint8_t opOff[2];
		opOff[0] = opOffset[op1Id[chn]];
		opOff[1] = opOffset[op2Id[chn]];
		
		OpSettings ops[2];

		ops[0].tremolo = true;
		ops[0].vibrato = true;
		ops[0].sustain = true;
		ops[0].ksr = false;
		ops[0].freqMult = 1;
		ops[0].attenuation = 0;
		ops[0].a = 3;
		ops[0].d = 5;
		ops[0].s = 8;
		ops[0].r = 4;
		ops[0].waveform = 0;
		
		ops[1].tremolo = true;
		ops[1].vibrato = true;
		ops[1].sustain = true;
		ops[1].ksr = false;
		ops[1].freqMult = 3;
		ops[1].attenuation = 0;
		ops[1].a = 8;
		ops[1].d = 8;
		ops[1].s = 8;
		ops[1].r = 4;
		ops[1].waveform = 6;
		
		for(int i(0); i < 2; ++i)
		{
			ymf->write_reg(arr, 0x20 + opOff[i], ops[i].freqMult | (ops[i].tremolo<<7) | (ops[i].vibrato<<6) | (ops[i].sustain<<5) | (ops[i].ksr<<4));
			ymf->write_reg(arr, 0x40 + opOff[i], ops[i].attenuation | 0);
			ymf->write_reg(arr, 0x60 + opOff[i], (ops[i].a << 4) | ops[i].d);
			ymf->write_reg(arr, 0x80 + opOff[i], (ops[i].s << 4) | ops[i].r);	
			ymf->write_reg(arr, 0xE0 + opOff[i], ops[i].waveform);
		}
		
		ChanSettings chs;
		chs.out_left = true;
		chs.out_right = true;
		chs.feedback = 0;
		
		ymf->write_reg(arr, 0xB0 + chn, 0);	// note off
		ymf->write_reg(arr, 0xC0 + chn, 0xF0 | (chs.feedback << 1));
		
		chan[i] = 0;
	}
}

void YMF262MIDI::noteOn(uint8_t channel, uint8_t note)
{
	for(int i(0); i < chan_count; ++i)
	{
		if(chan[i] == 0)
		{
			uint8_t arr = (i / 9);
			uint8_t chn = (i % 9);
			
			uint8_t b = (note - 19) / 12;
			uint16_t f = fnum[(note - 19) % 12];
			
			ymf->write_reg(arr, 0xA0 + chn, f & 0xFF);
			ymf->write_reg(arr, 0xB0 + chn, 0x20 | (b<<2) | (f>>8));
			chan[i] = note;
			break;
		}
	}
}

void YMF262MIDI::noteOff(uint8_t channel, uint8_t note)
{
	for(int i(0); i < chan_count; ++i)
	{
		if(chan[i] == note)
		{
			uint8_t arr = (i / 9);
			uint8_t chn = (i % 9);
			ymf->write_reg(arr, 0xB0 + chn, 0);
			chan[i] = 0;
			break;
		}
	}
}
	