#include "YMF262MIDI.h"

const uint8_t YMF262MIDI::opOffset[18] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15};
const uint8_t YMF262MIDI::op1Id[9] = {0, 1, 2, 6, 7,  8,  12, 13, 14};
const uint8_t YMF262MIDI::op2Id[9] = {3, 4, 5, 9, 10, 11, 15, 16, 17};
const uint16_t YMF262MIDI::fnum[12] = {517,547,580,615,651,690,731,774,820,869,921,975};

const char* YMF262MIDI::ctrl_text_op[] = {
	"Tremolo",
	"Vibrato",
	"Sustain",
	"KSR",
	"Freq Mult",
	"Quietness",
	"Attack",
	"Decay",
	"Sustain",
	"Release",
	"Waveform"
};
const char* YMF262MIDI::ctrl_text_chan[] = {
	"Out Left",
	"Out Right",
	"Feedback",
};

const uint8_t YMF262MIDI::ctrl_OpsettingsMax[11] = {1,1,1,1,15,63,15,15,15,15,7};
const uint8_t YMF262MIDI::ctrl_chanSettingsMax[3] = {1,1,7};

YMF262MIDI::YMF262MIDI(YMF262* ymf262, uLCD_4DGL *lcd):
ctrl_chanSettings{1,1,0}, ctrl_OpSettings{1,1,1,0,1,0,3,5,8,4,0, 1,1,1,0,3,0,8,8,8,4,6}
{
	ymf = ymf262;
	this->lcd = lcd;
	ctrl_selectedEntry = 0;
	ctrl_selectedEntryOld = 0;
	ctrl_editingEntry = false;
}

YMF262MIDI::~YMF262MIDI()
{
}

void YMF262MIDI::redraw()
{
	displayControls(true);
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
	
	ChanSettings chs;
	chs.out_left = true;
	chs.out_right = true;
	chs.feedback = 0;
	
	//arraysToSettings(ctrl_chanSettings, ctrl_OpSettings, chs, ops);
	setChannelSettings(chs, ops);
	
	chan_count = 18;
	for(uint8_t i(0); i < chan_count; ++i)
	{
		uint8_t arr = (i / 9);
		uint8_t chn = (i % 9);
		ymf->write_reg(arr, 0xB0 + chn, 0);	// note off

		chan[i] = 0;
	}
}

void YMF262MIDI::arraysToSettings(uint8_t chanArrSettings[], uint8_t opArrSettings[], ChanSettings &chs, OpSettings ops[])
{
	chs.out_left = chanArrSettings[0] != 0;
	chs.out_right = chanArrSettings[1] != 0;
	chs.feedback = chanArrSettings[2];
	
	for(int i(0); i < 2; ++i)
	{
		ops[i].tremolo = opArrSettings[11*i+0] != 0;
		ops[i].vibrato = opArrSettings[11*i+1] != 0;
		ops[i].sustain = opArrSettings[11*i+2] != 0;
		ops[i].ksr = opArrSettings[11*i+3] != 0;
		ops[i].freqMult = opArrSettings[11*i+4];
		ops[i].attenuation = opArrSettings[11*i+5];
		ops[i].a = opArrSettings[11*i+6];
		ops[i].d = opArrSettings[11*i+7];
		ops[i].s = opArrSettings[11*i+8];
		ops[i].r = opArrSettings[11*i+9];
		ops[i].waveform = opArrSettings[11*i+10];
	}
}

