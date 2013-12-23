#ifndef BLOCK_H
#define BLOCK_H
#include <string>
#include <bitset>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include "help.h"
using namespace std;

/******************************
class BlockService
------------------
Implements the last (bottom) layer that corresponds to allocation,
deallocation, writing, reading of blocks from the virtual disk
(I've created some helper functions that disk may not have - e.g. AllocateiNode)
*******************************/

class BlockService
{
private:
	char* device; //name of system file
	unsigned int blockSize; //block size
	unsigned int inodesNum; //number of inodes
	unsigned int inodesBMSize; //inodes bit map size
	unsigned int blockBMSize; //blocks bit map size
	unsigned int dirTableSize; //size of directory table
	//the 3 data structs saved in main memory
	bitset<8192> inodesBM; //inodes bit map
	bitset<122880> blockBM; //blocks bit map
	dirEntry* dirTable; //directory table
public:
	BlockService() { }
	BlockService(const char* file)
	{
		device = new char[strlen(file)];
		strcpy(device,file);
	}
	
	unsigned int MaxFiles()
	{
		return inodesNum;
	}
	
	unsigned int DirSize()
	{
		return dirTableSize;
	}
	
	unsigned int BlockSize()
	{
		return blockSize;
	}
	
	dirEntry* DirTable()
	{
		return dirTable;
	}
	
	void Initialize() //Initialization
	{
		int fp;
		fp = open(device, O_RDONLY);
		lseek(fp,0,SEEK_SET); //seek to superblock
		// read constants/parameters
		read(fp,&inodesNum,4);
		read(fp,&blockSize,4);
		read(fp,&inodesBMSize,4);
		read(fp,&blockBMSize,4);
		read(fp,&dirTableSize,4);
		inodesBMSize *= blockSize;
		blockBMSize *= blockSize;
		dirTableSize *= blockSize;
		lseek(fp,blockSize,SEEK_SET); //seek to inodes bit map start block
		// read and save bit map inodes
		char* data = new char[inodesBMSize];
		read(fp,data,inodesBMSize);
		for(unsigned int i=0;i<inodesBMSize;i++)
		{
			for(unsigned int j=0;j<8;j++)
			{
				inodesBM.set(i*8+j,(data[i]>>j)&0x01);
			}
		}
		lseek(fp,2*blockSize,SEEK_SET); //seek to blocks bit map start block
		delete[] data;
		data = new char[blockBMSize];
		// read and save bit map blocks
		read(fp,data,blockBMSize);
		for(unsigned int i=0;i<blockBMSize;i++)
		{
			for(unsigned int j=0;j<8;j++)
			{
				blockBM.set(i*8+j,(data[i]>>j)&0x01);
			}
		}
		delete[] data;
		if(dirTable)
			delete[] dirTable;
		dirTable = new dirEntry[dirTableSize/(sizeof(dirEntry))];
		lseek(fp,529*blockSize,SEEK_SET); //seek to directory table's start block
		// read of directory table
		read(fp,dirTable,dirTableSize);
		close(fp);
	}
	
	void WrapUp() //ÁðïèÞêåõóç áëëáãìÝíùí ôéìþí ðßóù óôï system file (üôáí êÜíïõìå unmount)
	{
		int fp;
		fp = open(device, O_WRONLY);
		lseek(fp,blockSize,SEEK_SET); //seek to inodes bit map start block
		// áðïèÞêåõóç óôï system file ôïõ inodes bit map
		for(unsigned int i=0;i<inodesBMSize;i++)
		{
			char x = 0;
			for(unsigned int j=0;j<8;j++)
			{
				x |= (inodesBM[i*8+j]<<j);
			}
			write(fp,&x,1);
		}
		lseek(fp,2*blockSize,SEEK_SET); //seek to blocks bit map start block
		// áðïèÞêåõóç óôï system file ôïõ blocks bit map
		for(unsigned int i=0;i<blockBMSize;i++)
		{
			char x = 0;
			for(unsigned int j=0;j<8;j++)
			{
				x |= (blockBM[i*8+j]<<j);
			}
			write(fp,&x,1);
		}
		lseek(fp,529*blockSize,SEEK_SET); //seek to directory table's start block
		// áðïèÞêåõóç óôï system file ôïõ directory table
		write(fp,dirTable,dirTableSize);
		close(fp);
	}
	
	int ReadBlock(unsigned int pos, void* data) // öÝñíåé áðü ôï äßóêï Ýíá block (ìáò åíäéáöÝñïõí ìüíï ôá blocks ãéá ôá áñ÷åßá ïðüôå ìðáßíåé Ýíá offset óôçí pos)
	{
		int fp,t;
		fp = open(device, O_RDONLY);
		if(fp<0)
			return -1;
		t = lseek(fp,(657+pos-1)*blockSize,SEEK_SET); // seek óôç èÝóç ôïõ block ðïõ æçôåßôáé
		if(t<0)
		{
			close(fp);
			return -1;
		}
		t = read(fp,data,blockSize); // áíÜãíùóç äåäïìÝíùí
		if(t<0)
		{
			close(fp);
			return -1;
		}
		close(fp);
		return 0;
	}
	
