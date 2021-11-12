// BPlusTree.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Todo: Comments
// Todo: To delete the pointer! and use reference for minnecessary

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <typeinfo>

// Order is the max children of a internal node, leaf also has 4, but last 1 for sibling
static const int gOrder{ 4 };
static const int gMinDegree{ (int)(gOrder / 2 + 0.5) };
static const int gMaxDegree{ gOrder };

class node {
public:
	// Constructor
	node();
	node(node*);
	// Setter and Getter
	void set_parent_ptr(node*);
	node* get_parent_ptr();
	std::string get_first_key();
	virtual int get_values_size() = 0;// get the size of children or records
	// Check node status
	virtual bool upper_bound() = 0;
	virtual bool lower_bound() = 0;
	virtual bool is_leaf() = 0;
	// Support querying and inserting
	virtual node* parse_key(std::string) = 0;
	//virtual node* insert(std::string&, node*) = 0;
	virtual void print() = 0;
private:
protected:
	node* mParentPtr;
	std::vector<std::string> mKeys;
	// Setter and Getter
	void set_key(std::string);
	void set_keys();
	void set_keys(std::vector<std::string>);
	std::vector<std::string> get_keys();
	// Support querying and inserting
	static int compare_key(std::string, std::string);
	//virtual void split(std::vector<std::string>&, std::vector<node*>&) = 0;
};

node::node() {
	set_parent_ptr(NULL);
	set_keys();
}

node::node(node* parentPtr) {
	set_parent_ptr(parentPtr);
	set_keys();
}

void node::set_parent_ptr(node* parentPtr) {
	mParentPtr = parentPtr;
}

node* node::get_parent_ptr() {
	return mParentPtr;
}

std::string node::get_first_key() {
	return mKeys.front();
}

void node::set_key(std::string newKey) {
	mKeys.push_back(newKey);
}

void node::set_keys() {
	mKeys.clear();
	mKeys.reserve(gMaxDegree - 1);
}

void node::set_keys(std::vector<std::string> newKeys) {
	mKeys = std::move(newKeys);
}

std::vector<std::string> node::get_keys() {
	return mKeys;
}

/// <summary>
/// Compares two String keys and returns an integer that indicates comparative relationship.
/// </summary>
int node::compare_key(std::string subKey, std::string objKey) {
	for (int i = 0; i < subKey.length(); i++) {
		if (subKey[i] > objKey[i]) return 1;// sub > obj
	}
	if (subKey == objKey) return 0;// sub = obj
	else return -1;// sub < obj
}

class non_leaf : public node {
	using node::node;
public:
	// Constructor
	non_leaf(std::string, node*, node*);
	non_leaf(node*, std::vector<std::string>, std::vector<node*>);
	// Setter and Getter
	int get_values_size();
	// Check node status
	// bool is_root();
	bool upper_bound();
	bool lower_bound();
	bool is_leaf();
	// Support querying and inserting
	node* parse_key(std::string);
	node* insert(std::string&, node*);
	void print();
private:
	std::vector<node*> mChildPtrs;
	// Setter and Getter
	void set_child_ptr(node*);
	node* get_child_ptr(int);
	void set_child_ptrs();
	void set_child_ptrs(std::vector<node*>);
	std::vector<node*> get_child_ptrs();
	// Support querying and inserting
	void split(std::vector<std::string>&, std::vector<node*>&);
};

//For root only
non_leaf::non_leaf(std::string key0, node* leafPtr0, node* leafPtr1) : node() {
	set_child_ptrs();
	set_child_ptr(leafPtr0);
	set_key(key0);
	set_child_ptr(leafPtr1);
}

non_leaf::non_leaf(node* parentPtr, std::vector<std::string> newKeys, std::vector<node*> newChildPtrs) : node(parentPtr) {
	set_child_ptrs();
	set_keys(newKeys);
	set_child_ptrs(newChildPtrs);
}

int non_leaf::get_values_size() {
	return mChildPtrs.size();
}

bool non_leaf::upper_bound() {
	return mChildPtrs.size() == gMaxDegree;
}

bool non_leaf::lower_bound() {
	return mChildPtrs.size() >= gMinDegree;
}

bool non_leaf::is_leaf() {
	return false;
}

