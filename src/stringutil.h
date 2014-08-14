#ifndef _STRINGUTIL_H
#define _STRINGUTIL_H

#include<string>
using namespace std;

namespace stringutil
{
	/*去掉str后面的c字符*/
	void trimleft(string &str,char c=' ');
	/*去掉str前面的c字符*/
	void trimright(string &str,char c=' ');
	/*去掉str前面和后面的空格符,Tab符等空白符*/
	void trim(string &str);
	/*将字符串str按分割符delim分割成多个子串*/
}

#endif
