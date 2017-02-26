#pragma once
#include <iostream>
#include <stdlib.h>
#include <string>
#include "include/sqlite3.h"

#pragma comment(lib,"../lib/sqlite3.lib")
using namespace std;

class db
{
public:
	typedef int (*callback)(void *data, int argc, char **argv, char **azColName);
	static int Open(const char* dbfile);
	static void Close();
	static void Exec(const char* sqlcommand,callback cb);
	
private:
	static sqlite3* sqldb;
};

