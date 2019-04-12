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

Serial pc(USBTX, USBRX);

void rad_write_reg_cb(void* userp, uint16_t reg, uint8_t val)
{
	YMF262* ymf262 = (YMF262*)userp;
	uint8_t array = (reg & 0x100) >> 8;
	ymf262->write_reg(array, reg & 0xFF, val);
	//pc.printf("%d, %d\r\n", reg, val);
}

struct DirEntry
{
	bool isDir;
	std::string name; 
};

void displayDir(uLCD_4DGL &lcd, const std::vector<DirEntry> &dirEntries, int currEntry)
{
	char buff[40];
	//lcd.cls();
	int display_min = 0;
	if(currEntry - 15 > 0)
		display_min = currEntry - 15;
	int display_max = display_min + 15;
	if(display_max >= dirEntries.size())
		display_max = dirEntries.size() - 1;
	int color = 0;
	for(int i(display_min); i <= display_max; ++i)
	{
		snprintf(buff, 40, "%-18s", dirEntries[i].name.c_str());
		lcd.locate(0, i - display_min);
		if(dirEntries[i].isDir)
		{
			if(color != 0xff7700)
				lcd.color(0xff7700);
		}
		else if(color != 0x00ff08)
		{
			lcd.color(0x00ff08);
		}
		if(i != currEntry)
			lcd.putstr(buff, 18);
		else
		{
			lcd.textbackground_color(0xFFFFFF);
			lcd.putstr(buff, 18);
			lcd.textbackground_color(0x000000);
		}
	}
	if(display_max - display_min < 15)
	{
		int lines_height = (display_max - display_min + 1) * 8;
		lcd.filled_rectangle(0, lines_height, 128, 128, 0x000000);
	}
}

void loadDir(std::vector<DirEntry> &dirEntries, const std::string &path)
{
	dirEntries.clear();
	dirEntries.push_back({true, ".."});
	DirHandle* dir = opendir(path.c_str());
	if(dir)
	{
		dirent* ent;
		while((ent = readdir(dir)))
		{
			dirEntries.resize(dirEntries.size() + 1);
			dirEntries.back().name = ent->d_name;
			DirHandle* testdir = opendir((path + '/' + dirEntries.back().name).c_str());
			if(testdir)
			{
				dirEntries.back().isDir = true;
				closedir(testdir);
			}
			else
				dirEntries.back().isDir = false;
		}
		closedir(dir);
	}
}

void getPathString(const std::vector<std::string> &path, std::string &pathstr)
{
	pathstr.clear();
	for(int i(0); i < path.size(); ++i)
	{
		pathstr += '/' + path[i];
	}
}

void getFileData(const std::string &path, uint8_t* &data, size_t &dataSize)
{
	FILE *fp = fopen(path.c_str(), "rb");
	
	fseek(fp, 0, SEEK_END);
	long fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	if(fsize > 32768)	// can't fit this file in ram
		dataSize = 0;
	else
	{
		data = (uint8_t*)0x2007C000;
		dataSize = fsize;
		fread(data, 1, dataSize, fp);
	}
	fclose(fp); 
}

int main() {	
	uLCD_4DGL lcd(p28, p27, p30);
	lcd.baudrate(3000000);
	lcd.background_color(0x000000);
	lcd.cls();
	
	SDFileSystem sd(p5, p6, p7, p8, "sd");
	
	DigitalIn up(p17, PullUp);
	DigitalIn down(p19, PullUp);
	DigitalIn action(p20, PullUp);
	bool up_press(false), down_press(false), action_press(false);
	
	std::vector<std::string> path;
	path.push_back("sd");
	std::string pathString;
	std::vector<DirEntry> dirEntries;
	int selectedEntry = 0;
	
	getPathString(path, pathString);
	loadDir(dirEntries, pathString);
	displayDir(lcd, dirEntries, selectedEntry);
	
	RADPlayer radplayer;
	Ticker updateTicker;
	bool playing = false;
	
	uint8_t* filedata;
	size_t filedataSize;
	
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
					if(dirEntries[selectedEntry].isDir)
					{
						if(dirEntries[selectedEntry].name == "..")
						{
							if (path.size() > 1)
								path.pop_back();
						}
						else
							path.push_back(dirEntries[selectedEntry].name);
						selectedEntry = 0;
						getPathString(path, pathString);
						loadDir(dirEntries, pathString);
						displayDir(lcd, dirEntries, selectedEntry);
					}
					else
					{
						getFileData(pathString + '/' + dirEntries[selectedEntry].name, filedata, filedataSize);

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
				if(selectedEntry > 0)
				{
					selectedEntry--;
					displayDir(lcd, dirEntries, selectedEntry);
				}
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
				if(selectedEntry < dirEntries.size() - 1)
				{
					selectedEntry++;
					displayDir(lcd, dirEntries, selectedEntry);
				}
			}
			down_press = true;
		}
		else
		{
			down_press = false;
		}
    }
}
