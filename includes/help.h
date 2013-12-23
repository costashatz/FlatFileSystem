#ifndef HELP_H
#define HELP_H

#include <cstring>
using namespace std;

/******************************
help.h
------------------
Helper structs and defines
*******************************/

#define min(x,y) x<y?x:y

struct dirEntry // record in directory table
{
	char filename[12];
	unsigned int index;
};

struct iNode // iNode
{
	int size;
	unsigned int indices[15];
};

struct openFile // open file
{
	char filename[12];
	unsigned int index;
	unsigned int ufid;
	iNode inode;
};

string system_file = "system.txt"; //system file name

#endif
