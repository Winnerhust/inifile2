#include <iostream>
#include <string>
#include <limits.h>
#include "gtest/gtest.h"
#define private   public
#define protected public

#include "inifile.h"

using std::cout;
using std::endl;

using namespace inifile;
namespace {
TEST(inifile, trim)
{
    string buf = "   aaa    ";
    IniFile::trim(buf);

    EXPECT_EQ(buf, string("aaa"));

    buf =  "   aaa";
    IniFile::trim(buf);
    EXPECT_EQ(buf, string("aaa"));

    buf =  "aaa    ";
    IniFile::trim(buf);
    EXPECT_EQ(buf, string("aaa"));
}

TEST(inifile, trimleft)
{
    string p = "   aaa    ";
    IniFile::trimleft(p);

    EXPECT_EQ(p, string("aaa    "));

    p =  "   aaa";
    IniFile::trimleft(p);
    EXPECT_EQ(p, string("aaa"));

    p = "aaa    ";
    IniFile::trimleft(p);
    EXPECT_EQ(p, string("aaa    "));
}

TEST(inifile, trimright)
{
    string p = "   aaa    ";
    IniFile::trimright(p);

    EXPECT_EQ(p, string("   aaa"));

    p =  "   aaa";
    IniFile::trimright(p);
    EXPECT_EQ(p, string("   aaa"));

    p = "aaa    ";
    IniFile::trimright(p);
    EXPECT_EQ(p, string("aaa"));
}

////////////////////////////

TEST(IniFile, pasre)
{
    IniFile section;
    string s = "DB=sys";
    string key, value;

    EXPECT_EQ(section.parse(s, &key, &value), true);
    EXPECT_EQ(key, "DB");
    EXPECT_EQ(value, "sys");

    s = "DB";
    key = value = "";

    EXPECT_EQ(section.parse(s, &key, &value), false);

    s = "DB=";
    key = value = "";

    EXPECT_EQ(section.parse(s, &key, &value), true);
    EXPECT_EQ(key, "DB");
    EXPECT_EQ(value, "");

    s = "=sys";
    key = value = "";

    EXPECT_EQ(section.parse(s, &key, &value), true);
    EXPECT_EQ(key, "");
    EXPECT_EQ(value, "sys");
}

TEST(IniFile, hasSection_and_getValue)
{
    // create a new ini file
    char filepath[] = "test/test.ini";
    FILE *fp = fopen(filepath, "w+");
    char content[] = " USER=root \r\n [COMMON] \n DB=sys   	\nPASSWD=tt   \n#commit   \n #--------- \n[DEFINE] \nname=cxy\nvalue=1\nbooltest=True #测试\n";
    fwrite(content, 1, strlen(content), fp);
    fclose(fp);

    // test
    IniFile ini;
    ini.Load(filepath);

    EXPECT_EQ(ini.HasSection(string("")), true);
    EXPECT_EQ(ini.HasSection("COMMON"), true);
    EXPECT_EQ(ini.HasSection("DEFINE"), true);
    EXPECT_EQ(ini.HasSection("ss"), false);

    string value;
    int intValue;
    bool boolValue;
    EXPECT_EQ(ini.GetStringValue("", "USER", &value), 0);
    EXPECT_EQ(value, string("root"));
    EXPECT_EQ(ini.GetStringValue("COMMON", "DB", &value), 0);
    EXPECT_EQ(value, string("sys"));
    EXPECT_EQ(ini.GetStringValue("COMMON", "PASSWD", &value), 0);
    EXPECT_EQ(value, string("tt"));
    EXPECT_EQ(ini.GetStringValue("DEFINE", "name", &value), 0);
    EXPECT_EQ(value, string("cxy"));
    EXPECT_EQ(ini.GetIntValue("DEFINE", "value", &intValue), 0);
    EXPECT_EQ(intValue, 1);
    EXPECT_EQ(ini.GetBoolValue("DEFINE", "booltest", &boolValue), 0);
    EXPECT_EQ(boolValue, true);
}

TEST(IniFile, getValue)
{
    // write config
    char filepath[] = "test/getValueTest.ini";
    FILE *fp = fopen(filepath, "w");
    char content[] = " USER=root \r\n [COMMON] \n DB=sys \nPASSWD=tt \nPASSWD=dd \n#commit \n ;--------- \n[DEFINE] \nname=cxy\nvalue=1 #test";
    fwrite(content, sizeof(char), strlen(content), fp);
    fclose(fp);

    // read config
    IniFile ini;
    ini.Load(filepath);
    string value;
    vector<string> valueList;
    ini.getValue("", "USER", &value);
    EXPECT_EQ(value, string("root"));
    ini.getValue("COMMON", "DB", &value);
    EXPECT_EQ(value, string("sys"));
    ini.GetValues("COMMON", "PASSWD", &valueList);
    EXPECT_EQ(valueList.size(), 2);
    EXPECT_EQ(valueList[0], string("tt"));
    EXPECT_EQ(valueList[1], string("dd"));
}

TEST(IniFile, reopen)
{
    // create a new ini file
    char filepath[] = "test/test.ini";
    FILE *fp = fopen(filepath, "w");
    char content[] = " USER=root \r\n [COMMON] \n DB=sys   	\nPASSWD=tt   \n#commit   \n #--------- \n[DEFINE] \nname=cxy\n";
    fwrite(content, sizeof(char), strlen(content), fp);
    fclose(fp);

    // test
    IniFile ini;
    ini.Load(filepath);

    // reopen

    fp = fopen(filepath, "w");
    strcpy(content, " USER=root2 \r\n [COMMON] \n DB=sys2   	\n\n#commit   \n #--------- \n\n\n");
    fwrite(content, sizeof(char), strlen(content), fp);
    fclose(fp);

    // test
    ini.Load(filepath);

    EXPECT_EQ(ini.HasSection(""), true);
    EXPECT_EQ(ini.HasSection("COMMON"), true);
    EXPECT_EQ(ini.HasSection("DEFINE"), false);

    string value;
    EXPECT_EQ(ini.GetStringValue("", "USER", &value), 0);
    EXPECT_EQ(value, string("root2"));
    EXPECT_EQ(ini.GetStringValue("COMMON", "DB", &value), 0);
    EXPECT_EQ(value, string("sys2"));
    EXPECT_EQ(ini.GetStringValue("COMMON", "PASSWD", &value), ERR_NOT_FOUND_KEY);
    EXPECT_EQ(ini.GetStringValue("DEFINE", "name", &value), ERR_NOT_FOUND_SECTION);
}

TEST(IniFile, SaveAs)
{
    // create a new ini file
    char filepath[] = "test/test.ini";
    FILE *fp = fopen(filepath, "w");
    char content[] = " USER=root \r\n [COMMON] \n DB=sys   	\nPASSWD=tt   \n#commit   \n #--------- \n[DEFINE] \nname=cxy\nvalue=1 #测试";
    fwrite(content, sizeof(char), strlen(content), fp);
    fclose(fp);

    IniFile ini;
    ini.Load(filepath);

    char saveas_path[] = "test/test_save_as.ini";

    ini.SaveAs(saveas_path);

    fp = fopen(saveas_path, "r");
    char buf[200];
    memset(buf, 0, 200 * sizeof(char));

    fread(buf, sizeof(char), 200, fp);
    fclose(fp);
    EXPECT_EQ(buf, string("USER=root\n[COMMON]\nDB=sys\nPASSWD=tt\n#commit\n#---------\n[DEFINE]\nname=cxy\nvalue=1 #测试\n"));
}

TEST(IniFile, setValue)
{
    IniFile ini;

    ini.setValue("COMMON", "DB", "sys", "数据库");
    ini.setValue("COMMON", "PASSWD", "root", "数据库密码");
    ini.setValue("", "NAME", "cxy", "");

    char filepath[] = "test/test_set.ini";
    ini.SaveAs(filepath);

    FILE *fp = fopen(filepath, "r");
    char buf[200];
    memset(buf, 0, 200 * sizeof(char));

    fread(buf, sizeof(char), 200, fp);
    fclose(fp);
    EXPECT_EQ(buf, string("NAME=cxy\n[COMMON]\n#数据库\nDB=sys\n#数据库密码\nPASSWD=root\n"));
}
TEST(IniFile, DeleteSection)
{
    // create a new ini file
    char filepath[] = "test/test.ini";
    FILE *fp = fopen(filepath, "w");
    char content[] = " USER=root \r\n [COMMON] \n DB=sys   	\nPASSWD=tt   \n#commit   \n #--------- \n[DEFINE] \nname=cxy\n";
    fwrite(content, sizeof(char), strlen(content), fp);
    fclose(fp);

    IniFile ini;
    ini.Load(filepath);

    ini.DeleteSection("COMMON");

    char saveas_path[] = "test/test_save_deleteS.ini";

    ini.SaveAs(saveas_path);

    fp = fopen(saveas_path, "r");
    char buf[200];
    memset(buf, 0, 200 * sizeof(char));

    fread(buf, sizeof(char), 200, fp);
    fclose(fp);
    EXPECT_EQ(buf, string("USER=root\n#commit\n#---------\n[DEFINE]\nname=cxy\n"));
}

TEST(IniFile, DeleteKey)
{
    // create a new ini file
    char filepath[] = "test/test.ini";
    FILE *fp = fopen(filepath, "w");
    char content[] = " USER=root \r\n [COMMON] \n DB=sys   	\nPASSWD=tt   \n#commit   \n #--------- \n[DEFINE] \nname=cxy\n";
    fwrite(content, sizeof(char), strlen(content), fp);
    fclose(fp);

    IniFile ini;
    ini.Load(filepath);

    ini.DeleteKey("COMMON", "DB");

    char saveas_path[] = "test/test_save_deleteK.ini";

    ini.SaveAs(saveas_path);

    fp = fopen(saveas_path, "r");
    char buf[200];
    memset(buf, 0, 200 * sizeof(char));

    fread(buf, sizeof(char), 200, fp);
    fclose(fp);
    EXPECT_EQ(buf, string("USER=root\n[COMMON]\nPASSWD=tt\n#commit\n#---------\n[DEFINE]\nname=cxy\n"));
}

}
