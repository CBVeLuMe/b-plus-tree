// BPlusTree.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <typeinfo>

const int gOrder = 4;//the max children of a internal node, leaf also has 4, but last 1 for sibling


//Base class for internal and leaf node
//To delete the pointer!
class node {
public:
	node();
	node(node* parentPtr);
	void set_parent_ptr(node* parentPtr);
	node* get_parent_ptr();
	std::string get_first_key();
	int get_keys_size();
	virtual bool is_max() = 0;
	virtual bool is_leaf() = 0;
	//Parse any key to compare
	//virtual node* query_next_ptr(int index) = 0;
	virtual node* query_key(std::string key) = 0;
	static int compare_key(std::string sub, std::string obj);
private:
protected:
	virtual void add_key(std::string newKey);
	node* mParentPtr;
	std::vector<std::string> mKeys;
};

node::node() {
	set_parent_ptr(NULL);
	mKeys.reserve(gOrder - 1);//round_up(order / 2) - 1 <= number of keys <= order - 1
}

node::node(node* parentPtr) {
	set_parent_ptr(parentPtr);
	mKeys.reserve(gOrder - 1);//round_up(order / 2) - 1 <= number of keys <= order - 1
}

void node::set_parent_ptr(node* parentPtr) {
	mParentPtr = parentPtr;
}

node* node::get_parent_ptr() {
	return mParentPtr;
}

std::string node::get_first_key() {//should be min key
	return mKeys.front();
}

int node::get_keys_size() {
	return mKeys.size();
}

int node::compare_key(std::string subStr, std::string objStr) {
	for (int i = 0; i < subStr.length(); i++) {
		if (subStr[i] > objStr[i]) return 1;//sub > obj
	}
	if (subStr == objStr) return 0;//sub = obj
	else return -1;//sub < obj
}

void node::add_key(std::string newKey) {
	mKeys.push_back(newKey);
}

class non_leaf : public node {
	using node::node;
public:
	non_leaf();
	non_leaf(std::vector<node*>* childPtrs);
	node* get_child_ptr(int index);
	//int get_children_size();//also key size => remove?
	bool is_max();
	bool is_leaf();
	//bool is_root();
	node* query_key(std::string key);
	void add_child_ptr(node* newChildPtr);
	void print();
private:
	std::vector<node*> mChildPtrs;
};

non_leaf::non_leaf() : node() {
	mChildPtrs.reserve(gOrder);
}

non_leaf::non_leaf(std::vector<node*>* childPtrs) : non_leaf() {
	std::vector<node*>::iterator it;
	for (it = childPtrs->begin(); it != childPtrs->end(); it++) {
		add_child_ptr(*it);
	}
}

//excpt max, also needs to check min
bool non_leaf::is_max() {
	return mChildPtrs.size() == gOrder;
}

bool non_leaf::is_leaf() {
	return false;
}

//Input a key and it will return a avalaible child index for next internal or leaf or record
//Also return -1 means no, you fool, cannot be no key from rule
node* non_leaf::query_key(std::string key) {
	std::string objStr;
	//Traverse the keys
	std::vector<std::string>::iterator it;
	int i = 0;
	for (it = mKeys.begin(); it != mKeys.end(); it++, i++) {
		objStr = *it;
		int result = compare_key(key, objStr);
		if (result < 0) return get_child_ptr(i);
		else if (result == 0) return get_child_ptr(i++);
	}
	return get_child_ptr(i);//return the last index
}

void non_leaf::add_child_ptr(node* newChildPtr) {
	if (!is_max())
		mChildPtrs.push_back(newChildPtr);
	else//to do split
		std::cout << "Warning: You CANNOT go here now." << std::endl;
}

node* non_leaf::get_child_ptr(int index) {
	return mChildPtrs.at(index);
}

void non_leaf::print() {
	std::vector<std::string>::iterator keysIt = mKeys.begin();
	std::vector<node*>::iterator childIt = mChildPtrs.begin();
	while (childIt != mChildPtrs.end()) {
		std::cout << /**keysIt <<*/ "    " << *childIt << " | ";
		//keysIt++;
		childIt++;
	}
	std::cout << std::endl;
}

