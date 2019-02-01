#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include "btree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <cstdio>


using namespace std;

int main()
{

	system("mkdir db");
	string dir = "./db";
	//TODO: try if works in linux
	if (chdir(dir.c_str())){
		switch (errno){
		case ENOENT:
			cout << "Unable to locate the directory: %s\n";
			break;
		case EINVAL:
			cout<<"Invalid buffer.\n";
			break;
		default:
			cout<<"Unknown error.\n";
		}
		cin.ignore();
		return -1;
	}
	
	btree tree_t("tree");
	tree_t.display();

	/************************************ PRINT STATS **********************************************/
	cout<<"Total number of nodes: "<<tree_t.n_nodes<<"\n";
	cout<<"Height of the tree: "<<tree_t.height<<"\n";
	cout<<"Degree of the tree: "<<tree_t.degree<<"\n";
	cout<<"Total number of disk reads done: " << disk_reads<<"\n";
	cout<<"Total number of disk writes done: "<< disk_writes<<endl;
	cout<<"Total time taken for all the disk reads: "<<disk_read_tot<<" s"<<endl;
	cout<<"Total time taken for disk writes: "<<disk_write_tot<<" s"<<endl;
	cout<<"Average disk read time: " << disk_read_tot / disk_reads << " s" << endl;
	cout<<"Average disk write time: "<<disk_write_tot/disk_writes<<" s"<<endl;
	cout<<"Root: "<<tree_t.root_file_name<<"\n";

	/*
		Building the tree took 1 minute(s) and 7.36 second(s)
		building the tree: done
		Total number of nodes: 623
		Height of the tree: 3
		Degree of the tree: 12
		Total number of disk reads done: 24448
		Total number of disk writes done: 11871
		Total time taken for all the disk reads: 0.003 s
		Total time taken for disk writes: 55.189 s
		Average disk read time: 1.22709e-07 s
		Average disk write time: 0.00464906 s
		Root: 00az
	*/
	cout<<"\n**************************************************\nEnter \nk to search for a (k)ey\nf to display the contents of a (f)ile\nx to e(x)it\n\n**************************************************\n";
	string search_key, result;
	ifstream sudoku_puzzles("./../sudoku.csv", ios::in);
	while (cin >> search_key){
		if(search_key == "k"){
			cout<<"enter a key: "<<endl;
			cin>>search_key;
			auto startTime = std::chrono::high_resolution_clock::now();
			//cout << tree_t.search(search_key) << endl;
			string solution = tree_t.search(search_key);
			if (solution == ""){
				cout<<"no solution found for this puzzle"<<endl;
			}
			else{
				cout<<solution<<endl;
			}
			auto endTime = std::chrono::high_resolution_clock::now();
			cout << "The search took ";
			cout << (std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count())/1000.0 << " milliseconds"<<endl;

		}else if (search_key == "f"){
			cout<< "enter a file name"<<endl;
			cin >> search_key;
			btree_node temp(search_key);
			temp.display();
		}
		else if (search_key == "x"){
			break;
		}else if(search_key == "p"){
			size_t pos;
			cin>>pos;
			sudoku_puzzles.seekg(pos, ios::beg);
			getline(sudoku_puzzles, result);
			cout<<result<<endl;
		}
	}
	sudoku_puzzles.close();
	cin.ignore();
}