/// <summary>
/// Compares each string key and returns a pointer to the children node.
/// </summary>
node* non_leaf::parse_key(std::string key) {
	std::vector<std::string> tempKeys = get_keys();
	std::vector<std::string>::iterator it;
	int i = 0;
	for (it = tempKeys.begin(); it != tempKeys.end(); it++, i++) {
		int result = compare_key(key, *it);
		if (result < 0) return get_child_ptr(i);
		else if (result == 0) return get_child_ptr(i++);
	}
	return get_child_ptr(i);// return the right-most children
}

/// <summary>
/// Inserts the key and pointer into the corresponding position and returns parent pointer or NULL.
/// </summary>
node* non_leaf::insert(std::string& newKey, node* newChildPtr) {
	// Constructs two new vectors to contain the original and new value.
	std::vector<std::string> oriKeys = get_keys();
	std::vector<node*> oriChildPtrs = get_child_ptrs();
	std::vector<std::string>::iterator keyIt = oriKeys.begin();
	std::vector<node*>::iterator childPtrsIt = oriChildPtrs.begin();
	std::vector<std::string> tempKeys;
	tempKeys.reserve(gMaxDegree);// extra one for potential splitting
	std::vector<node*> tempChildPtrs;
	tempChildPtrs.reserve(gMaxDegree + 1);// extra one for potential splitting
	// Traverses the current vectors and finds a suitable position for new value.
	// For a inserting key, it always comes with a right-side new pointer, and the left-side is orignial.
	tempChildPtrs.push_back(*childPtrsIt);
	childPtrsIt++;
	for (keyIt; keyIt != get_keys().end(); keyIt++, childPtrsIt++) {
		if (compare_key(newKey, *keyIt) < 0) {
			tempKeys.push_back(newKey);
			tempChildPtrs.push_back(newChildPtr);
		}
		tempKeys.push_back(*keyIt);
		tempChildPtrs.push_back(*childPtrsIt);
	}
	if (compare_key(newKey, tempKeys.back()) > 0) {
		tempKeys.push_back(newKey);
		tempChildPtrs.push_back(newChildPtr);
	}
	// If the vectors are not full, add the record.
	if (tempKeys.size() < gMaxDegree) {
		set_keys(tempKeys);
		set_child_ptrs(tempChildPtrs);
		return NULL;// NULL need to continue
	}
	// Otherwise, split the vectors.
	else {
		split(tempKeys, tempChildPtrs);
		// Get the new key and child pointer for next inserting.
		newKey = tempKeys.back();
		newChildPtr = tempChildPtrs.back();
		return get_parent_ptr();// continue to insert to parent
	}
}

void non_leaf::print() {
	std::vector<std::string> tempKeys = get_keys();
	std::vector<node*> tempChildPtrs = get_child_ptrs();
	std::vector<std::string>::iterator keysIt = tempKeys.begin();
	std::vector<node*>::iterator childPtrsIt = tempChildPtrs.begin();
	std::cout << "Child: " << *childPtrsIt << " | ";
	childPtrsIt++;
	while (keysIt != get_keys().end()) {
		std::cout << "key: " << *keysIt << " | " << "Child: " << *childPtrsIt << " | ";
		keysIt++;
		childPtrsIt++;
	}
	std::cout << std::endl;
}

void non_leaf::set_child_ptr(node* newChildPtr) {
	if (upper_bound()) return;
	mChildPtrs.push_back(newChildPtr);
}

node* non_leaf::get_child_ptr(int index) {
	try {
		return mChildPtrs.at(index);
	}
	catch (const std::out_of_range& oor) {
		return NULL;
	}
}

void non_leaf::set_child_ptrs() {
	mChildPtrs.clear();
	mChildPtrs.reserve(gMaxDegree);
}

void non_leaf::set_child_ptrs(std::vector<node*> newChildPtrs) {
	mChildPtrs = std::move(newChildPtrs);
}

std::vector<node*> non_leaf::get_child_ptrs() {
	return mChildPtrs;
}

