#include "DirBrowser.h"

DirBrowser::DirBrowser(const std::string &root, uLCD_4DGL *lcd)
{
	_root = root;
	_lcd = lcd;
	selectedDirEntry = 0;
	oldSelectedDirEntry = 0;
	updateCurrentPathStr();
	loadDir();
	displayDir(true);
}

DirBrowser::~DirBrowser()
{
}

void DirBrowser::redraw()
{
	displayDir(true);
}

void DirBrowser::selectNext()
{
	if(selectedDirEntry < dirEntries.size() - 1)
	{
		oldSelectedDirEntry = selectedDirEntry;
		selectedDirEntry++;
		displayDir(false);
	}
}

void DirBrowser::selectPrev()
{
	if(selectedDirEntry > 0)
	{
		oldSelectedDirEntry = selectedDirEntry;
		selectedDirEntry--;
		displayDir(false);
	}
}

bool DirBrowser::enterDir()
{
	if(dirEntries[selectedDirEntry].isDir)
	{
		if(dirEntries[selectedDirEntry].name == "..")
		{
			if (!path.empty())
				path.pop_back();
		}
		else
			path.push_back(dirEntries[selectedDirEntry].name);
		selectedDirEntry = 0;
		oldSelectedDirEntry = 0;
		updateCurrentPathStr();
		loadDir();
		displayDir(true);
		return true;
	}
	return false;
}

void DirBrowser::loadDir()
{
	dirEntries.clear();
	dirEntries.push_back({true, ".."});
	DirHandle* dir = opendir(currentPath.c_str());
	if(dir)
	{
		dirent* ent;
		while((ent = readdir(dir)))
		{
			dirEntries.resize(dirEntries.size() + 1);
			dirEntries.back().name = ent->d_name;
			DirHandle* testdir = opendir((currentPath + '/' + dirEntries.back().name).c_str());
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

void DirBrowser::updateCurrentPathStr()
{
	currentPath = _root;
	for(int i(0); i < path.size(); ++i)
	{
		currentPath += '/' + path[i];
	}
}

void DirBrowser::getSelectedFileData(uint8_t* &data, size_t &dataSize)
{
	data = 0;
	dataSize = 0;
	if(selectionIsFile())
	{
		std::string path = currentPath + '/' + dirEntries[selectedDirEntry].name;
		
		FILE *fp = fopen(path.c_str(), "rb");
		
		fseek(fp, 0, SEEK_END);
		long fsize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		
		if(fsize <= 32768)	// largest size that fits in ram
		{
			data = (uint8_t*)0x2007C000;
			dataSize = fsize;
			fread(data, 1, dataSize, fp);
		}
		fclose(fp);
	}
}

bool DirBrowser::selectionIsFile()
{
	return !(dirEntries[selectedDirEntry].isDir);
}

bool DirBrowser::selectionIsDir()
{
	return dirEntries[selectedDirEntry].isDir;
}

void DirBrowser::displayDir(bool redrawAll)
{
	char buff[40];
	int16_t display_min = selectedDirEntry & 0xFFF0;
	int16_t display_max = display_min + 15;
	if(display_max >= dirEntries.size())
		display_max = dirEntries.size() - 1;
	int color = 0;
	
	int16_t startOffset = 0;
	int16_t endOffset = 0;
	if(!redrawAll)
	{
		if((oldSelectedDirEntry & 0xFFF0) == display_min)
		{
			if(oldSelectedDirEntry < selectedDirEntry)
			{
				startOffset = oldSelectedDirEntry - display_min;
				endOffset = display_max - selectedDirEntry;
			}
			else
			{
				startOffset = selectedDirEntry - display_min;
				endOffset = display_max - oldSelectedDirEntry;
			}
		}
	}
	
	for(int i(display_min + startOffset); i <= display_max - endOffset; ++i)
	{
		snprintf(buff, 40, "%-18s", dirEntries[i].name.c_str());
		_lcd->locate(0, i - display_min);
		if(dirEntries[i].isDir)
		{
			if(color != 0xff7700)
				_lcd->color(0xff7700);
		}
		else if(color != 0x00ff08)
		{
			_lcd->color(0x00ff08);
		}
		if(i != selectedDirEntry)
			_lcd->putstr(buff, 18);
		else
		{
			_lcd->textbackground_color(0xFFFFFF);
			_lcd->putstr(buff, 18);
			_lcd->textbackground_color(0x000000);
		}
	}
	if(display_max - display_min < 15)
	{
		int lines_height = (display_max - display_min + 1) * 8;
		_lcd->filled_rectangle(0, lines_height, 128, 128, 0x000000);
	}
}