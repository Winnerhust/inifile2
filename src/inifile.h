/* Copyright (c) 2014-2019 WinnerHust
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 
 * The names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef INIFILE_H_
#define INIFILE_H_

#include <vector>
#include <algorithm>
#include <string>
#include <string.h>

using std::string;
using std::vector;

namespace inifile
{
const int RET_OK  = 0;
const int RET_ERR = -1;
const char delim[] = "\n";
struct IniItem {
    string key;
    string value;
    string comment;  // 每个键的注释，都是指该行上方的内容
    string rightComment;
};

struct IniSection {
    typedef vector<IniItem>::iterator IniItem_it;  // 定义一个迭代器，即指向键元素的指针
    IniItem_it begin() {
        return items.begin();  // 段结构体的begin元素指向items的头指针
    }

    IniItem_it end() {
        return items.end();  // 段结构体的begin元素指向items的尾指针
    }

    string name;
    string comment;  // 每个段的注释，都是指该行上方的内容
    string rightComment;
    vector<IniItem> items;  // 键值项数组，一个段可以有多个键值，所有用vector来储存
};

class IniFile
{
 public:
    IniFile();
    ~IniFile() {
        release();
    }

 public:
    /* 打开并解析一个名为fname的INI文件 */
    int Load(const string &fname);
    /* 将内容保存到当前文件 */
    int Save();
    /* 将内容另存到一个名为fname的文件 */
    int SaveAs(const string &fname);

    /* 获取section段第一个键为key的string值，成功返回0，否则返回错误码 */
    int GetStringValue(const string &section, const string &key, string *value);
    /* 获取section段第一个键为key的int值，成功返回0，否则返回错误码 */
    int GetIntValue(const string &section, const string &key, int *value);
    /* 获取section段第一个键为key的double值，成功返回0，否则返回错误码 */
    int GetDoubleValue(const string &section, const string &key, double *value);
    /* 获取section段第一个键为key的bool值，成功返回0，否则返回错误码 */
    int GetBoolValue(const string &section, const string &key, bool *value);

    /* 获取section段第一个键为key的string值，成功返回获取的值，否则返回默认值 */
    void GetStringValueOrDefault(const string &section, const string &key, string *value, const string &defaultValue);
    /* 获取section段第一个键为key的int值，成功返回获取的值，否则返回默认值 */
    void GetIntValueOrDefault(const string &section, const string &key, int *value, int defaultValue);
    /* 获取section段第一个键为key的double值，成功返回获取的值，否则返回默认值 */
    void GetDoubleValueOrDefault(const string &section, const string &key, double *value, double defaultValue);
    /* 获取section段第一个键为key的bool值，成功返回获取的值，否则返回默认值 */
    void GetBoolValueOrDefault(const string &section, const string &key, bool *value, bool defaultValue);

    /* 获取section段所有键为key的值,并将值赋到values的vector中 */
    int GetValues(const string &section, const string &key, vector<string> *values);

    /* 获取section列表,并返回section个数 */
    int GetSections(vector<string> *sections);
    /* 获取section个数，至少存在一个空section */
    int GetSectionNum();
    /* 是否存在某个section */
    bool HasSection(const string &section);

    /* 是否存在某个key */
    bool HasKey(const string &section, const string &key);

    int SetStringValue(const string &section, const string &key, const string &value);
    int SetIntValue(const string &section, const string &key, int value);
    int SetDoubleValue(const string &section, const string &key, double value);
    int SetBoolValue(const string &section, const string &key, bool value);

    /*删除段*/
    void DeleteSection(const string &section);
    /*删除特定段的特定参数*/
    void DeleteKey(const string &section, const string &key);
    /*设置注释标记符列表*/
    void SetCommentHead(const string &head);
 private:
    /* 获取section段所有键为key的值,并将值赋到values的vector中,,将注释赋到comments的vector中 */
    int GetValues(const string &section, const string &key, vector<string> *value, vector<string> *comments);

    /* 同时设置值和注释 */
    int setValue(const string &section, const string &key, const string &value, const string &comment = "");

    /* 去掉str前面的c字符 */
    static void trimleft(string &str, char c = ' ');
    /* 去掉str后面的c字符 */
    static void trimright(string &str, char c = ' ');
    /* 去掉str前面和后面的空格符,Tab符等空白符 */
    static void trim(string &str);
    /* 将字符串str按分割符delim分割成多个子串 */
 private:
    IniSection *getSection(const string &section = "");
    int UpdateSection(const string &cleanLine, const string &comment,
                      const string &rightComment, IniSection **section);
    int AddKeyValuePair(const string &cleanLine, const string &comment,
                        const string &rightComment, IniSection *section);

    void release();
    bool split(const string &str, const string &sep, string *left, string *right);
    bool IsCommentLine(const string &str);
    bool parse(const string &content, string *key, string *value);
    // for debug
    void print();

 private:
    /* 获取section段第一个键为key的值,并将值赋到value中 */
    int getValue(const string &section, const string &key, string *value);
    /* 获取section段第一个键为key的值,并将值赋到value中,将注释赋到comment中 */
    int getValue(const string &section, const string &key, string *value, string *comment);

    int StringCmpIgnoreCase(const string &str1, const string &str2);
    bool StartWith(const string &str, const string &prefix);

 private:
    typedef vector<IniSection *>::iterator IniSection_it;
    vector<IniSection *> sections_vt;  // 用于存储段集合的vector容器
    string fname_;
    string commentHead;
};

}  // endof namespace inifile

#endif  // INIFILE_H_