// Split and return new node pointer
void non_leaf::split(std::vector<std::string>& keys, std::vector<node*>& childPtrs) {
	if (keys.size() < gMaxDegree) return;
	// Re-intials the original vectors
	set_keys();
	set_child_ptrs();
	// Constructs a new vectors to contain the new value.
	std::vector<std::string> newKeys;
	newKeys.reserve(gMaxDegree - 1);// extra one for splitting
	std::vector<node*> newChildPtrs;
	newChildPtrs.reserve(gMaxDegree);// extra one for splitting
	// Traverses the new vectors and splits it into original and new Vectors.
	std::vector<std::string>::iterator keysIt;
	std::vector<node*>::iterator childPtrsIt = childPtrs.begin();
	int i = 0;
	std::string tempKey = "";
	for (keysIt = keys.begin(); keysIt != keys.end(); keysIt++, childPtrsIt++, i++) {
		if (i == gMinDegree) {
			set_child_ptr(*childPtrsIt);
			childPtrsIt++;
			tempKey = *keysIt;
			keysIt++;
			newChildPtrs.push_back(*childPtrsIt);
			childPtrsIt++;
		}
		if (i < gMinDegree) {
			set_child_ptr(*childPtrsIt);
			set_key(*keysIt);// extra one for parent
		}
		else {
			newKeys.push_back(*keysIt);
			newChildPtrs.push_back(*childPtrsIt);
		}
	}
	non_leaf* newNLeafPtr = new non_leaf(get_parent_ptr(), newKeys, newChildPtrs);
	// Add the extra key and new child pointer to vectors.
	keys.push_back(tempKey);
	childPtrs.push_back(newNLeafPtr);
}

class leaf : public node {
	using node::node;
public:
	// Constructor
	leaf(node*, std::vector<std::string>, std::vector<std::string>);
	// Setter and Getter
	void add_pair(std::string, std::string);
	int get_values_size();
	void set_next_sibling_ptr(leaf*);
	leaf* get_next_sibling_ptr();
	// Check node status
	bool upper_bound();
	bool lower_bound();
	bool is_leaf();
	// Support querying and inserting
	leaf* parse_key(std::string);
	void print();
private:
	std::vector<std::string> mRecords;
	leaf* mNextSiblingPtr = NULL;
	// Setter and Getter
	void set_record(std::string);
	void set_records(std::vector<std::string>);
	std::vector<std::string> get_records();
};

leaf::leaf(node* parentPtr, std::vector<std::string> newKeys, std::vector<std::string> newRecords) : node(parentPtr) {
	mRecords.reserve(gMaxDegree - 1);
	set_keys(newKeys);
	set_records(newRecords);
}

void leaf::add_pair(std::string newkey, std::string newRecord) {
	if (upper_bound()) return;
	set_key(newkey);
	set_record(newRecord);
}

int leaf::get_values_size() {
	return mRecords.size();
}

void leaf::set_next_sibling_ptr(leaf* nextPtr) {
	mNextSiblingPtr = nextPtr;
}

leaf* leaf::get_next_sibling_ptr() {
	return mNextSiblingPtr;
}

bool leaf::upper_bound() {
	return mRecords.size() == gMaxDegree - 1;
}

bool leaf::lower_bound() {
	return mRecords.size() >= gMinDegree;
}

bool leaf::is_leaf() {
	return true;
}

/// <summary>
/// Compares each string key and returns itself or NULL.
/// </summary>
leaf* leaf::parse_key(std::string key) {
	std::vector<std::string> tempKeys = get_keys();
	std::vector<std::string>::iterator it;
	int i = 0;
	for (it = tempKeys.begin(); it != tempKeys.end(); it++, i++) {
		if (compare_key(key, *it) == 0) {
			leaf* leafPtr = this;
			return leafPtr;
		}
	}
	return NULL;// means no key
}

void leaf::print() {
	std::vector<std::string> tempKeys = get_keys();
	std::vector<std::string> tempRecords = get_records();
	std::vector<std::string>::iterator keysIt = tempKeys.begin();
	std::vector<std::string>::iterator recordsIt = tempRecords.begin();
	while (recordsIt != tempRecords.end()) {
		std::cout << "key: " << *keysIt << " | " << "record: " << *recordsIt << " | ";
		keysIt++;
		recordsIt++;
	}
	std::cout << std::endl;
}

void leaf::set_record(std::string newRecord) {
	mRecords.push_back(newRecord);
}

void leaf::set_records(std::vector<std::string> newRecords) {
	mRecords = std::move(newRecords);
}

std::vector<std::string> leaf::get_records() {
	return mRecords;
}

class b_plus_tree {
public:
	// Constructor
	b_plus_tree(std::string filePath);
	void traverse_leaf();
	leaf* query(std::string key);
	void insert(std::string key, std::string record);
	void insert(leaf* leafPtr, std::string key, std::string record);
	// void delete(std::string key);
private:
	non_leaf* mRoot;
	leaf* mHeader;
	void set_root_ptr(non_leaf* newRoot);
	non_leaf* get_root_ptr();
	void set_header_ptr(leaf* newHeader);
	leaf* get_header_ptr();
	std::string trim(std::string);
};

