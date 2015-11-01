#include <iostream>
#include <gtest/gtest.h>
#include "inifile.h"

using namespace std;
using namespace inifile;

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

    EXPECT_EQ(section.parse(s.c_str(), key, value), true);
    EXPECT_EQ(key, "DB");
    EXPECT_EQ(value, "sys");

    s = "DB";
    key = value = "";

    EXPECT_EQ(section.parse(s.c_str(), key, value), false);
    EXPECT_EQ(key, "");
    EXPECT_EQ(value, "");

    s = "DB=";
    key = value = "";

    EXPECT_EQ(section.parse(s.c_str(), key, value), true);
    EXPECT_EQ(key, "DB");
    EXPECT_EQ(value, "");

    s = "=sys";
    key = value = "";

    EXPECT_EQ(section.parse(s.c_str(), key, value), true);
    EXPECT_EQ(key, "");
    EXPECT_EQ(value, "sys");
}



//////////////////////
TEST(IniFile, getline)
{
    //create a new ini file
    char filepath[] = "test/test.ini";
    FILE *fp = fopen(filepath, "w+");
    char content[] = " USER=root \r\n [COMMON] \n DB=sys   	\nPASSWD=tt   \n#commit   \n ;--------- \n[DEFINE] \nname=cxy\n";
    fwrite(content, 1, sizeof(content), fp);
    fclose(fp);

    //test
    IniFile ini;
    string line;
    fp = fopen(filepath, "r");

    ini.getline(line, fp);
    IniFile::trimright(line, '\n');
    IniFile::trimright(line, '\r');
    IniFile::trim(line);
    EXPECT_EQ(line, "USER=root");

    ini.getline(line, fp);
    IniFile::trimright(line, '\n');
    IniFile::trimright(line, '\r');
    IniFile::trim(line);
    EXPECT_EQ(line, "[COMMON]");

    ini.getline(line, fp);
    IniFile::trimright(line, '\n');
    IniFile::trimright(line, '\r');
    IniFile::trim(line);
    EXPECT_EQ(line, "DB=sys");

    ini.getline(line, fp);
    ini.getline(line, fp);
    IniFile::trimright(line, '\n');
    IniFile::trimright(line, '\r');
    IniFile::trim(line);
    EXPECT_EQ(line, "#commit");
    fclose(fp);
}
TEST(IniFile, hasSection_and_getValue)
{
    //create a new ini file
    char filepath[] = "test/test.ini";
    FILE *fp = fopen(filepath, "w+");
    char content[] = " USER=root \r\n [COMMON] \n DB=sys   	\nPASSWD=tt   \n#commit   \n ;--------- \n[DEFINE] \nname=cxy\nvalue=1 #测试\n";
    fwrite(content, 1, sizeof(content), fp);
    fclose(fp);

    //test
    IniFile ini;
    ini.load(filepath);
    /*cout<<"---------------\n"<<endl;
    ini.print();
    cout<<"---------------\n"<<endl;
    */
    EXPECT_EQ(ini.hasSection(string("")), true);
    EXPECT_EQ(ini.hasSection("COMMON"), true);
    EXPECT_EQ(ini.hasSection("DEFINE"), true);
    EXPECT_EQ(ini.hasSection("ss"), false);

    string value;
    EXPECT_EQ(ini.getValue("", "USER", value), 0);
    EXPECT_EQ(value, string("root"));
    EXPECT_EQ(ini.getValue("COMMON", "DB", value), 0);
    EXPECT_EQ(value, string("sys"));
    EXPECT_EQ(ini.getValue("COMMON", "PASSWD", value), 0);
    EXPECT_EQ(value, string("tt"));
    EXPECT_EQ(ini.getValue("DEFINE", "name", value), 0);
    EXPECT_EQ(value, string("cxy"));
    EXPECT_EQ(ini.getValue("DEFINE", "value", value), 0);
    EXPECT_EQ(value, string("1"));

}

