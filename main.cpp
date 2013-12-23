#include <iostream>
#include <stdio.h>
#include <termios.h>
#include "fileService.h"
using namespace std;

/******************************
main.cpp
------------------
Shell
*******************************/

int InitAll() // system initialization - (mkfs)
{
	int fp;
	fp = open(system_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if(fp<0)
		return -1;
	int i = 8192;
	write(fp,&i,4);
	i = 1024;
	write(fp,&i,4);
	i=1;
	write(fp,&i,4);
	i=15;
	write(fp,&i,4);
	i=128;
	write(fp,&i,4);
	lseek(fp,1024,SEEK_SET);
	int j=0;
	int* k = new int[256*123536];
	memset(k,0,256*123536*sizeof(int));
	write(fp,k,256*123536*sizeof(int));
	close(fp);
	return 0;
}


int main() //main program
{
	// if no system file, we create it
	int fp;
	fp = open(system_file.c_str(), O_RDONLY);
	if(fp < 0)
		InitAll();
	else
		close(fp);
	cout<<"SIMPLE FLAT FILESYSTEM\n\nType help for instructions\n\n";
	string c,command, op1, op2;
	int s1,s2;
	bool mounted = false;
	while(1) //shell
	{
		cout<<"Command>>";
		getline(cin,c);
		s1 = c.find(' ');
		s2 = c.find(' ',s1+1);
		if(s1 != string::npos)
		{
			command = c.substr(0,s1);
			op1 = c.substr(s1+1);
			if(s2 != string::npos)
			{
				op2 = op1.substr(s2-command.length());
				op1 = op1.substr(0, s2-command.length()-1);
			}
		}
		else
			command = c;
		if(command == "mkfs") //initialize system file
		{
			InitAll();
			mounted = false;
		}
		else if(command == "quit")
		{
			break;
		}
		else if(command == "help") //Get Help
		{
			cout<<"Commands:\n1. mkfs - initialize/create the system file\n2. mount - mount the virtual hard drive (system file)\n3. ls - show all files\n4. unmount - unmount the virtual drive\n5. cp src dest - copies the data from src file to dest (src";
			cout<<" must exist, dest is created if not existed, if existed dest loses all its previous data)\n6. rm file - removes file (if it exists)\n7. cat file - show the data of the file on the screen\n8. echo file - inputs data to file (ctrl-d to";
			cout<<" terminate)\n9. quit - exit the shell\n\nInformation:\nBefore you can type any command you must first mount the drive. If you exit without unmounting, nothing will be saved. So, first unmount and then quit.\n";
		}
		else if(!mounted) //If system file not loaded
		{
			if(command=="mount") //you have to mount it first
			{
				block->Initialize();
				mounted = true;
			}
			else
			{
				cout<<"Invalid command!\n";
			}
		}
		else
		{
			if(command == "ls") //display files
			{
				char** files;
				files = new char*[block->MaxFiles()];
				int n = directory->files(files);
				for(int i=0;i<n;i++)
				{
					cout<<files[i]<<endl;
				}
				if(n==0)
					cout<<"No files!\n";
			}
			else if(command == "unmount") //unmount
			{
				vector<openFile> v = directory->OpenFiles();
				for(int i=0;i<v.size();i++)
				{
					file->close(v[i].ufid);
				}
				block->WrapUp();
				mounted = false;
			}
			else if(command == "cp") //copy
			{
				int t;
				int f1 = directory->open(op1.c_str()); //open source file
				if(f1<0)
				{
					cout<<"Error opening source file!\n";
					continue;
				}
				int f2 = directory->create(op2.c_str()); //create or clear dest file
				if(f2<0)
				{
					cout<<"Error opening destination file!\n";
					continue;
				}
				f2 = directory->open(op2.c_str()); //open dest file
				if(f2<0)
				{
					cout<<"Error opening destination file!\n";
					continue;
				}
				int l1 = file->file_size(f1);
				if(l1<0)
				{
					cout<<"Error getting size of source file!\n";
					continue;
				}
				char* data = new char[l1];
				t = file->read(f1,data,l1,0); //get data from source file
				if(t<0)
				{
					cout<<"Error reading source file!\n";
					continue;
				}
				t = file->write(f2,data,l1,0); //write data to dest file
				if(t<0)
				{
					cout<<"Error writing destination file!\n";
					continue;
				}
				file->close(f1);
				file->close(f2);
			}
			else if(command == "rm") //remove file
			{
				int f1 = directory->remove(op1.c_str());
				if(f1<0)
				{
					cout<<"Error deleting file!\n";
					continue;
				}
			}
			else if(command == "cat") //get data from file
			{
				int t;
				int f1 = directory->open(op1.c_str());
				if(f1<0)
				{
					cout<<"Error opening file!\n";
					continue;
				}
				int l1 = file->file_size(f1);
				if(l1<0)
				{
					cout<<"Error getting size of file!\n";
					continue;
				}
				char* data = new char[l1];
				t = file->read(f1,data,l1,0);
				if(t<0)
				{
					cout<<"Error reading file!\n";
					continue;
				}
				file->close(f1);
				cout<<"***************"<<endl<<data<<endl<<"***************"<<endl;
			}
			else if(command == "echo") //add data to (end of) file, free-form text (ctrl-d to stop)
			{
				int t,l;
				int f1 = directory->open(op1.c_str());
				if(f1<0)
				{
					t = directory->create(op1.c_str()); //create new if file doesn't exist
					f1 = directory->open(op1.c_str());
					if(f1<0||t<0)
					{
						cout<<"Error opening file!\n";
						continue;
					}
				}
				char* buf = new char[1000];
				cout<<"Write Your Data:\n";
				while(fgets(buf,1000,stdin))
				{
					l = file->file_size(f1);
					t = file->write(f1,buf,strlen(buf),l);
					if(t<0)
					{
						cout<<"Error writing to file\n";
						break;
					}
				}
				file->close(f1);
				cout<<endl;
			}
			else
			{
				cout<<"Invalid command!\n";
			}
		}
	}
	return 0;
}
