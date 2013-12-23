#ifndef FILE_H
#define FILE_H
#include "directoryService.h"
#include <math.h>

/******************************
class FileService
------------------
Implements the layer that corresponds to file management
actions, like write, read and details (e.g. size)
*******************************/

class FileService
{
public:
	FileService() { }
	
	int close(int ufid) // close file with id ufid
	{
		vector<openFile> v = directory->OpenFiles(); // get from DirectoryService all open files
		int i;
		for(i=0;i<v.size();i++)
		{
			if(v[i].ufid == ufid) // find our file
				break;
		}
		if(i >= v.size()) // if not found in open files, error
			return -1;
		return directory->closeOpen(i); //close the open file
	}
	
	int file_size(int ufid) //get file size of file
	{
		vector<openFile> v = directory->OpenFiles(); // get from DirectoryService all open files
		int i;
		for(i=0;i<v.size();i++)
		{
			if(v[i].ufid == ufid) // find our file
				break;
		}
		if(i >= v.size()) // if not found in open files, error
			return -1;
		return v[i].inode.size; //return size of file
	}
	
	int read(int ufid, char* buf, int num, int pos) //read data from file with id ufid
	{
		if(num<0||pos<0) // check if params are valid
			return -1;
		vector<openFile> v = directory->OpenFiles(); // get from DirectoryService all open files
		int i;
		for(i=0;i<v.size();i++)
		{
			if(v[i].ufid == ufid) // find our file
				break;
		}
		if(i >= v.size()) // if not found in open files, error
			return -1;
		if(pos>=v[i].inode.size) //if reading out of file bounds, error
			return -1;
		unsigned int blSize = block->BlockSize();
		int index = pos/blSize; // which of the 15 indices we have to start
		int pp = pos%blSize; // offset in block
		int loop = (int)ceil((double)(num+pp)/blSize); // number of blocks needed
		unsigned int n = 0;
		char* data = new char[blSize];
		while(loop>0)
		{
			block->ReadBlock(v[i].inode.indices[index],data); // bring block to memory
			int size = min(blSize,num-n)-pp;
			if((index*blSize+pp+size)>v[i].inode.size)
				size  = v[i].inode.size - index*blSize-pp;
			memcpy(buf+n,data+pp,size); // copy to buf from memory
			n += size;
			pp = 0;
			index++;
			loop--;
		}
		return n;
	}
	
	int write(int ufid, char* buf, int num, int pos) //write data to file with id ufid
	{
		if(num<0||pos<0) // check if params are valid
			return -1;
		vector<openFile> v = directory->OpenFiles(); // get from DirectoryService all open files
		int i;
		for(i=0;i<v.size();i++)
		{
			if(v[i].ufid == ufid) // find our file
				break;
		}
		if(i >= v.size()) // if not found in open files, error
			return -1;
		if(pos>v[i].inode.size) //if writing out of file bounds, error
			return -1;
		unsigned int blSize = block->BlockSize();
		int index = pos/blSize; // which of the 15 indices we have to start
		int pp = pos%blSize; // offset in block
		int loop = (int)ceil(((double)(num+pp)/blSize)); // number of blocks needed
		unsigned int n = 0;
		char* data = new char[blSize];
		while(loop>0)
		{
			if(index>=15) // if we want more than 15 indices (max), stop
				break;
			int size = min(blSize,num-n);
			if(v[i].inode.indices[index] == 0) // if index with non allocated block
			{
				int t;
				unsigned int p;
				t = block->AllocateBlock(p);
				if(t<0)
				{
					return -1;
				}
				// refresh iNode
				v[i].inode.indices[index] = p;
				v[i].inode.size += size;
				directory->changeIndex(i,index,p);
				directory->changeSize(i,v[i].inode.size);
				t = block->putiNode(v[i].index,&v[i].inode);
				if(t<0)
					return -1;
				data = new char[blSize];
			}
			else //else read block
			{
				block->ReadBlock(v[i].inode.indices[index],data);
			}
			memcpy(data+pp,buf+n,size); // copy block to buf
			block->WriteBlock(v[i].inode.indices[index],data); // write to disk
			if(index*blSize+pp+size>v[i].inode.size)
			{
				int t;
				v[i].inode.size += size;
				directory->changeSize(i,v[i].inode.size);
				t = block->putiNode(v[i].index,&v[i].inode);
				if(t<0)
					return -1;
			}
			n += size;
			pp = 0;
			index++;
			loop--;
		}
		return n;
	}
};

FileService* file = new FileService; // global variable

#endif