class leaf : public node {
	using node::node;
public:
	leaf();
	void set_pair(std::string newKey, std::string newRecord);
	void set_next_sibling_ptr(leaf* nextPtr);
	leaf* get_next_sibling_ptr();
	bool is_max();
	bool is_leaf();
	leaf* query_key(std::string key);
	void print();
private:
	void add_record(std::string newRecord);
	std::vector<std::string> mRecords;
	leaf* mNextSiblingPtr = NULL;
};

leaf::leaf() : node() {
	mRecords.reserve(gOrder - 1);
}

void leaf::set_pair(std::string newkey, std::string newRecord) {
	add_key(newkey);
	add_record(newRecord);
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

void leaf::add_record(std::string newRecord) {
	mRecords.push_back(newRecord);
}

bool leaf::is_max() {
	return mRecords.size() == gOrder - 1;
}

bool leaf::is_leaf() {
	return true;
}

leaf* leaf::query_key(std::string key) {
	std::string objStr;
	//Traverse the keys
	std::vector<std::string>::iterator it;
	int i = 0;
	for (it = mKeys.begin(); it != mKeys.end(); it++, i++) {
		objStr = *it;
		if (compare_key(key, objStr) == 0) {
			leaf* leafPtr = this;
			return leafPtr;
		}
	}
	return NULL;//means no key
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
	b_plus_tree();
	void initial(std::string);
	void traverse_leaf();
	leaf* query(std::string key);
	void insert(std::string key, std::string record);
	//void delete(std::string key);
private:
	non_leaf* mRoot;
	leaf* mHeader;
	void set_root(non_leaf* newRoot);
	non_leaf* get_root_ptr();
	leaf* get_header_ptr();
	std::string trim(std::string);
};

b_plus_tree::b_plus_tree() {
	mRoot = new non_leaf();
	mHeader = new leaf();
}

//should ensure being sequential (next larger than cur, not same)
void b_plus_tree::initial(std::string filePath) {
	std::ifstream infile(filePath);
	std::string line, key, record;//record should be ptr to fstream address
	
	leaf* leafPtr = get_header_ptr();
	for (std::string line; std::getline(infile, line); ) {
		key = trim(line.substr(0, 7));
		record = trim(line.substr(15, 80));
		if (!(leafPtr->is_max())) {
			leafPtr->set_pair(key, record);
		}
		else {
			leaf* nextPtr = new leaf();
			leafPtr->set_next_sibling_ptr(nextPtr);
			leafPtr = nextPtr;
			leafPtr->set_pair(key, record);
		}
	}
}

void b_plus_tree::traverse_leaf() {
	//test
	non_leaf* rootPtr = get_root_ptr();

	leaf* leafPtr = get_header_ptr();
	leafPtr->print();
	while (leafPtr->get_next_sibling_ptr() != NULL) {
		leafPtr = leafPtr->get_next_sibling_ptr();
		leafPtr->print();
		//test
		if (!rootPtr->is_max()) {
			rootPtr->add_child_ptr(leafPtr);
		}
	}
	//Test
	rootPtr->print();
	query("AAA-377")->print();
}

leaf* b_plus_tree::query(std::string key) {
	node* nodePtr = get_root_ptr();
	do {
		nodePtr = nodePtr->query_key(key);
		if (nodePtr->is_leaf()) {
			//cast supertype to derived type
			return dynamic_cast<leaf*>(nodePtr->query_key(key));
		}
	} while (true);
	return NULL;
}

void b_plus_tree::insert(std::string key, std::string record) {
	//query

}

void b_plus_tree::set_root(non_leaf* newRoot) {
	mRoot = newRoot;
}

non_leaf* b_plus_tree::get_root_ptr() {
	return mRoot;
}

leaf* b_plus_tree::get_header_ptr() {
	return mHeader;
}

std::string b_plus_tree::trim(std::string str) {
	str.erase(0, str.find_first_not_of(" "));
	str.erase(str.find_last_not_of(" ") + 1);
	return str;
}

int main()
{
	b_plus_tree tree;
	tree.initial("D:\\CBVeLuMe\\Desktop\\WorkNote\\algorithm-concepts\\BPlusTree\\temp.txt");
	tree.traverse_leaf();

	//node* keyPtr = tree.get_root_ptr();
	//std::cout << typeid(keyPtr).name() << std::endl;
	//std::cout << keyPtr->is_leaf() << std::endl;
}