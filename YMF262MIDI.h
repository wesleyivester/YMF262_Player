#ifndef __YMF262MIDI_H__
#define __YMF262MIDI_H__

#include "YMF262/YMF262.h"
#include <cstdint>
#include "uLCD_4DGL.h"
#include <string>

class YMF262MIDI
{	
public:
	YMF262MIDI(YMF262* ymf262, uLCD_4DGL *lcd);
	~YMF262MIDI();
	void init();
	void noteOn(uint8_t channel, uint8_t note);
	void noteOff(uint8_t channel, uint8_t note);
	void downPress();
	void upPress();
	void actionPress();
	void redraw();
	
private:
	YMF262* ymf;
	uLCD_4DGL *lcd;
	uint8_t chan_count;
	uint8_t chan[18];
	static const uint8_t opOffset[18];
	static const uint8_t op1Id[9];
	static const uint8_t op2Id[9];
	static const uint16_t fnum[12];
	
	struct OpSettings
	{
		bool tremolo;
		bool vibrato;
		bool sustain;
		bool ksr;
		uint8_t freqMult;
		uint8_t attenuation;
		uint8_t a;
		uint8_t d;
		uint8_t s;
		uint8_t r;
		uint8_t waveform;
	};
	
	struct ChanSettings
	{
		bool out_left;
		bool out_right;
		uint8_t feedback;
	};
	
	void arraysToSettings(uint8_t chanArrSettings[], uint8_t opArrSettings[], ChanSettings &chs, OpSettings ops[]);
	void setChannelSettings(ChanSettings chs, OpSettings ops[]);
	
	int ctrl_selectedEntry;
	int ctrl_selectedEntryOld;
	bool ctrl_editingEntry;
	const int ctrl_count = 2 * 11 + 3;
	uint8_t ctrl_chanSettings[3];
	uint8_t ctrl_OpSettings[22];
	static const uint8_t ctrl_OpsettingsMax[11];
	static const uint8_t ctrl_chanSettingsMax[3];
	
	static const char* ctrl_text_chan[];
	static const char* ctrl_text_op[];
	
	const char* getControlText(int i);
	void displayControls(bool redrawAll);
};

#endif