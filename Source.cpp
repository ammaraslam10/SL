#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

#ifdef _DEBUG
#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif

#include <stdlib.h>
#include "SL.hpp"
#include "hash_table.hpp"
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <iostream>
using namespace std;

int main(int argc, char **argv)
{
	chrono::seconds ms = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch());
	string code = "", temp;
	ifstream f;
	if (argc > 1)
		f.open(argv[1]);
	else
		f.open("main.sl");
	if (!f.is_open())
	{
		cout << "No file given. Default file is main.sl";
		exit(0);
	}
	while (!f.eof())
	{
		getline(f, temp);
		code += (temp + '\n');
	}
	f.close();

	try
	{
		SL3::SL::run(code);
	}
	catch (exception e)
	{
		cout << e.what();
	}
	cout << "\nTotal time was " << to_string((chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()) - ms).count()) << " second(s)";
	SL3::SL::clear();

	_CrtDumpMemoryLeaks();
}
