// BPlusTree.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

//Base class for internal and leaf
class node {
public:

	virtual void set_keys(string* keys) = 0;
	virtual string* get_keys() = 0;
	
protected:
	string mKeys[3];//round_up(order / 2) - 1 <= number of keys <= order - 1
};

class root : public node {

};

class internal : public node {
public:
	void set_keys(string* newKeys);
	string* get_keys();
	void set_ptrs(node* newPtrs);
	internal(string* newKeys, node* newPtrs) {
		set_keys(newKeys);
		set_ptrs(newPtrs);
	}
private:
	node* mPtrs[4];//round_up(order / 2) <= number of children <= order
};

void internal::set_keys(string* newKeys) {
	for (int i = 0; i < sizeof(mKeys); i++) {
		mKeys[i] = newKeys[i];
	}
}

string* internal::get_keys() {
	return mKeys;
}

void internal::set_ptrs(node* newPtrs) {
	for (int i = 0; i < sizeof(mPtrs); i++) {
		mPtrs[i] = &newPtrs[i];
	}
}

class leaf : public node {//Or Leaf Node
public:
	void set_keys(string* newKeys);
	void set_keys(string newKey, int index);
	string* get_keys();
	void set_ptrs(string* newPtrs);
	string* get_ptrs(void);
	void set_next_sibling(leaf* nextPtr);
	leaf* get_next_sibling(void);
	void print(void);
	void print(int i);
private:
	string mPtrs[3];//Todo: should be fstream pointer
	leaf* mNextSibling = NULL;
};

void leaf::set_keys(string* newKeys) {
	for (int i = 0; i < sizeof(mKeys); i++) {
		mKeys[i] = newKeys[i];
	}
}

void leaf::set_keys(string newKey, int index) {
	mKeys[index] = newKey;
}

string* leaf::get_keys() {
	return mKeys;
}

void leaf::set_ptrs(string* newPtrs) {
	for (int i = 0; i < sizeof(mPtrs); i++) {
		mPtrs[i] = newPtrs[i];
	}
}

string* leaf::get_ptrs() {
	return mPtrs;
}

void leaf::set_next_sibling(leaf* nextPtr) {
	mNextSibling = nextPtr;
}

leaf* leaf::get_next_sibling() {
	return mNextSibling;
}

void leaf::print() {
	std::cout << " key: " + mKeys[0] << "\n";
	std::cout << " key: " + mKeys[1] << "\n";
	std::cout << " key: " + mKeys[2] << "\n";
	//for (int i = 0; i < sizeof(mKeys); i++) {
	//	std::cout << " key: " + mKeys[1] + " record: " << "\n";
	//}
}

void leaf::print(int i) {
	std::cout << " key: " + mKeys[i] + " record: " + mPtrs[i] << "\n";
}

class b_plus_tree {
public:
	int get_order();
	leaf* Initial(string);
	string trim(string);
	void traverse(leaf* header);
	//char* parse_key(string);
private:
	int order = 4;
};

int b_plus_tree::get_order() {
	return order;
}

leaf* b_plus_tree::Initial(string filePath) {
	fstream fsfile;
	string line, key, record;
	fsfile.open(filePath);
	int counter = 0;
	leaf* header = new leaf();
	leaf* leafPtr = header;
	while (getline(fsfile, line)) {
		istringstream iss(line);
		key = trim(line.substr(0, 7));
		//record = trim(line.substr(15, 80));
		//char keys [8] = strncpy_s(keys, key.c_str(), sizeof(keys));

		if (counter != 0 && counter % (get_order() - 1) == 0) {
			leafPtr->print();
			leaf* nextPtr = new leaf();
			leafPtr->set_next_sibling(nextPtr);
			leafPtr = nextPtr;
		}
		leafPtr->set_keys(key, (counter % (get_order() - 1)));
		counter++;
	}
	return header;
}

string b_plus_tree::trim(string str) {
	str.erase(0, str.find_first_not_of(" "));
	str.erase(str.find_last_not_of(" ") + 1);
	return str;
}

void b_plus_tree::traverse(leaf* header) {
	while (header->get_next_sibling() == NULL) {
		header->print();
		leaf* nextPtr = header->get_next_sibling();
	}
}

int main()
{
	b_plus_tree tree;
	leaf* header = tree.Initial("D:\\CBVeLuMe\\Desktop\\WorkNote\\algorithm-concepts\\BPlusTree\\partfile.txt");
	tree.traverse(header);
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
