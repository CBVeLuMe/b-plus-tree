#include "pch.h"

using namespace System;

//int main(array<System::String ^> ^args)
//{
//    return 0;
//}

#include "Form1.h"

using namespace System::Windows::Forms;

[STAThread]
int main() {
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);
	Application::Run(gcnew CppCLRWinformsProjekt::Form1()); 
	return 0;
	b_plus_tree tree("C:\\Users\\mhenriques\\source\\repos\\bPlussTreeProject\\bPlussTreeProject\\partfile.txt");
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