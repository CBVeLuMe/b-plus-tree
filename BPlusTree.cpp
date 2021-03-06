// BPlusTree.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Todo: Comments
// Todo: To delete the pointer! and use reference for minnecessary
// todo: isLeaf to template
// todo: reset the header after delete?

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
	virtual node* insert(std::string&, std::string&, node*) = 0;
	virtual void print();
private:
protected:
	node* mParentPtr;
	std::vector<std::string> mKeys;
	// Setter and Getter
	void set_key(std::string);
	void set_keys();
	void set_keys(std::vector<std::string>&);
	std::vector<std::string> get_keys();
	// Support querying and inserting
	static int compare_key(std::string, std::string);
	virtual node* split(std::vector<std::string>&, std::vector<std::string>&, std::vector<node*>&) = 0;
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

void node::print() {
	std::string nodeName = "Non-leaf " + get_first_key() + ": ";
	if (is_leaf())
		nodeName = "Leaf " + get_first_key() + ": ";
	std::string parentName = "Parent: NULL(Root) | ";
	if (get_parent_ptr() != NULL)
		parentName = "Parent: " + get_parent_ptr()->get_first_key() + " | ";
	std::cout << nodeName << parentName;
}

void node::set_key(std::string newKey) {
	mKeys.push_back(newKey);
}

void node::set_keys() {
	mKeys.clear();
	mKeys.reserve(gMaxDegree - 1);
}

void node::set_keys(std::vector<std::string>& newKeys) {
	set_keys();
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
	node* insert(std::string&, std::string&, node*);
	void print();
	//debug
	std::vector<node*> get_child_ptrs();
private:
	std::vector<node*> mChildPtrs;
	// Setter and Getter
	void set_child_ptr(node*);
	node* get_child_ptr(int);
	void set_child_ptrs();
	void set_child_ptrs(std::vector<node*>&);
	//std::vector<node*> get_child_ptrs();
	// Support querying and inserting
	non_leaf* split(std::vector<std::string>&, std::vector<node*>&);
	non_leaf* split(std::vector<std::string>&, std::vector<std::string>&, std::vector<node*>&);
};

//For root only
non_leaf::non_leaf(std::string key0, node* leafPtr0, node* leafPtr1) : node() {
	set_child_ptrs();
	set_child_ptr(leafPtr0);
	set_key(key0);
	set_child_ptr(leafPtr1);
}

