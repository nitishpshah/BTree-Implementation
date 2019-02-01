#pragma once

// TODO: store using linked lists instead of arrays, should be much faster then current implementatioin

#include "btree_node.h"
#include <iostream>
#include <fstream>	
#include <string>

char next_char(char z);
void new_name(std::string& s);

class btree
{
	/*
	each node is stored in a file
	*/
public:
	short height;
	short degree;	// t, number of minimum records in each file = t-1 to 2t-1
	short file_name_length;
	std::string root_file_name;
	std::string current_file_name;
	std::string directory;
	size_t n_nodes;
	//size_t disk_reads;
	//size_t disk_writes;
	std::string own_name;
	btree_node root;
	
	btree(std::string tree_filename, int d = 0, int fnl = FILE_NAME_SIZE): root("0000", 1) {
		if (!this->read(tree_filename)){
			std::cout << "read from file unsuccessful" << std::endl;
			n_nodes = 0;
			height = 0;
			if (d == 0){
				/*if no value given, compute one*/
				degree = DEGREE;
			}else{
				degree = d;
			}
			current_file_name = "0000";
			root_file_name = "0000";
			root.read(root_file_name);
			file_name_length = fnl;
			own_name = tree_filename;
			write();
			root.write();
			root.check_size();
		}
	}

	void display(){
		std::cout << "number of nodes: " << n_nodes << std::endl;
		std::cout << "height: "<<height<<std::endl;
		std::cout << "degree: "<<degree<< std::endl;
		std::cout << "root_file_name: "<<root_file_name<< std::endl;
		std::cout << "own_name: "<<own_name<< std::endl;
		std::cout << "current file name: "<<current_file_name<< std::endl;
	}
	std::string search_in_node(const std::string& key, std::string filename);
	std::string search_in_node(const std::string& key, btree_node& cur_node);
	std::string search(const std::string& key);
	void insert(record& r);
	void insert_nofull(btree_node& r, record& k);
	bool read(std::string in_file_name);
	void write();
};

std::string btree::search(const std::string& key) {
	return search_in_node(key, root);
}

std::string btree::search_in_node(const std::string& key, std::string filename) {
	btree_node cur_node(filename);
	return search_in_node(key, cur_node);
}

std::string btree::search_in_node(const std::string& key, btree_node& cur_node){
	int i = 0;
	while (i < cur_node.n_records && cur_node.records[i].key < key){
		i++;
	}
	if (i <= cur_node.n_records && key == cur_node.records[i].key){
		return  cur_node.records[i].value;
	}
	else if (cur_node.isleaf){
		return "";
	}
	else{
		return search_in_node(key, cur_node.filenames[i]);
	}
}

void btree::insert(record& r){
	//btree_node r = root;
	if (root.n_records == 2 * degree - 1) {
		//use new? : make sure it is written to disk
		//newname for s
		new_name(current_file_name);
		btree_node s(current_file_name);
		s.filenames[0] = root_file_name;
		//s is the new root
		root_file_name = current_file_name;
		//root in memory still old root
		s.isleaf = false;
		s.n_records = 0;
		s.write();//write all the changes and store file name
		new_name(current_file_name);//newname for new child
		s.split_child(root, current_file_name, 0);//split s
		n_nodes += 1;
		height += 1;
		root.read(s.own_name);////change root in the memory to s
		insert_nofull(root, r);//insert to root in memory
	}
	else {
		insert_nofull(root, r);
	}
}