TEST(IniFile, reopen)
{
    //create a new ini file
    char filepath[] = "test/test.ini";
    FILE *fp = fopen(filepath, "w");
    char content[] = " USER=root \r\n [COMMON] \n DB=sys   	\nPASSWD=tt   \n#commit   \n ;--------- \n[DEFINE] \nname=cxy\n";
    fwrite(content, sizeof(char), strlen(content), fp);
    fclose(fp);

    //test
    IniFile ini;
    ini.load(filepath);

    //reopen

    fp = fopen(filepath, "w");
    strcpy(content, " USER=root2 \r\n [COMMON] \n DB=sys2   	\n\n#commit   \n ;--------- \n\n\n");
    fwrite(content, sizeof(char), strlen(content), fp);
    fclose(fp);

    //test
    ini.load(filepath);

    EXPECT_EQ(ini.hasSection(""), true);
    EXPECT_EQ(ini.hasSection("COMMON"), true);
    EXPECT_EQ(ini.hasSection("DEFINE"), false);

    string value;
    EXPECT_EQ(ini.getValue("", "USER", value), 0);
    EXPECT_EQ(value, string("root2"));
    EXPECT_EQ(ini.getValue("COMMON", "DB", value), 0);
    EXPECT_EQ(value, string("sys2"));
    EXPECT_EQ(ini.getValue("COMMON", "PASSWD", value), -1);
    EXPECT_EQ(ini.getValue("DEFINE", "name", value), -1);

}

TEST(IniFile, saveas)
{
    //create a new ini file
    char filepath[] = "test/test.ini";
    FILE *fp = fopen(filepath, "w");
    char content[] = " USER=root \r\n [COMMON] \n DB=sys   	\nPASSWD=tt   \n#commit   \n ;--------- \n[DEFINE] \nname=cxy\nvalue=1 #测试";
    fwrite(content, sizeof(char), strlen(content), fp);
    fclose(fp);

    //test
    IniFile ini;
    ini.load(filepath);

    char saveas_path[] = "test/test_save_as.ini";

    ini.saveas(saveas_path);

    fp = fopen(saveas_path, "r");
    char buf[200];
    memset(buf, 0, 200 * sizeof(char));

    fread(buf, sizeof(char), 200, fp);
    fclose(fp);
    EXPECT_EQ(buf, string("USER=root\n[COMMON]\nDB=sys\nPASSWD=tt\n#commit\n;---------\n[DEFINE]\nname=cxy\n#测试\nvalue=1\n"));

}

TEST(IniFile, setValue)
{
    IniFile ini;

    ini.setValue("COMMON", "DB", "sys", "数据库");
    ini.setValue("COMMON", "PASSWD", "root", "数据库密码");
    ini.setValue("", "NAME", "cxy", "");

    char filepath[] = "test/test_set.ini";
    ini.saveas(filepath);

    FILE *fp = fopen(filepath, "r");
    char buf[200];
    memset(buf, 0, 200 * sizeof(char));

    fread(buf, sizeof(char), 200, fp);
    fclose(fp);
    EXPECT_EQ(buf, string("NAME=cxy\n[COMMON]\n#数据库\nDB=sys\n#数据库密码\nPASSWD=root\n"));
}
TEST(IniFile, deleteSection)
{
    //create a new ini file
    char filepath[] = "test/test.ini";
    FILE *fp = fopen(filepath, "w");
    char content[] = " USER=root \r\n [COMMON] \n DB=sys   	\nPASSWD=tt   \n#commit   \n ;--------- \n[DEFINE] \nname=cxy\n";
    fwrite(content, sizeof(char), strlen(content), fp);
    fclose(fp);

    //test
    IniFile ini;
    ini.load(filepath);

    ini.deleteSection("COMMON");

    char saveas_path[] = "test/test_save_deleteS.ini";

    ini.saveas(saveas_path);

    fp = fopen(saveas_path, "r");
    char buf[200];
    memset(buf, 0, 200 * sizeof(char));

    fread(buf, sizeof(char), 200, fp);
    fclose(fp);
    EXPECT_EQ(buf, string("USER=root\n#commit\n;---------\n[DEFINE]\nname=cxy\n"));

}
TEST(IniFile, deleteKey)
{
    //create a new ini file
    char filepath[] = "test/test.ini";
    FILE *fp = fopen(filepath, "w");
    char content[] = " USER=root \r\n [COMMON] \n DB=sys   	\nPASSWD=tt   \n#commit   \n ;--------- \n[DEFINE] \nname=cxy\n";
    fwrite(content, sizeof(char), strlen(content), fp);
    fclose(fp);

    //test
    IniFile ini;
    ini.load(filepath);

    ini.deleteKey("COMMON", "DB");

    char saveas_path[] = "test/test_save_deleteK.ini";

    ini.saveas(saveas_path);

    fp = fopen(saveas_path, "r");
    char buf[200];
    memset(buf, 0, 200 * sizeof(char));

    fread(buf, sizeof(char), 200, fp);
    fclose(fp);
    EXPECT_EQ(buf, string("USER=root\n[COMMON]\nPASSWD=tt\n#commit\n;---------\n[DEFINE]\nname=cxy\n"));

}

//#ifdef GTEST_MAIN
int main(int argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

//#endif
