#include "db.h"


sqlite3* db::sqldb = NULL;

int db::Open(const char* dbfile)
{
	return sqlite3_open(dbfile, &sqldb);
}

void db::Close()
{
	sqlite3_close(sqldb);
}

void db::Exec(const char* sqlcommand, callback cb)
{
	sqlite3_exec(sqldb, sqlcommand, cb, NULL, NULL);
}