#ifndef __CBTime__
#define __CBTime__

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <cstring>
#include <vector>
#include <cassert>
#include <algorithm>
#include <sstream>
#include <map>
#include <stdexcept>
#include <list>
#include <ctime>
#include <fstream>
#include <atlcomtime.h>

using namespace std;

template <class T>
int len(T x[]) { return (sizeof(*x)/sizeof(x[0])); }// get the length of an array

template <class T> string str(T x) {
	std::stringstream ss;
	ss << x;
	return ss.str();
}


COleDateTime * ToTime(string str_time) {
	 int year, month, day, hour, minute, second, miliseconds;

	COleDateTime * t;

	if (std::sscanf(str_time.c_str(), "%2u/%2u/%4u", &month, &day, &year) != 3)
	{
		std::cout << "Parse failed" << std::endl;
		return NULL;
	}
	else
	{
		t = new COleDateTime(year, month, day,0,0,0);
	}

	return t;
};

/*tm* ToTime(string str_time ) {
	unsigned int year, month, day, hour, minute, second, miliseconds;

	time_t rawtime;
	tm * t;

	if (std::sscanf(str_time.c_str(), "%2u/%2u/%4u", &month, &day, &year) !=3 )
	{
		std::cout << "Parse failed" << std::endl;
		return NULL;
	}
	else
	{
		t = new tm;
		t->tm_year = year - 1900 ;
		t->tm_mon = month -1;
		t->tm_mday = day;
	}

	return t ;
};*/

COleDateTime * dayadd(COleDateTime* a, int days){

	*a += COleDateTimeSpan(days, 0, 0, 0);

	return a;
};

int daydiff(COleDateTime* a, COleDateTime* b){
	return -((*a)-(*b)) ;
};

double yearfrac(COleDateTime* a, COleDateTime* b){

	return -((*a)-(*b)) / (365.);
};

#endif
