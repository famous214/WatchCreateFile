#pragma once
#include <iostream>
#include <string>
using namespace std;



#ifdef _UNICODE
typedef wstring tstring;
#define tcout	wcout
#define tcin	wcin
#else
typedef string tstring;
#define tcout	cout
#define tcin	cin
#endif
