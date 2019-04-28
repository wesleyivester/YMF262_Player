#ifndef __DIR_BROWSER_H__
#define __DIR_BROWSER_H__

#include <string>
#include <vector>
#include "uLCD_4DGL.h"
#include <cstdint>

class DirBrowser
{
public:
	DirBrowser(const std::string &root, uLCD_4DGL *lcd);
	~DirBrowser();
	
	void selectNext();
	void selectPrev();
	bool enterDir();
	void getSelectedFileData(uint8_t* &data, size_t &dataSize);
	
	bool selectionIsFile();
	bool selectionIsDir();
	void redraw();
	
private:
	std::string _root;
	uLCD_4DGL *_lcd;
	
	void displayDir(bool redrawAll);
	std::string currentPath;
	void updateCurrentPathStr();
	void loadDir();
	
	struct DirEntry
	{
		bool isDir;
		std::string name; 
	};
	
	std::vector<std::string> path;
	std::vector<DirEntry> dirEntries;
	int selectedDirEntry;
	int oldSelectedDirEntry;

};

#endif