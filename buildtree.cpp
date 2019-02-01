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

	ifstream sudoku_puzzles("./../sudoku.csv", ios::in);
	if (sudoku_puzzles.fail()){
		cout << "fail" << endl;
		cin.ignore();
		return 0;
	}
	cout<<"Building the tree\n";
	string puzzle;
	auto startTime = std::chrono::high_resolution_clock::now();

	auto startTime2 = std::chrono::high_resolution_clock::now();
	auto endTime2 = std::chrono::high_resolution_clock::now();

	auto startTime3 = std::chrono::high_resolution_clock::now();
	auto endTime3 = std::chrono::high_resolution_clock::now();
	double eta = 0.0, weight_prev_eta = 0.75;
	//cout << (std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count()) / 1000.0 << " seconds" <<	endl;

	size_t line_num = 0, total_lines = 1000000 , start_line = 0;
	size_t acc = 1000;	//accuracy
	size_t to_100 = acc/100;
	size_t oneper = total_lines / acc;
	size_t per = 0;

	cout<<"total lines to read: "<<total_lines<<endl;

	getline(sudoku_puzzles, puzzle); //skip first line

	while (getline(sudoku_puzzles, puzzle)){
		if (line_num < start_line){
			continue;
		}
		line_num ++;
		if (line_num % oneper == 0){
			per++;
			endTime3 = std::chrono::high_resolution_clock::now();
			endTime2 = std::chrono::high_resolution_clock::now();
			eta = (std::chrono::duration_cast<std::chrono::milliseconds>(endTime2 - startTime2).count()) * (acc - per) / 1000.0;
			int eta_mins = floor(eta/60);
			int eta_secs = floor(eta - eta_mins*60);
			cout << per/to_100 << "% done eta: " << eta_mins << " minute(s) "<<eta_secs<<" second(s) Elapsed time: "<<(std::chrono::duration_cast<std::chrono::seconds>(endTime3 - startTime3).count())<<"(s) "<<line_num<<endl;
			startTime2 = std::chrono::high_resolution_clock::now();

		}
			
		if (line_num >= total_lines){
			break;
		}

		// ********************** insert ********************************
		record r(puzzle.substr(0, 81), puzzle.substr(82, 163));
		tree_t.insert(r);
		tree_t.write();		
	}
	sudoku_puzzles.close();
	cout<<"100% done\n";
	auto endTime = std::chrono::high_resolution_clock::now();
	cout << "\nBuilding the tree took ";
	float total_time = (std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count()) / 1000.0;
	int mins = floor(total_time/60);
	float seconds = total_time - mins*60;
	cout << mins << " minute(s) and " << seconds << " second(s)"<<endl;

	cout << "building the tree: done" << endl;
	tree_t.write();

	/************************************ PRINT STATS **********************************************/
	cout<<"Total lines read: "<<line_num<<endl;
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
	return 0;
}
