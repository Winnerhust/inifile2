#ifndef _INIFILE_CPP
#define _INIFILE_CPP

#include "inifile.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

namespace inifile
{

int INI_BUF_SIZE = 2048;

//构造函数，会初始化注释字符集合flags_（容器），目前只使用#和;作为注释前缀
IniFile::IniFile()
{
    flags_.push_back("#");
    flags_.push_back(";");
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
bool IniFile::parse(const string &content, string &key, string &value, char c/*= '='*/)
{
    int i = 0;
    int len = content.length();
	//找到'='的位置
    while (i < len && content[i] != c) {
        ++i;
    }

	//通过i和len确认有key和value
    if (i >= 0 && i < len) {
        key = string(content.c_str(), i);
        value = string(content.c_str() + i + 1, len - i - 1);
        return true;
    }

    return false;
}

/*读取一行数据，返回string和长度*/
int IniFile::getline(string &str, FILE *fp)
{
    int plen = 0;
    int buf_size = INI_BUF_SIZE * sizeof(char);

    char *buf = (char *) malloc(buf_size);
    char *pbuf = NULL;
    char *p = buf;

    if (buf == NULL) {
        fprintf(stderr, "no enough memory!exit!\n");
        exit(-1);
    }

    memset(buf, 0, buf_size);
	//假定总长度为buf长度
    int total_size = buf_size;

    while (fgets(p, buf_size, fp) != NULL) {
        plen = strlen(p);

		//如果得到的字符串长度不为0 && 末尾字符不是回车符&&也没有到达文件末尾
		//则说明确实还没读够一行，需要继续分配空间，继续读行。
        if (plen > 0 && p[plen - 1] != '\n' && !feof(fp)) {

            total_size = strlen(buf) + buf_size;
            pbuf = (char *)realloc(buf, total_size);

            if (pbuf == NULL) {
                free(buf);
                fprintf(stderr, "no enough memory!exit!\n");
                exit(-1);
            }

            buf = pbuf;

            p = buf + strlen(buf);

            continue;
        } else {
            break;
        }
    }

    str = buf;

    free(buf);
    buf = NULL;
    return str.length();

}
int IniFile::load(const string &filename)
{
    release();
    fname_ = filename;
    IniSection *section = NULL;//初始化一个字段指针
    FILE *fp = fopen(filename.c_str(), "r");

    if (fp == NULL) {
        return -1;
    }

    string line;
    string comment;
    string right_comment;

    //每次读取一行内容到line
    while (getline(line, fp) > 0) {

/*
		int isLF = false;

		if(line[0]=='\n' || line[0]=='\r'){
			isLF = true;
		} else {
			isLF = false;
		}
*/

        trim(line);

		//如果长度为0，说明是空行
        if (line.length() <= 0) {
	        comment += delim;
            continue;
        }

		//step 1
		//如果行首不是注释，查找行尾是否存在注释
        if (!isComment(line)) {
            /* 针对 “value=1 #测试” 这种后面有注释的语句
             * 重新分割line，并添加注释到commnet
             * 注意：这种情况保存后会变成
             * #测试
             * value=1
             * */
            string subline;//临时行，存放从line取出的部分
            string oldline = line;

			//去掉注释，也有可能行尾没有注释，此时不会影响原数据
            for (size_t i = 0; i < flags_.size(); ++i) {
				//line.find： 找到注释位置，如果没有，返回line自身长度
				//截取从0位置到发现注释字符的位置，即只取出键名和键值，存入subline
                subline = line.substr(0, line.find(flags_[i]));
                trim(subline);
                line = subline;//已去掉注释，更新line，交给step 2处理
            }
			//从oldline取出注释内容，放入comment
			string line_comment;
			line_comment = oldline.substr(subline.length());

			if(line_comment.length())
            {
                // comment += line_comment + delim;
                right_comment += line_comment + delim;
            	//cout<<"debug: found comment after sec/key\n";
            }
        }

		//step 2
		//对line进行整理，去掉首尾的空格符
        trim(line);

        //判断line内容是否为段或键
		//段开头
        if (line[0] == '[') {

            section = NULL;
			//查找右中括号
            int index = line.find_first_of(']');

            if (index == -1) {
                fclose(fp);
                fprintf(stderr, "没有找到匹配的]\n");
                return -1;
            }
			//
            int len = index - 1;
			//若段名为空，继续下一行
            if (len <= 0) {
                fprintf(stderr, "段为空\n");
                continue;
            }
			//取段名
            string s(line, 1, len);

            trim(s);//clear blank

			//检查段是否已存在
            if (getSection(s.c_str()) != NULL) {
                fclose(fp);
                fprintf(stderr, "此段已存在:%s\n", s.c_str());
                return -1;
            }

			//申请一个新段，由于map容器会自动排序，打乱原有顺序，因此改用vector存储
            section = new IniSection();

			sections_vt.push_back(section);

			//填充段名
            section->name = s;
            printf("[%s]: section name = %s\n", __func__, s.c_str());
			//填充段开头的注释
            section->comment = comment;
            section->right_comment = right_comment;
			//comment清零
            comment = "";
            right_comment = "";

		//如果该行以注释开头
        } else if (isComment(line)) {
        	//cout<<"\nyes\n";
            comment += line + delim;
            //cout<<"comment="<<comment;

		//如果该行是键值
        } else {

        	trim(line);

            string key, value;

            if (parse(line, key, value)==true) {
                IniItem item;
                trim(key);//clear blank
                trim(value);//clear blank
                item.key = key;
                item.value = value;
                item.comment = comment;
                item.right_comment = right_comment;

                section->items.push_back(item);
            } else {
                fprintf(stderr, "解析参数失败[%s]\n", line.c_str());
            }

            comment = "";
            right_comment = "";
        }
    }

    fclose(fp);

    return 0;
}

int IniFile::save()
{
    return saveas(fname_);
}

int IniFile::saveas(const string &filename)
{
    string data = "";

	/* 载入section数据 */
    for (IniSection_it sect = sections_vt.begin(); sect != sections_vt.end(); ++sect) {
        if ((*sect)->comment != "") {
            data += (*sect)->comment;
            if(data[data.length()-1] != '\n')
           		data += delim;
        }

        if ((*sect)->name != "") {
            data += string("[") + (*sect)->name + string("]");
        }

        if ((*sect)->right_comment != "") {
            data += (*sect)->right_comment;
        }
        if(data[data.length()-1] != '\n')
            data += delim;

		/* 载入item数据 */
        for (IniSection::IniItem_it item = (*sect)->items.begin(); item != (*sect)->items.end(); ++item) {
            if (item->comment != "") {
                data += item->comment;
                if(data[data.length()-1] != '\n')
                    data += delim;
            }

            data += item->key + "=" + item->value;

            if (item->right_comment != "") {
                data += item->right_comment;
            }
            if(data[data.length()-1] != '\n')
                data += delim;
        }
    }

    FILE *fp = fopen(filename.c_str(), "w");

    fwrite(data.c_str(), 1, data.length(), fp);

    fclose(fp);

    return 0;
}

//从容器sections_map中查找section的指针位置
IniSection *IniFile::getSection(const string &section /*=""*/)
{
	for (IniSection_it it = sections_vt.begin(); it != sections_vt.end(); ++it) {
		if ((*it)->name == section) {
			return *it;
		}
	}

    return NULL;
}

string IniFile::getStringValue(const string &section, const string &key, int &ret)
{
    string value, comment;

    ret = getValue(section, key, value, comment);

    return value;
}

int IniFile::getIntValue(const string &section, const string &key, int &ret)
{
    string value, comment;

    ret = getValue(section, key, value, comment);

    return atoi(value.c_str());
}

double IniFile::getDoubleValue(const string &section, const string &key, int &ret)
{
    string value, comment;

    ret = getValue(section, key, value, comment);

    return atof(value.c_str());

}

int IniFile::getValue(const string &section, const string &key, string &value)
{
    string comment;
    return getValue(section, key, value, comment);
}
int IniFile::getValue(const string &section, const string &key, string &value, string &comment)
{
    IniSection *sect = getSection(section);

    if (sect != NULL) {
        for (IniSection::IniItem_it it = sect->begin(); it != sect->end(); ++it) {
            if (it->key == key) {
                value = it->value;
                comment = it->comment;
                return RET_OK;
            }
        }
    }

    return RET_ERR;
}
int IniFile::getValues(const string &section, const string &key, vector<string> &values)
{
    vector<string> comments;
    return getValues(section, key, values, comments);
}
int IniFile::getValues(const string &section, const string &key,
                       vector<string> &values, vector<string> &comments)
{
    string value, comment;

    values.clear();
    comments.clear();

    IniSection *sect = getSection(section);

    if (sect != NULL) {
        for (IniSection::IniItem_it it = sect->begin(); it != sect->end(); ++it) {
            if (it->key == key) {
                value = it->value;
                comment = it->comment;

                values.push_back(value);
                comments.push_back(comment);
            }
        }
    }

    return (values.size() ? RET_OK : RET_ERR);

}
bool IniFile::hasSection(const string &section)
{
    return (getSection(section) != NULL);

}

bool IniFile::hasKey(const string &section, const string &key)
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
int IniFile::getSectionComment(const string &section, string &comment)
{
    comment = "";
    IniSection *sect = getSection(section);

    if (sect != NULL) {
        comment = sect->comment;
        return RET_OK;
    }

    return RET_ERR;
}
int IniFile::setSectionComment(const string &section, const string &comment)
{
    IniSection *sect = getSection(section);

    if (sect != NULL) {
        sect->comment = comment;
        return RET_OK;
    }

    return RET_ERR;
}

int IniFile::setValue(const string &section, const string &key,
                      const string &value, const string &comment /*=""*/)
{
    IniSection *sect = getSection(section);

    string comt = comment;

    if (comt != "") {
        comt = flags_[0] + comt;
    }

    if (sect == NULL) {
		//如果段不存在
        sect = new IniSection();

        if (sect == NULL) {
            fprintf(stderr, "no enough memory!\n");
            exit(-1);
        }

        sect->name = section;
        // sections_map[section] = sect;

		sections_vt.push_back(sect);
    }

    for (IniSection::IniItem_it it = sect->begin(); it != sect->end(); ++it) {
        if (it->key == key) {
            it->value = value;
            it->comment = comt;
            return RET_OK;
        }
    }

    //not found key
    IniItem item;
    item.key = key;
    item.value = value;
    item.comment = comt;

    sect->items.push_back(item);

    return RET_OK;

}
void IniFile::getCommentFlags(vector<string> &flags)
{
    flags = flags_;
}
void IniFile::setCommentFlags(const vector<string> &flags)
{
    flags_ = flags;
}
void IniFile::deleteSection(const string &section)
{
	for (IniSection_it it = sections_vt.begin(); it != sections_vt.end(); ) {
		if ((*it)->name == section) {
			delete *it;
			it = sections_vt.erase(it);
			break;
		}
		else
			it++;
	}
}
void IniFile::deleteKey(const string &section, const string &key)
{
    IniSection *sect = getSection(section);

    if (sect != NULL) {
        for (IniSection::IniItem_it it = sect->begin(); it != sect->end(); ++it) {
            if (it->key == key) {
                sect->items.erase(it);
                break;
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
    fname_ = "";

    for (IniSection_it it = sections_vt.begin(); it != sections_vt.end(); ++it) {
		// (*it)->items.clear();//清除items容器,vector容器会自动销毁
        delete (*it);//清除section
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
bool IniFile::isComment(const string &str)
{
    bool ret = false;

    for (size_t i = 0; i < flags_.size(); ++i) {
        size_t k = 0;

        if (str.length() < flags_[i].length()) {
            continue;
        }

        for (k = 0; k < flags_[i].length(); ++k) {
            if (str[k] != flags_[i][k]) {
                break;
            }
        }

        if (k == flags_[i].length()) {
            ret = true;
            break;
        }
    }

    return ret;
}
//for debug
/*-------------------------------------------------------------------------*/
/**
  @brief    print for debug
  @param    none
  @return   none
 */
/*--------------------------------------------------------------------------*/
void IniFile::print()
{
    printf("filename:[%s]\n", fname_.c_str());

    printf("flags_:[");

    for (size_t i = 0; i < flags_.size(); ++i) {
        printf(" %s ", flags_[i].c_str());
    }

    printf("]\n");

    for (IniSection_it it = sections_vt.begin(); it != sections_vt.end(); ++it) {
        printf("section:\n[%s]\n", (*it)->name.c_str());
        printf("comment:[\n%s]\n", (*it)->comment.c_str());
        if((*it)->right_comment!="")
            printf("rcomment:\n%s", (*it)->right_comment.c_str());

        for (IniSection::IniItem_it i = (*it)->items.begin(); i != (*it)->items.end(); ++i) {
            printf("    comment:%s\n", i->comment.c_str());
            printf("    parm   :%s=%s\n", i->key.c_str(), i->value.c_str());
            if(i->right_comment!="")
                printf("    rcomment:[%s]\n", i->right_comment.c_str());
        }
    }

}

void IniFile::trimleft(string &str, char c/*=' '*/)
{
    //trim head

    int len = str.length();

    int i = 0;

    while (str[i] == c && str[i] != '\0') {
        i++;
    }

    if (i != 0) {
        str = string(str, i, len - i);
    }
}

void IniFile::trimright(string &str, char c/*=' '*/)
{
    //trim tail
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
  @param    str 一个string变量
  @return   none
 */
/*--------------------------------------------------------------------------*/
void IniFile::trim(string &str)
{
    //trim head

    int len = str.length();

    int i = 0;

    while (isspace(str[i]) && str[i] != '\0') {
        i++;
    }

    if (i != 0) {
        str = string(str, i, len - i);
    }

    //trim tail
    len = str.length();

    for (i = len - 1; i >= 0; --i) {
        if (!isspace(str[i])) {
            break;
        }
    }

    str = string(str, 0, i + 1);
}
}
#endif