// by default, it being sequential (next larger than cur, not same)
// initial and then inserting
b_plus_tree::b_plus_tree(std::string filePath) {
	set_header_ptr(NULL);
	set_root_ptr(NULL);

	std::vector<std::string> tempKeys;
	tempKeys.reserve(gMaxDegree - 1);
	std::vector<std::string> tempRecords;
	tempRecords.reserve(gMaxDegree - 1);

	leaf* leafPtr;

	std::ifstream infile(filePath);
	std::string line;
	int i = 0;
	for (std::string line; std::getline(infile, line); i++ ) {
		tempKeys.push_back(trim(line.substr(0, 7)));
		tempRecords.push_back(trim(line.substr(15, 80)));
		// Initials the starter root and two leaf.
		if (i == gMinDegree) {// meet the first minimum degree
			set_header_ptr(new leaf(NULL, tempKeys, tempRecords));
			tempKeys.clear();
			tempKeys.reserve(gMaxDegree - 1);
			tempRecords.clear();
			tempRecords.reserve(gMaxDegree - 1);
		}
		else if (i == gMinDegree * 2) {// meet the second minimum degree
			leafPtr = new leaf(NULL, tempKeys, tempRecords);
			get_header_ptr()->set_next_sibling_ptr(leafPtr);
			set_root_ptr(new non_leaf(leafPtr->get_first_key(), get_header_ptr(), leafPtr));
			get_header_ptr()->set_parent_ptr(get_root_ptr());
			leafPtr->set_parent_ptr(get_root_ptr());
		}
	}

	infile.close();
}

void b_plus_tree::traverse_leaf() {
	/// / test
	// non_leaf* rootPtr = get_root_ptr();

	leaf* leafPtr = get_header_ptr();
	leafPtr->print();
	while (leafPtr->get_next_sibling_ptr() != NULL) {
		leafPtr = leafPtr->get_next_sibling_ptr();
		leafPtr->print();
		/// / test
		// if (!rootPtr->is_max()) {
		// 	rootPtr->set_child_ptr(leafPtr);
		// }
	}
	/// / Test
	// rootPtr->print();
	query("AAA-377")->print();
}

// return leaf ptr if the key exists, ortherwise, NULL
leaf* b_plus_tree::query(std::string key) {
	node* nodePtr = get_root_ptr();
	node* nextPtr = nodePtr->parse_key(key);
	while (nextPtr != NULL) {
		if (nodePtr->is_leaf()) {
			// cast supertype to derived type
			return dynamic_cast<leaf*>(nodePtr->parse_key(key));
		}
		nextPtr = nodePtr->parse_key(key);
	};
	return NULL;
}

void b_plus_tree::insert(std::string key, std::string record) {
	// query
	// If null, 
	node* nodePtr = get_root_ptr();
	node* nextPtr = nodePtr->parse_key(key);
	// while (nextPtr != NULL) {
	// 	if (nodePtr->is_leaf()) {
	// 		// cast supertype to derived type
	// 		dynamic_cast<leaf*>(nodePtr->query_key(key));
	// 	}
	// 	nextPtr = nodePtr->query_key(key);
	// };
	// No leaf under root, it is a new b
	if (nextPtr == NULL) {

	}
}

void b_plus_tree::insert(leaf* leafPtr, std::string key, std::string record) {
	node* nodePtr = get_root_ptr();
	node* nextPtr = nodePtr->parse_key(key);
	if (nextPtr == NULL) {

	}
}

void b_plus_tree::set_root_ptr(non_leaf* newRoot) {
	mRoot = newRoot;
}

non_leaf* b_plus_tree::get_root_ptr() {
	return mRoot;
}

void b_plus_tree::set_header_ptr(leaf* newHeader) {
	mHeader = newHeader;
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
	b_plus_tree tree("D:\\CBVeLuMe\\Desktop\\WorkNote\\algorithm-concepts\\BPlusTree\\temp.txt");
	tree.insert("KGB-123", "");
	// tree.initial("D:\\CBVeLuMe\\Desktop\\WorkNote\\algorithm-concepts\\BPlusTree\\temp.txt");
	// tree.traverse_leaf();
}