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
Copyright � 2016 Vertushkin Roman Pavlovich <https://vk.com/earthbirthbook>.

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
#include "jsonRVM.h"
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
	char	*fileNameInput = NULL,
			*entryPoint = NULL;

	switch (argc)
	{
	case 3:
		entryPoint = argv[2];
	case 2:
		fileNameInput = argv[1];
		break;
	default:
		cout << "          R                                                                   " << endl;
		cout << "       S__|__O                                                                " << endl;
		cout << "     O   _|_   S                                                              " << endl;
		cout << "  R__|__/_|_\\__|__R  rmvm [Version " << rmvm_version << "]                    " << endl;
		cout << "     |  \\_|_/  |     json Relations (Model) Virtual Machine                  " << endl;
		cout << "     S    |    O     https://github.com/netkeep80/jsonRVM                     " << endl;
		cout << "        __|__                                                                 " << endl;
		cout << "       /  |  \\                                                               " << endl;
		cout << "      /___|___\\                                                              " << endl;
		cout << "Fractal Triune Entity                                                         " << endl;
		cout << "                                                                              " << endl;
		cout << "Licensed under the MIT License <http://opensource.org/licenses/MIT>.          " << endl;
		cout << "Copyright (c) 2016 Vertushkin Roman Pavlovich <https://vk.com/earthbirthbook>." << endl;
		cout << "                                                                              " << endl;
		cout << "Usage:                                                                        " << endl;
		cout << "       rmvm.exe [relation_model.json] <main_entry_point>                      " << endl;
		cout << "       rmvm.exe [relation_model_library.dll]                                  " << endl;
		cout << "       rmvm.exe [rmvm.exe]                                                    " << endl;
		return 0;	//	ok
	}

	file_database_t	db(".\\");
	jsonRVM root(&db);

	size_t fileNameInputLen = strlen(fileNameInput);

	if (fileNameInputLen > 4)
	{
		char* fileNameInputExt = &fileNameInput[fileNameInputLen - 4];

		if (!strcmp(fileNameInputExt, ".dll") || !strcmp(fileNameInputExt, ".DLL"))
		{
			string	FullFileName = string(".\\") + fileNameInput;

			try
			{
				cout << "{ \"version\" : \"" << LoadAndInitDict(FullFileName, root) << "\",";
				cout << "\"" << fileNameInput << "\" : " << root.dump(3);
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
	ImportRelationsModel(root);
	ImportLoadDLLEntity(root);

	if (!strcmp(fileNameInput, "rmvm.exe"))
	{
		cout << root.dump(3);
		return 0;	//	ok
	}
	
	try
	{
		cout << root.exec(res, json(fileNameInput)).dump(2);
		return 0;	//	ok
	}
	catch (json& j)
	{
		cerr << j.dump(2);
		dump_json("rmvm.dump.json"s, root);
	}
	
	return 1;	//	error
}
