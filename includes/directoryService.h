#ifndef DIRECTORY_H
#define DIRECTORY_H
#include "blockService.h"
#include <vector>
using namespace std;

/******************************
class DirectoryService
------------------
Implements the layer that corresponds to file/catalog management, like create, remove, open
*******************************/

class DirectoryService
{
private:
	vector<openFile> openFiles; // keeping track of open files
public:
	DirectoryService() { }
	
	int files(char** filenames) //get list of files (returns num of files)
	{
		unsigned int i = 0;
		dirEntry* table = block->DirTable(); // pointer to directory table
		while(table[i].index!=0)
		{
			filenames[i] = new char[12];
			strcpy(filenames[i],table[i].filename);
			i++;
		}
		return i;
	}
	
	vector<openFile> OpenFiles()
	{
		return openFiles;
	}
	
	int changeSize(int pos, unsigned int size)
	{
		if(pos<0||pos>=openFiles.size())
			return -1;
		openFiles[pos].inode.size = size;
		return 0;
	}
	
	int changeIndex(int pos, int index, int p)
	{
		if(pos<0||pos>=openFiles.size())
			return -1;
		openFiles[pos].inode.indices[index] = p;
		return 0;
	}
	
	int closeOpen(int pos) //close file
	{
		if(pos>=0 && pos<openFiles.size())
		{
			int t;
			t = block->putiNode(openFiles[pos].index,&openFiles[pos].inode);
			if(t<0)
				return -1;
			openFiles.erase(openFiles.begin()+pos);
			return 0;
		}
		return -1;
	}
	
	int create(const char* filename) // create new file
	{
		dirEntry* table = block->DirTable();
		unsigned int i = 0;
		unsigned int s  = block->DirSize()/sizeof(dirEntry);
		while(i<s&&table[i].index!=0&&strcmp(table[i].filename,filename)!=0) i++;
		if(i<s&&table[i].index!=0) // file already exists
		{
			//clear data
			iNode* inode = new iNode;
			inode->size = 0;
			for(int j=0;j<15;j++) inode->indices[j] = 0;
			block->putiNode(table[i].index, inode);
			return 0;
		}
		else if(i<s) // file doesn't exist
		{
			int j;
			j = block->AllocateiNode(); // allocate new iNode
			if(j<0)
				return -1;
			strcpy(table[i].filename,filename);
			table[i].index = j; // update directory table
			return 0;
		}
		return -1;
	}
	
	int remove(const char* filename) // remove file
	{
		dirEntry* table = block->DirTable();
		unsigned int i = 0;
		unsigned int s  = block->DirSize()/sizeof(dirEntry);
		while(i<s&&table[i].index!=0&&strcmp(table[i].filename,filename)!=0) i++;
		if(i<s&&table[i].index!=0) // file exists
		{
			int t;
			t = block->DeAllocateiNode(table[i].index); // deallocate iNode
			if(t<0)
				return -1;
			while(i<(s-2)&&table[i].index!=0) // shift data in directory table (not good but for the assignment is good enough)
			{
				table[i].index = table[i+1].index;
				strcpy(table[i].filename,table[i+1].filename);
				i++;
			}
			return 0;
		}
		return -1;
	}
	
	int open(const char* filename) // open file - returns ufid
	{
		dirEntry* table = block->DirTable();
		unsigned int i = 0;
		unsigned int s  = block->DirSize()/sizeof(dirEntry);
		int ufid = -1;
		while(i<s&&table[i].index!=0&&strcmp(table[i].filename,filename)!=0) i++;
		if(i<s&&table[i].index!=0) // file exists
		{
			ufid = openFiles.size()+1; //generate unique ufid
			openFile f;
			strcpy(f.filename,filename);
			f.ufid = ufid;
			f.index = table[i].index;
			block->getiNode(table[i].index, &f.inode);
			openFiles.push_back(f);
			return ufid;
		}
		return -1;
	}
};

DirectoryService* directory = new DirectoryService; // global variable

#endif
