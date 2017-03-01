#include "curl/ICurl.h"

#include <iostream>

using namespace std;

int main()
{
	ICurl *curl = ICurl::GetInstance();
	string s;
	curl->Get("http://www.baidu.com", s);
	cout << s << endl;
	getchar();
	return 0;
}