void YMF262MIDI::setChannelSettings(ChanSettings chs, OpSettings ops[])
{
	chan_count = 18;
	for(uint8_t i(0); i < chan_count; ++i)
	{
		uint8_t arr = (i / 9);
		uint8_t chn = (i % 9);
		uint8_t opOff[2];
		opOff[0] = opOffset[op1Id[chn]];
		opOff[1] = opOffset[op2Id[chn]];
		
		for(int i(0); i < 2; ++i)
		{
			ymf->write_reg(arr, 0x20 + opOff[i], ops[i].freqMult | (ops[i].tremolo<<7) | (ops[i].vibrato<<6) | (ops[i].sustain<<5) | (ops[i].ksr<<4));
			ymf->write_reg(arr, 0x40 + opOff[i], ops[i].attenuation | 0);
			ymf->write_reg(arr, 0x60 + opOff[i], (ops[i].a << 4) | ops[i].d);
			ymf->write_reg(arr, 0x80 + opOff[i], (ops[i].s << 4) | ops[i].r);	
			ymf->write_reg(arr, 0xE0 + opOff[i], ops[i].waveform);
		}

		ymf->write_reg(arr, 0xC0 + chn, 0xF0 | (chs.feedback << 1));
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

void YMF262MIDI::upPress()
{
	if(!ctrl_editingEntry)
	{
		if(ctrl_selectedEntry > 0)
		{
			ctrl_selectedEntryOld = ctrl_selectedEntry;
			ctrl_selectedEntry--;
			displayControls(false);
		}
	}
	else
	{
		uint8_t *val;
		uint8_t max;
		if(ctrl_selectedEntry < 3)
		{
			val = &(ctrl_chanSettings[ctrl_selectedEntry]);
			max = ctrl_chanSettingsMax[ctrl_selectedEntry];
		}
		else
		{
			val = &(ctrl_OpSettings[ctrl_selectedEntry-3]);
			max = ctrl_OpsettingsMax[(ctrl_selectedEntry-3)%11];
		}
		if(*val < max)
		{
			(*val)++;
			ChanSettings chs;
			OpSettings ops[2];
			arraysToSettings(ctrl_chanSettings, ctrl_OpSettings, chs, ops);
			setChannelSettings(chs, ops);
			displayControls(false);
		}
	}
}

void YMF262MIDI::downPress()
{
	if(!ctrl_editingEntry)
	{
		if(ctrl_selectedEntry < ctrl_count - 1)
		{
			ctrl_selectedEntryOld = ctrl_selectedEntry;
			ctrl_selectedEntry++;
			displayControls(false);
		}
	}
	else
	{
		uint8_t *val;
		if(ctrl_selectedEntry < 3)
			val = &(ctrl_chanSettings[ctrl_selectedEntry]);
		else
			val = &(ctrl_OpSettings[ctrl_selectedEntry-3]);
		if(*val > 0)
		{
			(*val)--;
			ChanSettings chs;
			OpSettings ops[2];
			arraysToSettings(ctrl_chanSettings, ctrl_OpSettings, chs, ops);
			setChannelSettings(chs, ops);
			displayControls(false);
		}
	}
}

void YMF262MIDI::actionPress()
{
	if(!ctrl_editingEntry)
	{
		ctrl_editingEntry = true;
	}
	else
	{
		ctrl_editingEntry = false;
	}
	ctrl_selectedEntryOld = ctrl_selectedEntry;
	displayControls(false);
}

const char* YMF262MIDI::getControlText(int i)
{
	static char buff[40];
	if(i < 3)
	{
		snprintf(buff, 40, "%-14s: %2d", ctrl_text_chan[i], ctrl_chanSettings[i]);
	}
	else
	{
		int op = (i-3)/11;
		int ind = (i-3)%11;
		snprintf(buff, 40, "op%1d %-10s: %2d", op+1, ctrl_text_op[ind], ctrl_OpSettings[i-3]);
	}
	return buff;
}

void YMF262MIDI::displayControls(bool redrawAll)
{
	int16_t display_min = ctrl_selectedEntry & 0xFFF0;
	int16_t display_max = display_min + 15;
	if(display_max >= ctrl_count)
		display_max = ctrl_count - 1;
	
	int16_t startOffset = 0;
	int16_t endOffset = 0;
	if(!redrawAll)
	{
		if((ctrl_selectedEntryOld & 0xFFF0) == display_min)
		{
			if(ctrl_selectedEntryOld < ctrl_selectedEntry)
			{
				startOffset = ctrl_selectedEntryOld - display_min;
				endOffset = display_max - ctrl_selectedEntry;
			}
			else
			{
				startOffset = ctrl_selectedEntry - display_min;
				endOffset = display_max - ctrl_selectedEntryOld;
			}
		}
	}
	
	lcd->color(0x00ff08);
	for(int i(display_min + startOffset); i <= display_max - endOffset; ++i)
	{
		lcd->locate(0, i - display_min);
		if(i == ctrl_selectedEntry)
		{
			lcd->color(0x004408);
			if(ctrl_editingEntry)
				lcd->textbackground_color(0xff7700);
			else
				lcd->textbackground_color(0xFFFFFF);
		}
		
		lcd->putstr(getControlText(i), 18);
		
		if(i == ctrl_selectedEntry)
		{
			lcd->textbackground_color(0x000000);
			lcd->color(0x00ff08);
		}
	}
	if(display_max - display_min < 15)
	{
		int lines_height = (display_max - display_min + 1) * 8;
		lcd->filled_rectangle(0, lines_height, 128, 128, 0x000000);
	}
}
	