	int WriteBlock(unsigned int pos, void* data) // ãñÜöåé óôï äßóêï Ýíá block (ìáò åíäéáöÝñïõí ìüíï ôá blocks ãéá ôá áñ÷åßá ïðüôå ìðáßíåé Ýíá offset óôçí pos)
	{
		if(data)
		{
			int fp,t;
			fp = open(device, O_WRONLY);
			if(fp<0)
				return -1;
			t = lseek(fp,(657+pos-1)*blockSize,SEEK_SET); // seek óôç èÝóç ôïõ block ðïõ æçôåßôáé
			if(t<0)
			{
				close(fp);
				return -1;
			}
			t = write(fp,data,blockSize); // åããñáöÞ äåäïìÝíùí
			if(t<0)
			{
				close(fp);
				return -1;
			}
			close(fp);
			return 0;
		}
		return -1;
	}
	
	int AllocateBlock(unsigned int& pos) // äÝóìåõóç åíüò block (ìáò åíäéáöÝñïõí ìüíï ôá blocks ãéá ôá áñ÷åßá ïðüôå ìðáßíåé Ýíá offset óôçí pos)
	{
		for(unsigned int i=0;i<blockBM.size();i++)
		{
			if(blockBM[i] == 0) // äåóìåýïõìå ôï ðñþôï åëåýèåñï
			{
				blockBM[i] = 1;
				pos = i+1;
				return 0;
			}
		}
		return -1;
	}
	
	int FreeBlock(unsigned int pos) // áðïäÝóìåõóç åíüò block (ìáò åíäéáöÝñïõí ìüíï ôá blocks ãéá ôá áñ÷åßá ïðüôå ìðáßíåé Ýíá offset óôçí pos)
	{
		if(pos<=blockBM.size()&&pos>0) // Ýëåã÷ïò ãéá valid pos
		{
			blockBM[pos-1] = 0;
			return 0;
		}
		return -1;
	}
	
	int getiNode(unsigned int index, iNode* inode) // öÝñíåé áðü ôï äßóêï Ýíá iNode
	{
		int fp,t;
		if(inodesBM[index-1]==0)
			return -1;
		fp = open(device, O_RDONLY);
		if(fp<0)
			return -1;
		t = lseek(fp,17*blockSize+(index-1)*sizeof(iNode),SEEK_SET); // seek óôç èÝóç ôïõ iNode ðïõ æçôåßôáé
		if(t<0)
		{
			close(fp);
			return -1;
		}
		t = read(fp,inode,sizeof(iNode)); // áíÜãíùóç äåäïìÝíùí
		if(t<0)
		{
			close(fp);
			return -1;
		}
		close(fp);
		return 0;
	}
	
	int putiNode(unsigned int index, iNode* inode) // áðïèçêåýåé óôï äßóêï Ýíá iNode
	{
		int fp,t;
		if(inodesBM[index-1]==0)
			return -1;
		fp = open(device, O_WRONLY);
		if(fp<0)
			return -1;
		t = lseek(fp,17*blockSize+(index-1)*sizeof(iNode),SEEK_SET); // seek óôç èÝóç ôïõ iNode ðïõ æçôåßôáé
		if(t<0)
		{
			close(fp);
			return -1;
		}
		t = write(fp,inode,sizeof(iNode)); // åããñáöÞ äåäïìÝíùí
		if(t<0)
		{
			close(fp);
			return -1;
		}
		close(fp);
		return 0;
	}
	
	int AllocateiNode() // äÝóìåõóç åíüò iNode
	{
		int i;
		for(i=0;i<inodesNum;i++)
		{
			if(inodesBM[i] == 0) // äåóìåýïõìå ôï ðñþôï åëåýèåñï
			{
				break;
			}
		}
		if(i>= inodesNum)
			return -1;
		inodesBM[i] = 1;
		// áñ÷éêïðïßçóç ôïõ iNode
		iNode* inode = new iNode;
		inode->size=0;
		for(int j=0;j<15;j++) inode->indices[j] = 0;
		putiNode(i+1, inode);
		return i+1;
	}
	
	int DeAllocateiNode(unsigned int index) // áðïäÝóìåõóç åíüò iNode
	{
		if(index<=0||index>inodesNum) // Ýëåã÷ïò ãéá valid index
			return -1;
		inodesBM[index-1] = 0;
		return 0;
	}
};

BlockService* block = new BlockService(system_file.c_str()); // global variable

#endif
