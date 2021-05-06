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
#include <iostream>
#include <fstream>
#include <string>
#include "jsonRVM.h"
#include "base.rm.h"
#include "file_database.h"

using namespace rm;

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
	json	val;
	char	*fileNameInput = NULL,
			*entryPoint = NULL;	//	вторым аргументом должна идти текущая проекция сущности, т.е. её состояние, содержимое локального адресного пространства

	switch (argc)
	{
	case 3:
		entryPoint = argv[2];
	case 2:
		fileNameInput = argv[1];
		break;
	default:
		cout << "          c                                                                   " << endl;
		cout << "       v__|__m                                                                " << endl;
		cout << "     m   _|_   v                                                              " << endl;
		cout << "  c__|__/_|_\\__|__c  trmvm [Version " << RVM_version << "]                   " << endl;
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
		cout << "Usage:                                                                        " << endl;
		cout << "       trmvm.exe [relation_model.json] <main_entry_point>                     " << endl;
		cout << "       trmvm.exe [relation_model_library.dll]                                 " << endl;
		return 0;	//	ok
	}

	if (!strcmp(fileNameInput, "trmvm.exe"))
	{
		cout << jsonRVM().dump(3);
		return 0;	//	ok
	}

	file_database_t	db(".\\");
	jsonRVM root(&db);
	
	try
	{
		//	создаём контекст исполнения
		EntContext ctx(val, root[""]);

		try
		{
			std::ifstream in(fileNameInput);

			if (in.good())
				in >> root[""];
			else
				throw json("Can't restore RM json from the "s + fileNameInput + " file"s);
			 	
			if (entryPoint) root.JSONExec(ctx, json(entryPoint));
			else root.JSONExec(ctx, root[""]);

			cout << val.dump(2);

			return 0;	//	ok
		}
		catch (json& j) { ctx.throw_json(__FUNCTION__, j ); }
		catch (json::exception& e) { ctx.throw_json(__FUNCTION__, "json::exception: "s + e.what() + ", id: "s + to_string(e.id) ); }
		catch (std::exception& e) { ctx.throw_json(__FUNCTION__, "std::exception: "s + e.what() ); }
		catch (...) { ctx.throw_json(__FUNCTION__, "unknown exception"s ); }
	}
	catch (json& j)
	{
		cerr << j.dump(2);
		dump_json("trmvm.dump.json"s, root);
	}
	
	return 1;	//	error
}
