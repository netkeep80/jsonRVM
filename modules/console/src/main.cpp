/*        R
	   S__|__O
	 O   _|_   S
  R__|__/_|_\__|__R  jsonRVM
	 |  \_|_/  |     json Relations (Model) Virtual Machine
	 S    |    O     https://github.com/netkeep80/jsonRVM
		__|__
	   /  |  \
	  /___|___\
Fractal Triune Entity

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
Copyright (c) 2016-2021 Vertushkin Roman Pavlovich <https://vk.com/earthbirthbook>.

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
// main.cpp: ���������� ����� ����� ��� ����������� ����������.
//
#include <iostream>
#include <fstream>
#include <string>
#include "vm.rm.h"
#include "base.rm.h"
#include "dll.rm.h"
#include "file_database.h"

using namespace rm;

void	dump_json(string& filename, json& res)
{
	std::ofstream out(filename);
	if (out.good())
		out << res.dump();
	else
		cerr << "Can't store object in the " << filename << " file.\n";
}


int main(int argc, char* argv[])
{
	json	res;
	char	*entry_point = NULL;

	switch (argc)
	{
	case 2:
		entry_point = argv[1];
		break;
	default:
		cout << "          R                                                                   " << endl;
		cout << "       S__|__O                                                                " << endl;
		cout << "     O   _|_   S                                                              " << endl;
		cout << "  R__|__/_|_\\__|__R  rmvm [Version " << vm::version << "]                    " << endl;
		cout << "     |  \\_|_/  |     json Relations (Model) Virtual Machine                  " << endl;
		cout << "     S    |    O     https://github.com/netkeep80/jsonRVM                     " << endl;
		cout << "        __|__                                                                 " << endl;
		cout << "       /  |  \\                                                               " << endl;
		cout << "      /___|___\\                                                              " << endl;
		cout << "Fractal Triune Entity                                                         " << endl;
		cout << "                                                                              " << endl;
		cout << "Licensed under the MIT License <http://opensource.org/licenses/MIT>.          " << endl;
		cout << "Copyright (c) 2016-2021 Vertushkin Roman Pavlovich <https://vk.com/earthbirthbook>." << endl;
		cout << "                                                                              " << endl;
		cout << "Usage:                                                                        " << endl;
		cout << "       rmvm.exe [entry_point]                                                 " << endl;
		cout << "       rmvm.exe [relation_model_library.dll]                                  " << endl;
		cout << "       rmvm.exe [rmvm.exe]                                                    " << endl;
		return 0;	//	ok
	}

	file_database_t	db(".\\");
	vm root(&db);

	size_t fileNameInputLen = strlen(entry_point);

	if (fileNameInputLen > 4)
	{
		char* fileNameInputExt = &entry_point[fileNameInputLen - 4];

		if (!strcmp(fileNameInputExt, ".dll") || !strcmp(fileNameInputExt, ".DLL"))
		{
			string	FullFileName = string(".\\") + entry_point;

			try
			{
				cout << "{ \"version\" : \"" << LoadAndInitDict(FullFileName, root) << "\",";
				cout << "\"" << entry_point << "\" : " << root.dump(3);
				cout << "}";
				return 0;	//	ok
			}
			catch (string& error)
			{
				cerr << "Exception: " << error;
			}
			catch (std::exception& e)
			{
				cerr << "Exception: " << e.what();
			}
			catch (...)
			{
				cerr << "Unknown exception";
			}
			return 1;	//	error
		}
	}

	//	base vocabulary
	import_relations_model_to(root);
	ImportLoadDLLEntity(root);

	if (!strcmp(entry_point, "rmvm.exe"))
	{
		cout << root.dump(3);
		return 0;	//	ok
	}
	
	try
	{
		cout << root.objectify(json(entry_point), res).dump(2);
		return 0;	//	ok
	}
	catch (json& j)
	{
		cerr << j.dump(2);
		dump_json("rmvm.dump.json"s, root);
	}
	
	return 1;	//	error
}
