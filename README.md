inifile2
========

## 一个轻量级的inifile文件解析库

### 一. 使用介绍
* 支持解析ini文件
* 支持修改、保存ini文件
* 支持设置多个注释符，默认为“#”和';'
* 支持参数名重复
* 使用gtest和valgrind做过测试

使用很简单，生成一个ini文件test.ini
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

### 二. 本地测试构建方式

* 构建: `./build.sh`
* 清理: `./build.sh clean`
* 帮助: `./build.sh -h` 或者`./build.sh --help`

```
$./build.sh --help
usage: ./build.sh [clean] [-h|--help]
```

### 三. 如何引用inifile库

1. 源码引用:

编译时将src目录下的inifile.cpp 和inifile.h 拷贝到您的源码中直接进行编译即可,对于编译选项或者编译器有要求的建议使用这种源码拷贝的方式.

2. 二进制库引用:libinifile.so或者libinifile.a 引用

执行`./build.sh`后在build/lib目录下会生成libinifile.so和libinifile.a,您可以将这两个库添加到您的工程中，对应的头文件为inifile.h.
