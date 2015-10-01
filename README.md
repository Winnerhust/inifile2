inifile2
========

一个轻量级的inifile文件解析库

* 支持解析ini文件
* 支持修改、保存ini文件
* 支持设置多个注释符，默认为“#”和';'
* 支持参数名重复
* 使用gtest和valgrind做过测试

使用很简单 生成一个ini文件test.ini
```
>cat > test.ini
#this is commit
;this is commit
[COMMON]
DB = mysql
PASSWD=root
```

首先指定头文件和命名空间 然后使用open函数打开ini文件 getValue可以获取指定段的指定项的值

```
#include "inifile.h"
using namespace inifile;

filepath = "test.ini";

IniFile ini;
ini.load(filepath);

//获取指定段的指定项的值
int ret = 0;
string db_name = ini.getValue("COMMON","DB",ret);
```
