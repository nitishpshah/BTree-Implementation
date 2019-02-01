#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

#define PAGE_SIZE 4096


#define FILE_NAME_SIZE 4
#define KEY_SIZE 81
#define VALUE_SIZE 81
#define RECORD_SIZE KEY_SIZE + VALUE_SIZE

const int MAX_RECORDS_IGNORE = (PAGE_SIZE - FILE_NAME_SIZE) / (RECORD_SIZE + FILE_NAME_SIZE);
const int DEGREE = (MAX_RECORDS_IGNORE + 1) / 2;
const int MAX_RECORDS = (DEGREE * 2) - 1;
const int MAX_FILE_POINTERS = MAX_RECORDS + 1;

size_t disk_reads = 0;
size_t disk_writes = 0;
double disk_read_tot = 0.0;
double disk_write_tot = 0.0;

struct record{
	std::string key;
	std::string value;
	record(){
		key.resize(81);
		value.resize(81);
	}
	record(std::string k, std::string v){
		key = k;
		value = v;
		key.resize(81);
		value.resize(81);
	}
};

struct btree_node
{
	bool isleaf;
	short n_records;
	std::string own_name;
	std::string filenames[MAX_FILE_POINTERS];
	record records[MAX_RECORDS];
	void split_child(btree_node& y, std::string new_name, int index);
	bool read(std::string in_file_name);
	void write();
	btree_node(std::string name, bool leaf = 0, short m = 0){
		//if the file exists, read from the file
		bool x = read(name);
		if (x == 0){
			//else make new file and set all to defaults
			//will make a new file if the file cant be read for whatever reason
			own_name = name;
			isleaf = leaf;
			n_records = m;
			own_name.resize(FILE_NAME_SIZE);
			for(size_t i = 0; i <= MAX_RECORDS; i++){
				filenames[i].resize(FILE_NAME_SIZE);
			}
		}
	}
	bool check_size(){
		if((sizeof(bool) + sizeof(short) + FILE_NAME_SIZE + RECORD_SIZE*MAX_RECORDS + FILE_NAME_SIZE*MAX_FILE_POINTERS) > PAGE_SIZE){
			std::cout<<"record size exceeds the page size"<<std::endl;
			return false;
		}else{
			return true;
		}

	}
	void display(){
		std::cout << "isleaf: " << isleaf << std::endl;
		std::cout << "n_rocords: " << n_records << std::endl;
		std::cout << "own_name: " << own_name << std::endl;
		if(n_records > 0){
			for (size_t i = 0; i < n_records; i++){
				std::cout << filenames[i] << " " << records[i].key <<"	"<<records[i].value << " " << i <<std::endl;
			}
			std::cout<<filenames[n_records]<<std::endl<<std::endl;
		}
		return;
	}
};


void btree_node::split_child(btree_node& y, std::string new_name, int index){
	//y is the child of the node this funcion is called on 
	//( so that it is onl read once), 
	//newname is the filename the new node will be stored in
	//index if the index of the node the child is at

	//assume that the check that the nod is full is done before calling
	// therefore the degree is the total number of nodes in (y-1)/2

	btree_node z(new_name);
	int degree =  (y.n_records + 1)/2;
	z.isleaf = y.isleaf;
	z.n_records = degree - 1;

	for (size_t j = 0; j < degree - 1; j++) {
		z.records[j] = y.records[degree + j];
	}
	if (!y.isleaf) {
		for (size_t j = 0; j < degree; j++) {
			z.filenames[j] = y.filenames[degree + j];
		}
	}
	y.n_records = degree - 1;
	for (size_t i = n_records; i >= index + 1; i--)
	{
		//shift filenames
		filenames[i+1] = filenames[i];
	}
	filenames[index + 1] = new_name;

	for (int j = n_records - 1 ; j >= index ; j--)
	{
		records[j+1] = records[j];
	}
	records[index] = y.records[degree - 1];
	n_records += 1;
	z.write();
	y.write();
	this->write();
	return;
}


bool btree_node::read(std::string in_file_name)
{
	disk_reads += 1;
	auto startTime_r = std::chrono::high_resolution_clock::now();
	std::ifstream infile(in_file_name.c_str(), std::ios::in | std::ios::binary);
	if (infile.fail()) {
		return false;
	}
	own_name.resize(FILE_NAME_SIZE);
	infile.read(&own_name[0], FILE_NAME_SIZE);
	infile.read((char *)&isleaf, sizeof(bool));
	infile.read((char *)&n_records, sizeof(short));
	for (int i = 0; i < n_records; i++)
	{
		filenames[i].resize(FILE_NAME_SIZE);
		records[i].key.resize(KEY_SIZE);
		records[i].value.resize(VALUE_SIZE);
		infile.read(&filenames[i][0], FILE_NAME_SIZE);
		infile.read(&records[i].key[0], KEY_SIZE);
		infile.read(&records[i].value[0], VALUE_SIZE);
	}
	filenames[n_records].resize(FILE_NAME_SIZE);
	infile.read(&filenames[n_records][0], FILE_NAME_SIZE);
	infile.close();
	auto endTime_r = std::chrono::high_resolution_clock::now();
	disk_read_tot += (std::chrono::duration_cast<std::chrono::milliseconds>(endTime_r - startTime_r).count()) / 1000.0;
	return true;
}

void btree_node::write() {
	disk_writes += 1;
	auto startTime_r = std::chrono::high_resolution_clock::now();
	std::ofstream outfile(own_name.c_str(), std::ios::out | std::ios::binary);
	outfile.write(&own_name[0], FILE_NAME_SIZE);
	outfile.write((char *)&isleaf, sizeof(bool));
	outfile.write((char *)&n_records, sizeof(short));
	for (int i = 0; i < n_records; i++)
	{
		filenames[i].resize(FILE_NAME_SIZE);
		records[i].key.resize(KEY_SIZE);
		records[i].value.resize(VALUE_SIZE);
		outfile.write(&filenames[i][0], FILE_NAME_SIZE);
		outfile.write(&records[i].key[0], KEY_SIZE);
		outfile.write(&records[i].value[0], VALUE_SIZE);
	}
	filenames[n_records].resize(FILE_NAME_SIZE);
	outfile.write(&filenames[n_records][0], FILE_NAME_SIZE);
	outfile.close();
	auto endTime_r = std::chrono::high_resolution_clock::now();
	disk_write_tot += (std::chrono::duration_cast<std::chrono::milliseconds>(endTime_r - startTime_r).count()) / 1000.0;
}