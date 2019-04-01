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

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include "inifile.h"

namespace inifile
{

// 构造函数，会初始化注释字符集合flags_（容器），目前只使用#和;作为注释前缀
IniFile::IniFile()
:commentDelimiter("#")
{
}

//解析一行数据，得到键值
/* --------------------------------------------------------------------------*/
/**
 * @brief   parse
 *
 * @param   content 数据源指针
 * @param   key     键名
 * @param   value   键值
 * @param   c       分隔符
 *
 * @return  bool
 */
/* ----------------------------------------------------------------------------*/
bool IniFile::parse(const string &content, string *key, string *value)
{
    return split(content, "=", key, value);
}

int IniFile::UpdateSection(const string &cleanLine, const string &comment,
                           const string &rightComment, IniSection **section)
{
    IniSection *newSection;
    // 查找右中括号
    size_t index = cleanLine.find_first_of(']');
    if (index == string::npos) {
        errMsg = string("no matched ] found");
        return ERR_UNMATCHED_BRACKETS;
    }

    int len = index - 1;
    // 若段名为空，继续下一行
    if (len <= 0) {
        errMsg = string("section name is empty");
        return ERR_SECTION_EMPTY;
    }

    // 取段名
    string s(cleanLine, 1, len);

    trim(s);

    //检查段是否已存在
    if (getSection(s) != NULL) {
        errMsg = string("section ") + s + string("already exist");
        return ERR_SECTION_ALREADY_EXISTS;
    }

    // 申请一个新段，由于map容器会自动排序，打乱原有顺序，因此改用vector存储（sections_vt）
    newSection = new IniSection();
    // 填充段名
    newSection->name = s;
    // 填充段开头的注释
    newSection->comment = comment;
    newSection->rightComment = rightComment;

    sections_vt.push_back(newSection);

    *section = newSection;

    return 0;
}

int IniFile::AddKeyValuePair(const string &cleanLine, const string &comment,
                             const string &rightComment, IniSection *section)
{
    string key, value;

    if (!parse(cleanLine, &key, &value)) {
        errMsg = string("parse line failed:") + cleanLine;
        return ERR_PARSE_KEY_VALUE_FAILED;
    }

    IniItem item;
    item.key = key;
    item.value = value;
    item.comment = comment;
    item.rightComment = rightComment;

    section->items.push_back(item);

    return 0;
}

int IniFile::Load(const string &filePath)
{
    int err;
    string line;  // 带注释的行
    string cleanLine;  // 去掉注释后的行
    string comment;
    string rightComment;
    IniSection *currSection = NULL;  // 初始化一个字段指针

    release();

    iniFilePath = filePath;
    std::ifstream ifs(iniFilePath);
    if (!ifs.is_open()) {
        errMsg = string("open") +iniFilePath+ string(" file failed");
        return ERR_OPEN_FILE_FAILED;
    }

    //增加默认段，即 无名段""
    currSection = new IniSection();
    currSection->name = "";
    sections_vt.push_back(currSection);

    // 每次读取一行内容到line
    while (std::getline(ifs, line)) {
        trim(line);

        // step 0，空行处理，如果长度为0，说明是空行，添加到comment，当作是注释的一部分
        if (line.length() <= 0) {
            comment += delim;
            continue;
        }

        // step 1
        // 如果行首不是注释，查找行尾是否存在注释
        // 如果该行以注释开头，添加到comment，跳过当前循环，continue
        if (IsCommentLine(line)) {
            comment += line + delim;
            continue;
        }

        // 如果行首不是注释，查找行尾是否存在注释，若存在，切割该行，将注释内容添加到rightComment
        split(line, commentDelimiter, &cleanLine, &rightComment);

        // step 2，判断line内容是否为段或键
        //段开头查找 [
        if (cleanLine[0] == '[') {
            err = UpdateSection(cleanLine, comment, rightComment, &currSection);
        } else {
            // 如果该行是键值，添加到section段的items容器
             err = AddKeyValuePair(cleanLine, comment, rightComment, currSection);
        }

        if (err != 0) {
            ifs.close();
            return err;
        }

        // comment清零
        comment = "";
        rightComment = "";
    }

    ifs.close();

    return 0;
}

int IniFile::Save()
{
    return SaveAs(iniFilePath);
}

int IniFile::SaveAs(const string &filePath)
{
    string data = "";

    /* 载入section数据 */
    for (IniSection_it sect = sections_vt.begin(); sect != sections_vt.end(); ++sect) {
        if ((*sect)->comment != "") {
            data += (*sect)->comment;
        }

        if ((*sect)->name != "") {
            data += string("[") + (*sect)->name + string("]");
            data += delim;
        }

        if ((*sect)->rightComment != "") {
            data += " " + commentDelimiter +(*sect)->rightComment;
        }

        /* 载入item数据 */
        for (IniSection::IniItem_it item = (*sect)->items.begin(); item != (*sect)->items.end(); ++item) {
            if (item->comment != "") {
                data += item->comment;
                if (data[data.length()-1] != '\n') {
                    data += delim;
                }
            }

            data += item->key + "=" + item->value;

            if (item->rightComment != "") {
                data += " " + commentDelimiter + item->rightComment;
            }

            if (data[data.length()-1] != '\n') {
                data += delim;
            }
        }
    }

    std::ofstream ofs(filePath);
    ofs << data;
    ofs.close();
    return 0;
}

IniSection *IniFile::getSection(const string &section /*=""*/)
{
    for (IniSection_it it = sections_vt.begin(); it != sections_vt.end(); ++it) {
        if ((*it)->name == section) {
            return *it;
        }
    }

    return NULL;
}

int IniFile::GetSections(vector<string> *sections)
{
    for (IniSection_it it = sections_vt.begin(); it != sections_vt.end(); ++it) {
        sections->push_back((*it)->name);
    }

    return sections->size();
}

int IniFile::GetSectionNum()
{
    return sections_vt.size();
}

int IniFile::GetStringValue(const string &section, const string &key, string *value)
{
    return getValue(section, key, value);
}

int IniFile::GetIntValue(const string &section, const string &key, int *intValue)
{
    int err;
    string strValue;

    err = getValue(section, key, &strValue);

    *intValue = atoi(strValue.c_str());

    return err;
}

int IniFile::GetDoubleValue(const string &section, const string &key, double *value)
{
    int err;
    string strValue;

    err = getValue(section, key, &strValue);

    *value = atof(strValue.c_str());

    return err;
}

int IniFile::GetBoolValue(const string &section, const string &key, bool *value)
{
    int err;
    string strValue;

    err = getValue(section, key, &strValue);

    if ((StringCmpIgnoreCase(strValue, "true") == 0) || (StringCmpIgnoreCase(strValue, "1") == 0)) {
        *value = true;
    } else if ((StringCmpIgnoreCase(strValue, "false") == 0) || (StringCmpIgnoreCase(strValue, "0") == 0)) {
        *value = false;
    }

    return err;
}

/* 获取section段第一个键为key的string值，成功返回获取的值，否则返回默认值 */
void IniFile::GetStringValueOrDefault(const string &section, const string &key,
                                      string *value, const string &defaultValue)
{
    if (GetStringValue(section, key, value) != 0) {
        *value = defaultValue;
    }

    return;
}

/* 获取section段第一个键为key的int值，成功返回获取的值，否则返回默认值 */
void IniFile::GetIntValueOrDefault(const string &section, const string &key, int *value, int defaultValue)
{
    if (GetIntValue(section, key, value) != 0) {
        *value = defaultValue;
    }

    return;
}

/* 获取section段第一个键为key的double值，成功返回获取的值，否则返回默认值 */
void IniFile::GetDoubleValueOrDefault(const string &section, const string &key, double *value, double defaultValue)
{
    if (GetDoubleValue(section, key, value) != 0) {
        *value = defaultValue;
    }

    return;
}

/* 获取section段第一个键为key的bool值，成功返回获取的值，否则返回默认值 */
void IniFile::GetBoolValueOrDefault(const string &section, const string &key, bool *value, bool defaultValue)
{
    if (GetBoolValue(section, key, value) != 0) {
        *value = defaultValue;
    }

    return;
}

/* 获取注释，如果key=""则获取段注释 */
int IniFile::GetComment(const string &section, const string &key, string *comment)
{
    IniSection *sect = getSection(section);

    if (sect == NULL) {
        errMsg = string("not find the section ")+section;
        return ERR_NOT_FOUND_SECTION;
    }

    if (key == "") {
        *comment = sect->comment;
        return RET_OK;
    }

    for (IniSection::IniItem_it it = sect->begin(); it != sect->end(); ++it) {
        if (it->key == key) {
            *comment = it->comment;
            return RET_OK;
        }
    }

    errMsg = string("not find the key ")+section;
    return ERR_NOT_FOUND_KEY;
}

/* 获取行尾注释，如果key=""则获取段的行尾注释 */
int IniFile::GetRightComment(const string &section, const string &key, string *rightComment)
{
    IniSection *sect = getSection(section);

    if (sect == NULL) {
        errMsg = string("not find the section ")+section;
        return ERR_NOT_FOUND_SECTION;
    }

    if (key == "") {
        *rightComment = sect->rightComment;
        return RET_OK;
    }

    for (IniSection::IniItem_it it = sect->begin(); it != sect->end(); ++it) {
        if (it->key == key) {
            *rightComment = it->rightComment;
            return RET_OK;
        }
    }

    errMsg = string("not find the key ")+key;
    return ERR_NOT_FOUND_KEY;
}

int IniFile::getValue(const string &section, const string &key, string *value)
{
    string comment;
    return getValue(section, key, value, &comment);
}

int IniFile::getValue(const string &section, const string &key, string *value, string *comment)
{
    IniSection *sect = getSection(section);

    if (sect == NULL) {
        errMsg = string("not find the section ")+section;
        return ERR_NOT_FOUND_SECTION;
    }

    for (IniSection::IniItem_it it = sect->begin(); it != sect->end(); ++it) {
        if (it->key == key) {
            *value = it->value;
            *comment = it->comment;
            return RET_OK;
        }
    }

    errMsg = string("not find the key ")+key;
    return ERR_NOT_FOUND_KEY;
}

int IniFile::GetValues(const string &section, const string &key, vector<string> *values)
{
    vector<string> comments;
    return GetValues(section, key, values, &comments);
}

int IniFile::GetValues(const string &section, const string &key, vector<string> *values, vector<string> *comments)
{
    string value, comment;

    values->clear();
    comments->clear();

    IniSection *sect = getSection(section);

    if (sect == NULL) {
        errMsg = string("not find the section ")+section;
        return ERR_NOT_FOUND_SECTION;
    }

    for (IniSection::IniItem_it it = sect->begin(); it != sect->end(); ++it) {
        if (it->key == key) {
            value = it->value;
            comment = it->comment;

            values->push_back(value);
            comments->push_back(comment);
        }
    }

    if (values->size() == 0) {
        errMsg = string("not find the key ")+key;
        return ERR_NOT_FOUND_KEY;
    }

    return RET_OK;
}

bool IniFile::HasSection(const string &section)
{
    return (getSection(section) != NULL);
}

bool IniFile::HasKey(const string &section, const string &key)
{
    IniSection *sect = getSection(section);

    if (sect != NULL) {
        for (IniSection::IniItem_it it = sect->begin(); it != sect->end(); ++it) {
            if (it->key == key) {
                return true;
            }
        }
    }

    return false;
}

int IniFile::setValue(const string &section, const string &key, const string &value, const string &comment /*=""*/)
{
    IniSection *sect = getSection(section);

    string comt = comment;

    if (comt != "") {
        comt = commentDelimiter + comt;
    }

    if (sect == NULL) {
        //如果段不存在，新建一个
        sect = new IniSection();

        if (sect == NULL) {
            errMsg = string("no enough memory!");
            return ERR_NO_ENOUGH_MEMORY;
        }

        sect->name = section;
        if (sect->name == "") {
            // 确保空section在第一个
            sections_vt.insert(sections_vt.begin(), sect);
        } else {
            sections_vt.push_back(sect);
        }
    }

    for (IniSection::IniItem_it it = sect->begin(); it != sect->end(); ++it) {
        if (it->key == key) {
            it->value = value;
            it->comment = comt;
            return RET_OK;
        }
    }

    // not found key
    IniItem item;
    item.key = key;
    item.value = value;
    item.comment = comt;

    sect->items.push_back(item);

    return RET_OK;
}

int IniFile::SetStringValue(const string &section, const string &key, const string &value)
{
    return setValue(section, key, value);
}

int IniFile::SetIntValue(const string &section, const string &key, int value)
{
    char buf[64] = {0};
    snprintf(buf, sizeof(buf), "%d", value);
    return setValue(section, key, buf);
}

int IniFile::SetDoubleValue(const string &section, const string &key, double value)
{
    char buf[64] = {0};
    snprintf(buf, sizeof(buf), "%f", value);
    return setValue(section, key, buf);
}

int IniFile::SetBoolValue(const string &section, const string &key, bool value)
{
    if (value) {
        return setValue(section, key, "true");
    } else {
        return setValue(section, key, "false");
    }
}

int IniFile::SetComment(const string &section, const string &key, const string &comment)
{
    IniSection *sect = getSection(section);

    if (sect == NULL) {
        errMsg = string("Not find the section ")+section;
        return ERR_NOT_FOUND_SECTION;
    }

    if (key == "") {
        sect->comment = comment;
        return RET_OK;
    }

    for (IniSection::IniItem_it it = sect->begin(); it != sect->end(); ++it) {
        if (it->key == key) {
            it->comment = comment;
            return RET_OK;
        }
    }

    errMsg = string("not find the key ")+key;
    return ERR_NOT_FOUND_KEY;
}

int IniFile::SetRightComment(const string &section, const string &key, const string &rightComment)
{
    IniSection *sect = getSection(section);

    if (sect == NULL) {
        errMsg = string("Not find the section ")+section;
        return ERR_NOT_FOUND_SECTION;
    }

    if (key == "") {
        sect->rightComment = rightComment;
        return RET_OK;
    }

    for (IniSection::IniItem_it it = sect->begin(); it != sect->end(); ++it) {
        if (it->key == key) {
            it->rightComment = rightComment;
            return RET_OK;
        }
    }

    errMsg = string("not find the key ")+key;
    return ERR_NOT_FOUND_KEY;
}

void IniFile::SetCommentDelimiter(const string &delimiter)
{
    commentDelimiter = delimiter;
}

void IniFile::DeleteSection(const string &section)
{
    for (IniSection_it it = sections_vt.begin(); it != sections_vt.end(); ) {
        if ((*it)->name == section) {
            delete *it;
            it = sections_vt.erase(it);
            break;
        } else {
            it++;
        }
    }
}

void IniFile::DeleteKey(const string &section, const string &key)
{
    IniSection *sect = getSection(section);

    if (sect != NULL) {
        for (IniSection::IniItem_it it = sect->begin(); it != sect->end();) {
            if (it->key == key) {
                it = sect->items.erase(it);
                break;
            } else {
                it++;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/
/**
  @brief    release: 释放全部资源，清空容器
  @param    none
  @return   none
 */
/*--------------------------------------------------------------------------*/
void IniFile::release()
{
    iniFilePath = "";

    for (IniSection_it it = sections_vt.begin(); it != sections_vt.end(); ++it) {
        delete (*it);  // 清除section
    }

    sections_vt.clear();
}

/*-------------------------------------------------------------------------*/
/**
  @brief    判断是否是注释
  @param    str 一个string变量
  @return   如果是注释则为真
 */
/*--------------------------------------------------------------------------*/
bool IniFile::IsCommentLine(const string &str)
{
    return StartWith(str, commentDelimiter);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    print for debug
  @param    none
  @return   none
 */
/*--------------------------------------------------------------------------*/
void IniFile::print()
{
    printf("############ print start ############\n");
    printf("filePath:[%s]\n", iniFilePath.c_str());
    printf("commentDelimiter:[%s]\n", commentDelimiter.c_str());

    for (IniSection_it it = sections_vt.begin(); it != sections_vt.end(); ++it) {
        printf("comment :[\n%s]\n", (*it)->comment.c_str());
        printf("section :\n[%s]\n", (*it)->name.c_str());
        if ((*it)->rightComment != "") {
            printf("rightComment:\n%s", (*it)->rightComment.c_str());
        }

        for (IniSection::IniItem_it i = (*it)->items.begin(); i != (*it)->items.end(); ++i) {
            printf("    comment :[\n%s]\n", i->comment.c_str());
            printf("    parm    :%s=%s\n", i->key.c_str(), i->value.c_str());
            if (i->rightComment != "") {
                printf("    rcomment:[\n%s]\n", i->rightComment.c_str());
            }
        }
    }

    printf("############ print end ############\n");
    return;
}

const string & IniFile::GetErrMsg()
{
    return errMsg;
}

bool IniFile::StartWith(const string &str, const string &prefix)
{
    if (strncmp(str.c_str(), prefix.c_str(), prefix.size()) == 0) {
        return true;
    }

    return false;
}

void IniFile::trimleft(string &str, char c /*=' '*/)
{
    int len = str.length();

    int i = 0;

    while (str[i] == c && str[i] != '\0') {
        i++;
    }

    if (i != 0) {
        str = string(str, i, len - i);
    }
}

void IniFile::trimright(string &str, char c /*=' '*/)
{
    int i = 0;
    int len = str.length();

    for (i = len - 1; i >= 0; --i) {
        if (str[i] != c) {
            break;
        }
    }

    str = string(str, 0, i + 1);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    trim，整理一行字符串，去掉首尾空格
  @param    str string变量
 */
/*--------------------------------------------------------------------------*/
void IniFile::trim(string &str)
{
    int len = str.length();

    int i = 0;

    while ((i < len) && isspace(str[i]) && (str[i] != '\0')) {
        i++;
    }

    if (i != 0) {
        str = string(str, i, len - i);
    }

    len = str.length();

    for (i = len - 1; i >= 0; --i) {
        if (!isspace(str[i])) {
            break;
        }
    }

    str = string(str, 0, i + 1);
}

/*-------------------------------------------------------------------------*/
/**
  @brief    split，用分隔符切割字符串
  @param    str 输入字符串
  @param    left_str 分隔后得到的左字符串
  @param    right_str 分隔后得到的右字符串（不包含seperator）
  @param    seperator 分隔符
  @return   pos
 */
/*--------------------------------------------------------------------------*/
bool IniFile::split(const string &str, const string &sep, string *pleft, string *pright)
{
    size_t pos = str.find(sep);
    string left, right;

    if (pos != string::npos) {
        left = string(str, 0, pos);
        right = string(str, pos+1);

        trim(left);
        trim(right);

        *pleft = left;
        *pright = right;
        return true;
    } else {
        left = str;
        right = "";

        trim(left);

        *pleft = left;
        *pright = right;
        return false;
    }
}

int IniFile::StringCmpIgnoreCase(const string &str1, const string &str2)
{
    string a = str1;
    string b = str2;
    transform(a.begin(), a.end(), a.begin(), towupper);
    transform(b.begin(), b.end(), b.begin(), towupper);

    return (a == b);
}

}  /* namespace inifile */