void btree::insert_nofull(btree_node& x, record& k) {
	if (x.n_records == 0){
		x.records[0] = k;
		x.n_records += 1;
		x.write();
		return;
	}
	int i = x.n_records - 1;
	if (x.isleaf) {
		while (i >= 0 && k.key < x.records[i].key) {
			// shift record i
			x.records[i+1] = x.records[i];
			i -= 1;
		}
		i += 1;       //necessary?
		x.records[i] = k;
		x.n_records += 1;
		x.write();
	}
	else {
		while (i >= 0 && k.key < x.records[i].key) {
			i--;
		}
		i += 1;
		btree_node child(x.filenames[i]); //will be deleted from the memory when the function is deleted
		if (child.n_records == 2 * degree - 1){
			new_name(current_file_name);
			x.split_child(child, current_file_name, i);
			n_nodes += 1;
			if (k.key > x.records[i].key){
				child.read(x.filenames[i+1]);
			}
		}
		insert_nofull(child, k);
	}
}

bool btree::read(std::string in_file_name)
{
	disk_reads += 1;
	//also loads root in the main memory
	auto startTime_r = std::chrono::high_resolution_clock::now();
	std::ifstream infile(in_file_name.c_str(), std::ios::in | std::ios::binary);
	if (infile.fail()){
		return false;
	}
	own_name.resize(FILE_NAME_SIZE);
	infile.read(&own_name[0], FILE_NAME_SIZE);
	infile.read((char *)&height, sizeof(short));
	infile.read((char *)&degree, sizeof(short));
	infile.read((char *)&file_name_length, sizeof(short));
	infile.read((char *)&n_nodes, sizeof(size_t));

	root_file_name.resize(FILE_NAME_SIZE);
	current_file_name.resize(FILE_NAME_SIZE);
	infile.read(&root_file_name[0], FILE_NAME_SIZE);
	infile.read(&current_file_name[0], FILE_NAME_SIZE);
	infile.close();
	root.read(root_file_name);
    auto endTime_r = std::chrono::high_resolution_clock::now();
	disk_read_tot += (std::chrono::duration_cast<std::chrono::milliseconds>(endTime_r - startTime_r).count())/1000.0;
	return true;
}

void btree::write() {
	disk_writes += 1;
	auto startTime_r = std::chrono::high_resolution_clock::now();
	std::ofstream outfile(own_name.c_str(), std::ios::out | std::ios::binary);
	own_name.resize(FILE_NAME_SIZE);
	root_file_name.resize(FILE_NAME_SIZE);
	current_file_name.resize(FILE_NAME_SIZE);

	outfile.write(&own_name[0], FILE_NAME_SIZE);
	outfile.write((char *)&height, sizeof(short));
	outfile.write((char *)&degree, sizeof(short));
	outfile.write((char *)&file_name_length, sizeof(short));
	outfile.write((char *)&n_nodes, sizeof(size_t));
	outfile.write(&root_file_name[0], FILE_NAME_SIZE);
	outfile.write(&current_file_name[0], FILE_NAME_SIZE);
	outfile.close();
	auto endTime_r = std::chrono::high_resolution_clock::now();
	disk_write_tot += (std::chrono::duration_cast<std::chrono::milliseconds>(endTime_r - startTime_r).count()) / 1000.0;
}

void new_name(std::string& s) {
	//start with 000
	int i = 0;
	for (i = s.length() - 1; i >= 0; i--) {
		if (s[i] != 'z') {
			s[i] = next_char(s[i]);
			return;
	}
		else {
			//reset
			s[i] = '0';
		}
	}
}

char next_char(char z) {
	/*

	ASCII values of characters
	48 = '0' 2^5 + 2^4 = 16 + 32
	57 = '9'
	65 = 'A' 2^6 + 1
	90 = 'Z'
	97 = 'a' 2^5 + 2^6 = 32 + 64
	122 = 'z'

	*/

	if ((short)z < 57) {
		return (char)((short)z + 1);
	}
	else if ((short)z == 57) {
		return (char)(97);
	}/*
	else if ((short)z < 90) {
		return (char)((short)z + 1);
	}
	else if ((short)z == 90) {
		return (char)97;
	}*/
	else if ((short)z < 122) {
		return (char)((short)z + 1);
	}
	else if ((short)z == 122) {
		return z;
	}
	else return (char)0;
}