non_leaf::non_leaf(node* parentPtr, std::vector<std::string> newKeys, std::vector<node*> newChildPtrs) : node(parentPtr) {
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
/// Compares each string newKey and returns a pointer to the children node.
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
/// Inserts the newKey and pointer into the corresponding position and returns parent pointer or NULL.
/// </summary>
node* non_leaf::insert(std::string& newKey, std::string& newRecord, node* newChildPtr) {
	if (newKey == "") return NULL;
	// Construct two new vectors to contain the original and new value.
	std::vector<std::string> oriKeys = get_keys();
	std::vector<node*> oriChildPtrs = get_child_ptrs();
	std::vector<std::string>::iterator keysIt = oriKeys.begin();
	std::vector<node*>::iterator childPtrsIt = oriChildPtrs.begin();
	std::vector<std::string> tempKeys;
	tempKeys.reserve(gMaxDegree);// extra one for potential splitting
	std::vector<node*> tempChildPtrs;
	tempChildPtrs.reserve(gMaxDegree + 1);// extra one for potential splitting
	// Traverse the current keys and find a suitable position for new value.
	// When inserting a new Key, it always comes with the left-side orignial pointer and the right-side new pointer.
	tempChildPtrs.push_back(*childPtrsIt);
	childPtrsIt++;
	for (keysIt; keysIt != oriKeys.end(); keysIt++, childPtrsIt++) {
		if (compare_key(newKey, *keysIt) < 0) {
			tempKeys.push_back(newKey);
			tempChildPtrs.push_back(newChildPtr);
		}
		tempKeys.push_back(*keysIt);
		tempChildPtrs.push_back(*childPtrsIt);
	}
	if (compare_key(newKey, tempKeys.back()) > 0) {
		tempKeys.push_back(newKey);
		tempChildPtrs.push_back(newChildPtr);
	}
	// If the vectors are not full, add the newRecord.
	if (tempKeys.size() < gMaxDegree) {
		set_keys(tempKeys);
		set_child_ptrs(tempChildPtrs);
		newKey = "";
		newChildPtr = NULL;
		return NULL;// NULL need to continue
	}
	// Otherwise, split the vectors.
	else {
		// Get the new newKey and child pointer for next inserting.
		newChildPtr = split(tempKeys, tempChildPtrs);
		newKey = tempKeys.back();
		if (get_parent_ptr() == NULL) {
			non_leaf* oriChildPtr = this;
			set_parent_ptr(new non_leaf(newKey, oriChildPtr, newChildPtr));
			// No new key means it has a new root
			newKey = "";
			newChildPtr = NULL;
			return get_parent_ptr();// NULL need to continue but return new root and no key
		}
		return get_parent_ptr();// continue to insert to parent
	}
}

void non_leaf::print() {
	node::print();
	std::string content = "";
	std::vector<std::string> tempKeys = get_keys();
	std::vector<node*> tempChildPtrs = get_child_ptrs();
	std::vector<std::string>::iterator keysIt = tempKeys.begin();
	std::vector<node*>::iterator childPtrsIt = tempChildPtrs.begin();
	content += "Child: " + (*childPtrsIt)->get_first_key() + " | ";
	childPtrsIt++;
	while (keysIt != tempKeys.end()) {
		content += "Key: " + *keysIt + " | " + "Child: " + (*childPtrsIt)->get_first_key() + " | ";
		keysIt++;
		childPtrsIt++;
	}
	std::cout << content << std::endl;
}

void non_leaf::set_child_ptr(node* newChildPtr) {
	if (upper_bound()) return;
	mChildPtrs.push_back(newChildPtr);
	newChildPtr->set_parent_ptr(this);
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

void non_leaf::set_child_ptrs(std::vector<node*>& newChildPtrs) {
	//mChildPtrs = std::move(newChildPtrs);
	set_child_ptrs();
	std::vector<node*>::iterator it;
	for (it = newChildPtrs.begin(); it != newChildPtrs.end(); it++) {
		set_child_ptr(*it);
		(*it)->set_parent_ptr(this);
	}
}

std::vector<node*> non_leaf::get_child_ptrs() {
	return mChildPtrs;
}

non_leaf* non_leaf::split(std::vector<std::string>& keys, std::vector<node*>& childPtrs) {
	std::vector<std::string> records;
	return split(keys, records, childPtrs);
}

// Split and return new sibling node pointer
non_leaf* non_leaf::split(std::vector<std::string>& keys, std::vector<std::string>& records, std::vector<node*>& childPtrs) {
	if (keys.size() < gMaxDegree) return NULL;
	// Re-intial the original vectors
	set_keys();
	get_child_ptrs();
	// Construct a new vectors to contain the new value.
	std::vector<std::string> newKeys;
	newKeys.reserve(gMaxDegree - 1);
	std::vector<node*> newChildPtrs;
	newChildPtrs.reserve(gMaxDegree);
	// Traverse the new vectors and split it into original and new Vectors.
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
	// Add the extra newKey and new child pointer to vectors.
	keys.push_back(tempKey);
	return newNLeafPtr;
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
	node* insert(std::string&, std::string&, node*);
	void print();
private:
	std::vector<std::string> mRecords;
	leaf* mNextSiblingPtr = NULL;
	// Setter and Getter
	void set_record(std::string);
	void set_records();
	void set_records(std::vector<std::string>);
	std::vector<std::string> get_records();
	// Support querying and inserting
	leaf* split(std::vector<std::string>&, std::vector<std::string>&);
	leaf* split(std::vector<std::string>&, std::vector<std::string>&, std::vector<node*>&);
};

leaf::leaf(node* parentPtr, std::vector<std::string> newKeys, std::vector<std::string> newRecords) : node(parentPtr) {
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
/// Compares each string newKey and returns itself or NULL.
/// </summary>
leaf* leaf::parse_key(std::string key) {
	std::vector<std::string> tempKeys = get_keys();
	std::vector<std::string>::iterator it;
	for (it = tempKeys.begin(); it != tempKeys.end(); it++) {
		if (compare_key(key, *it) == 0) {
			return this;
		}
	}
	return NULL;// means no newKey
}

node* leaf::insert(std::string& newKey, std::string& newRecord, node* newChildPtr) {
	// Construct two new vectors to contain the original and new value.
	std::vector<std::string> oriKeys = get_keys();
	std::vector<std::string> oriRecords = get_records();
	std::vector<std::string>::iterator keyIt = oriKeys.begin();
	std::vector<std::string>::iterator recordIt = oriRecords.begin();
	std::vector<std::string> tempKeys;
	tempKeys.reserve(gMaxDegree);// extra one for potential splitting
	std::vector<std::string> tempRecords;
	tempRecords.reserve(gMaxDegree);// extra one for potential splitting
	// Traverse the current keys and find a suitable position for new value.
	for (keyIt; keyIt != oriKeys.end(); keyIt++, recordIt++) {
		if (compare_key(newKey, *keyIt) < 0) {
			tempKeys.push_back(newKey);
			tempRecords.push_back(newRecord);
		}
		tempKeys.push_back(*keyIt);
		tempRecords.push_back(*recordIt);
	}
	if (compare_key(newKey, tempKeys.back()) > 0) {
		tempKeys.push_back(newKey);
		tempRecords.push_back(newRecord);
	}
	// If the vectors are not full, add the newRecord.
	if (tempKeys.size() < gMaxDegree) {
		set_keys(tempKeys);
		set_records(tempRecords);
		return NULL;// NULL need to continue
	}
	// Otherwise, split the vectors.
	else {
		// Get the new newKey and child pointer for next inserting.
		newChildPtr = split(tempKeys, tempRecords);
		newKey = tempKeys.back();
		return get_parent_ptr();// continue to insert to parent
	}
	return NULL;
}

void leaf::print() {
	node::print();
	std::string content = "";
	std::vector<std::string> tempKeys = get_keys();
	std::vector<std::string> tempRecords = get_records();
	std::vector<std::string>::iterator keysIt = tempKeys.begin();
	std::vector<std::string>::iterator recordsIt = tempRecords.begin();
	while (recordsIt != tempRecords.end()) {
		content += "Key: " + *keysIt /*<< " | " << "Record: " << *recordsIt */ + " | ";
		keysIt++;
		recordsIt++;
	}
	std::string nextSibling = "Next sibling: NULL(end)";
	if (get_next_sibling_ptr() != NULL)
		nextSibling = "Next sibling: " + get_next_sibling_ptr()->get_first_key();
	std::cout << content << nextSibling << std::endl;
}

void leaf::set_record(std::string newRecord) {
	mRecords.push_back(newRecord);
}

void leaf::set_records() {
	mRecords.clear();
	mRecords.reserve(gMaxDegree - 1);
}

void leaf::set_records(std::vector<std::string> newRecords) {
	set_records();
	mRecords = std::move(newRecords);
}

std::vector<std::string> leaf::get_records() {
	return mRecords;
}

leaf* leaf::split(std::vector<std::string>& keys, std::vector<std::string>& records) {
	std::vector<node*> childPtrs;
	return split(keys, records, childPtrs);
}

leaf* leaf::split(std::vector<std::string>& keys, std::vector<std::string>& records, std::vector<node*>& childPtrs) {
	if (keys.size() < gMaxDegree) return NULL;
	// Re-intial the original vectors
	set_keys();
	set_records();
	// Construct a new vectors to contain the new value.
	std::vector<std::string> newKeys, newRecords;
	newKeys.reserve(gMaxDegree - 1);
	newRecords.reserve(gMaxDegree - 1);
	// Traverse the new vectors and split it into original and new Vectors.
	std::vector<std::string>::iterator keysIt;
	std::vector<std::string>::iterator recordIt = records.begin();
	int i = 0;
	std::string tempKey = "";
	for (keysIt = keys.begin(); keysIt != keys.end(); keysIt++, recordIt++, i++) {
		if (i < gMinDegree) {
			set_key(*keysIt);// extra one for parent
			set_record(*recordIt);
		}
		else {
			newKeys.push_back(*keysIt);
			newRecords.push_back(*recordIt);
		}
	}
	leaf* newLeafPtr = new leaf(get_parent_ptr(), newKeys, newRecords);
	newLeafPtr->set_next_sibling_ptr(get_next_sibling_ptr());
	set_next_sibling_ptr(newLeafPtr);
	// Add the extra newKey and new child pointer to vectors.
	keys.push_back(tempKey);
	return newLeafPtr;
}

class b_plus_tree {
public:
	// Constructor
	b_plus_tree(std::string filePath);
	void traverse_leaf();
	void traverse_all();
	leaf* query(std::string key);
	void insert(std::string, leaf*);
	void insert(std::string key, std::string record);
	void insert(leaf* leafPtr, std::string key, std::string record);
	void remove(std::string key);
private:
	non_leaf* mRoot;
	leaf* mHeader;
	void set_root_ptr(non_leaf* newRoot);
	non_leaf* get_root_ptr();
	void set_header_ptr(leaf* newHeader);
	leaf* get_header_ptr();
	std::string trim(std::string);
	void reset(std::vector<std::string>&);
};

// by default, it being sequential (next larger than cur, not same)
// initial and then inserting
b_plus_tree::b_plus_tree(std::string filePath) {
	set_header_ptr(NULL);
	set_root_ptr(NULL);
	std::vector<std::string> tempKeys;
	std::vector<std::string> tempRecords;
	reset(tempKeys);
	reset(tempRecords);
	leaf* leafPtr = NULL;
	leaf* nextPtr = NULL;
	// Insert keys and records from the lines of flat-file
	std::ifstream infile(filePath);
	std::string line;
	int counter = 1;
	for (std::string line; std::getline(infile, line);) {
		tempKeys.push_back(trim(line.substr(0, 7)));
		tempRecords.push_back(trim(line.substr(15, 80)));
		if (counter == 1 && tempKeys.size() == gMinDegree) {// meet the first minimum degree
			// Initial the first minimum leaf
			leafPtr = new leaf(NULL, tempKeys, tempRecords);
			set_header_ptr(leafPtr);
			reset(tempKeys);
			reset(tempRecords);
			counter++;
		}
		else if (counter == 2 && tempKeys.size() == gMinDegree) {// meet the second minimum degree
			// Initial the second minimum leaf
			nextPtr = new leaf(NULL, tempKeys, tempRecords);
			leafPtr->set_next_sibling_ptr(nextPtr);
			// Initial the root and add children
			set_root_ptr(new non_leaf(nextPtr->get_first_key(), leafPtr, nextPtr));
			leafPtr->set_parent_ptr(get_root_ptr());
			nextPtr->set_parent_ptr(get_root_ptr());
			leafPtr = nextPtr;
			reset(tempKeys);
			reset(tempRecords);
			counter++;
		}
		else if (counter > 2 && tempKeys.size() == gMaxDegree - 1) {
			// Insert the lefted lines from leaf to non-leaf
			nextPtr = new leaf(NULL, tempKeys, tempRecords);
			leafPtr->set_next_sibling_ptr(nextPtr);
			leafPtr = nextPtr;
			insert(leafPtr->get_first_key(), leafPtr);
			reset(tempKeys);
			reset(tempRecords);
		}
	}
	// After for-loop, insert the rest lines one by one
	if (!tempKeys.empty()) {
		std::vector<std::string>::iterator keysIt = tempKeys.begin();
		std::vector<std::string>::iterator recordsIt = tempRecords.begin();
		for (keysIt; keysIt != tempKeys.end(); keysIt++, recordsIt++) {
			insert(*keysIt, *recordsIt);
		}
	}
	infile.close();
}

void b_plus_tree::reset(std::vector<std::string>& vector) {
	vector.clear();
	vector.reserve(gMaxDegree - 1);
}

void b_plus_tree::traverse_leaf() {
	std::cout << "**********debug traverse leaf**********" << std::endl;
	leaf* leafPtr = get_header_ptr();
	leafPtr->print();
	while (leafPtr->get_next_sibling_ptr() != NULL) {
		leafPtr = leafPtr->get_next_sibling_ptr();
		leafPtr->print();
	}
	std::cout << "**********debug traverse end**********" << std::endl;
}

void b_plus_tree::traverse_all() {
	std::cout << "**********debug traverse top-down**********" << std::endl;
	non_leaf* nleafPtr = get_root_ptr();
	nleafPtr->print();
	// Contains the children pointers in same depth
	std::vector<node*> tempPtrs = nleafPtr->get_child_ptrs();
	std::vector<node*> nextPtrs;
	std::vector<node*>::iterator tempIt = tempPtrs.begin();
	int depth = 1;
	while (!tempPtrs.empty()) {
		std::cout << "*****DEPTH: " << depth << "*****" << std::endl;
		while (tempIt != tempPtrs.end()) {
			(*tempIt)->print();
			if (!(*tempIt)->is_leaf()) {
				std::vector<node*> childPtrs = dynamic_cast<non_leaf*>(*tempIt)->get_child_ptrs();
				std::vector<node*>::iterator childPtrsIt;
				for (childPtrsIt = childPtrs.begin(); childPtrsIt != childPtrs.end(); childPtrsIt++) {
					nextPtrs.push_back(*childPtrsIt);
				}


			}
			else {

			}
			tempIt++;
		}
		depth++;
		tempPtrs.clear();
		tempPtrs = std::move(nextPtrs);
		tempIt = tempPtrs.begin();
	}
	std::cout << "**********debug traverse top-down end**********" << std::endl;
}

// return leaf pointer containing the key if the newKey exists, ortherwise, non_leaf ptr closed to newKey for insertion
leaf* b_plus_tree::query(std::string key) {
	node* nodePtr = get_root_ptr();
	while (!nodePtr->is_leaf()) {
		nodePtr = nodePtr->parse_key(key);
	}
	return dynamic_cast<leaf*>(nodePtr->parse_key(key));
}

//For initial, insert a leaf not only a recor
void b_plus_tree::insert(std::string newKey, leaf* newLeafPtr) {
	if (query(newKey) != NULL) return;
	std::string key = newKey;
	std::string record = "";
	node* leafPtr = newLeafPtr;
	node* nodePtr = get_root_ptr();// always be non-leaf
	node* nextPtr = get_root_ptr()->parse_key(key);// finally be leaf
	// Find the non-leaf over leaf
	while (!nextPtr->is_leaf()) {
		nodePtr = nextPtr;// assign a nleaf above the leaf
		nextPtr = nodePtr->parse_key(key);
	};
	while (nodePtr != NULL && key != "") {
		nodePtr = nodePtr->insert(key, record, leafPtr);
	}
	if (nodePtr != NULL && key == "")
		set_root_ptr(dynamic_cast<non_leaf*>(nodePtr));
}

void b_plus_tree::insert(std::string newKey, std::string newRecord) {
	//debug
	// !!!!!!! FIX THE Q AAS-312 BUG !!!!!!!! here
	std::cout << "Debug insert single key: " << newKey << std::endl;
	return;
	if (query(newKey) != NULL) return;
	std::string key = newKey;
	std::string record = newRecord;
	node* nodePtr = get_root_ptr();
	node* nextPtr = NULL;
	while (!nodePtr->is_leaf()) {
		nodePtr = nodePtr->parse_key(key);
	};
	// Get target leaf here, insert from bottom
	do {
		nextPtr = nodePtr->insert(key, record, nodePtr);
	} while (nextPtr != NULL);
}

void b_plus_tree::insert(leaf* leafPtr, std::string key, std::string record) {
	node* nodePtr = get_root_ptr();
	node* nextPtr = nodePtr->parse_key(key);
	if (nextPtr == NULL) {

	}
}

void b_plus_tree::remove(std::string key) {
	//if its null that means nothing so go back
	if (query(key) == NULL) return;
	//find the leaf
	//leaf* leafPtr = query(key);
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
	b_plus_tree tree("C:\\Users\\CBVeLuMe\\source\\repos\\ConsoleApplication2\\temp.txt");
	tree.traverse_leaf();
	tree.traverse_all();
	//tree.query("123131-077");
	//tree.query("AAA-077");
	//tree.query("AAS-237");
	std::cout << std::endl;

	//debug
	//tree.query("AAA-133")->print();
	//tree.query("AAA-459")->print();
	//tree.query("AAA-0")->print();
	//tree.query("AAA-996")->print();
	//tree.insert("KGB-123", "");
	// tree.initial("D:\\CBVeLuMe\\Desktop\\WorkNote\\algorithm-concepts\\BPlusTree\\temp.txt");
}