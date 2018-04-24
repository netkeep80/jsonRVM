/*        c
       v__|__m
     m   _|_   v
  c__|__/_|_\__|__c  jsonRVM
     |  \_|_/  |     json Relations (Model) Virtual Machine
     v    |    m     https://github.com/netkeep80/jsonRVM
        __|__
       /  |  \
      /___|___\
Fractal Triune Entity

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
Copyright © 2016 Vertushkin Roman Pavlovich <https://vk.com/earthbirthbook>.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
// main.cpp: определяет точку входа для консольного приложения.
//
#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include "jsonRVM.h"


void	dump_json(string& filename, json& val)
{
	std::ofstream out(filename);
	if (out.good())
		out << val.dump();
	else
		cerr << "Can't store object in the " << filename << " file.\n";
}


int main(int argc, char* argv[])
{
	char *fileNameInput = NULL, *fileNameOutput = NULL;

	switch (argc)
	{
	case 3:
		fileNameOutput = argv[2];
	case 2:
		fileNameInput = argv[1];
		break;
	default:
		cout << "          c                                                                   " << endl;
		cout << "       v__|__m                                                                " << endl;
		cout << "     m   _|_   v                                                              " << endl;
		cout << "  c__|__/_|_\\__|__c  jsonRVM                                                 " << endl;
		cout << "     |  \\_|_/  |     json Relations (Model) Virtual Machine                  " << endl;
		cout << "     v    |    m     https://github.com/netkeep80/jsonRVM                     " << endl;
		cout << "        __|__                                                                 " << endl;
		cout << "       /  |  \\                                                               " << endl;
		cout << "      /___|___\\                                                              " << endl;
		cout << "Fractal Triune Entity                                                         " << endl;
		cout << "                                                                              " << endl;
		cout << "Licensed under the MIT License <http://opensource.org/licenses/MIT>.          " << endl;
		cout << "Copyright (c) 2016 Vertushkin Roman Pavlovich <https://vk.com/earthbirthbook>." << endl;
		cout << "                                                                              " << endl;
		cout << "usage: jsonRVM.exe [input RM json file [output RM json file]]                 " << endl;
		return 1;
	}

	std::ifstream in(fileNameInput);
	json input_json;

	if (in.good())
		in >> input_json;
	else
		cerr << "Can't restore RM json from the " << fileNameInput << " file.\n";

	// добавляем в корневую сущность базовый словарь
	ImportRelationsModel(input_json);

	//	создаём контекст исполнения
	json	returnValue;
	input_json["CallStack"] = json::array();

	json	root;
	InitCtx(root, input_json, returnValue, root);
	ExecEntity(root, input_json, returnValue);

	if (fileNameOutput)
		dump_json(string(fileNameOutput), input_json);

	try
	{
		return returnValue.get<int>();
	}
	catch (...)
	{
		cout << returnValue.dump(3);
		return 1;
	}
}

