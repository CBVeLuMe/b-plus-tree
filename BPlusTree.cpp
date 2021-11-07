// BPlusTree.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

const int gOrder = 4;//the max children of a internal node, leaf also has 4, but last 1 for sibling

//Base class for internal and leaf node
class node {
public:
	node();
	node(node* parentPtr);
	std::string get_first_key();
	int get_keys_size();
	virtual bool is_full() = 0;
	//Parse key to compare
	static bool is_same_key(std::string sub, std::string obj);
	static bool is_larger_key(std::string sub, std::string obj);
private:
protected:
	node* mParentPtr;
	std::vector<std::string> mKeys;
	virtual void push_key_back(std::string newKey);
};

node::node() {
	mParentPtr = NULL;
	mKeys.reserve(gOrder - 1);//round_up(order / 2) - 1 <= number of keys <= order - 1
}

node::node(node* parentPtr) {
	mParentPtr = parentPtr;
	mKeys.reserve(gOrder - 1);//round_up(order / 2) - 1 <= number of keys <= order - 1
}

std::string node::get_first_key() {//should be min key
	return mKeys.front();
}

int node::get_keys_size() {
	return mKeys.size();
}

bool node::is_same_key(std::string sub, std::string obj) {
	if (sub == obj) {
		return true;
		std::cout << "Warning: Key cannot be same." << std::endl;
	}
	return false;
}

bool node::is_larger_key(std::string sub, std::string obj) {
	if (is_same_key(sub, obj)) return false;
	for (int i = 0; i < sub.length(); i++) {
		if (sub[i] > obj[i]) return true;
	}
	return false;
}

void node::push_key_back(std::string newKey) {
	mKeys.push_back(newKey);
}

class root : public node {
	using node::node;
	bool is_full() {
		return true;
	}
};

class non_leaf : public node {
	using node::node;
	bool is_full() {
		return true;
	}
};

class leaf : public node {
	using node::node;
public:
	leaf();
	void set_pair(std::string newKey, std::string newRecord);
	void set_next_sibling_ptr(leaf* nextPtr);
	leaf* get_next_sibling_ptr();
	bool is_full();
	void print();
private:
	void push_record_back(std::string newRecord);
	std::vector<std::string> mRecords;
	leaf* mNextSiblingPtr = NULL;
};

leaf::leaf() : node() {
	mRecords.reserve(gOrder - 1);
}

void leaf::set_pair(std::string newkey, std::string newRecord) {
	push_key_back(newkey);
	push_record_back(newRecord);
	//for (int i = 0; i < sizeof(mKeys); i++) {
		////should compare here
	//}
}

void leaf::set_next_sibling_ptr(leaf* nextPtr) {
	mNextSiblingPtr = nextPtr;
}

leaf* leaf::get_next_sibling_ptr() {
	return mNextSiblingPtr;
}

void leaf::push_record_back(std::string newRecord) {
	mRecords.push_back(newRecord);
}

bool leaf::is_full() {
	return mRecords.size() == gOrder - 1;
}

void leaf::print() {
	std::vector<std::string>::iterator keysIt = mKeys.begin();
	std::vector<std::string>::iterator recordsIt = mRecords.begin();
	while (recordsIt != mRecords.end()) {
		std::cout << *keysIt << "    " << *recordsIt << " | ";
		keysIt++;
		recordsIt++;
	}
	std::cout << std::endl;
}

class b_plus_tree {
public:
	leaf* initial(std::string);
	std::string trim(std::string);
	void traverse(leaf* header);
private:
	leaf* mHeader;
};

//should ensure being sequential (next larger than cur, not same)
leaf* b_plus_tree::initial(std::string filePath) {
	std::ifstream infile(filePath);
	std::string line, key, record;//record should be ptr to fstream address
	leaf* headerPtr = new leaf();
	leaf* leafPtr = headerPtr;
	for (std::string line; std::getline(infile, line); ) {
		key = trim(line.substr(0, 7));
		record = trim(line.substr(15, 80));
		if (!(leafPtr->is_full())) {
			leafPtr->set_pair(key, record);
		}
		else {
			leaf* nextPtr = new leaf();
			leafPtr->set_next_sibling_ptr(nextPtr);
			leafPtr = nextPtr;
			leafPtr->set_pair(key, record);
		}
	}
	return headerPtr;
}

std::string b_plus_tree::trim(std::string str) {
	str.erase(0, str.find_first_not_of(" "));
	str.erase(str.find_last_not_of(" ") + 1);
	return str;
}

void b_plus_tree::traverse(leaf* headerPtr) {
	headerPtr->print();
	while (headerPtr->get_next_sibling_ptr() != NULL) {
		headerPtr = headerPtr->get_next_sibling_ptr();
		headerPtr->print();
	}
}

int main()
{
	b_plus_tree tree;
	leaf* headerPtr = tree.initial("D:\\CBVeLuMe\\Desktop\\WorkNote\\algorithm-concepts\\BPlusTree\\temp.txt");
	tree.traverse(headerPtr);
}