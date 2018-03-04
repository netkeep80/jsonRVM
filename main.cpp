// main.cpp: определяет точку входа для консольного приложения.
//
#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
//#include "Common.h"
#include "jsonRVM.h"


void	store_out(string& filename, json& val)
{
	std::ofstream out(filename);
	if (out.good())
		out << val.dump(3);
	else
		cerr << "Can't store object in the " << filename << " file.\n";
}


int main(int argc, char* argv[])
{/*
	json	a = "hello";
	jsonPtr	b = &a;	//	explicit
	json	c = b;
	json	d = &a;	//	implicit
	json*	e = c.get<jsonPtr>();
	json*	f = d.get<jsonPtr>();
	json*	g = (jsonPtr)d;
	
	cout << "a = " << a << "\n";
	//cout << "b = " << b << "\n";
	cout << "c = " << c << "\n";
	cout << "d = " << d << "\n";
	cout << "*e = " << *e << "\n";
	cout << "*f = " << *f << "\n";
	cout << "*g = " << *g << "\n";
	*/
	char *fileNameInput = NULL, *fileNameOutput = NULL;

	switch (argc)
	{
	case 3:
		fileNameOutput = argv[2];
	case 2:
		fileNameInput = argv[1];
		break;
	default:
		cout << "{ \"result\" : 1 }" << endl;
		return 1;
	}

	if (fileNameInput)
	{
		if (fileNameInput[0] == 0)
		{
			cout << "{ \"result\" : 1 }" << endl;
			cerr << "Error filename.\n";
			return 1;
		}
	}

	std::ifstream in(fileNameInput);
	json input_json;

	if (in.good())
		in >> input_json;
	else
		cerr << "Can't restore json object from the " << fileNameInput << " file.\n";

	// добавляем в корневую сущность базовые словари
	InitRVMDict(input_json);

	//	создаём контекст исполнения
	json	result;
	vector<string>	CallStack;
	EntView	root(input_json, CallStack);
	result = root.jsonExec(input_json);
	cout << result.dump(3);

	if (fileNameOutput)
		store_out(string(fileNameOutput), input_json);
	else
		store_out(fileNameInput + ".out.json"s, input_json);

	return 0;
}

