#ifndef __RADIXUN_DB_H__
#define __RADIXUN_DB_H__

#include <memory>
#include <string>
#include <mysql/mysql.h>
#include <error.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
namespace radixun {
class DB{
public:
    typedef std::shared_ptr<DB> ptr;
    //插入数据
    //INSERT INTO 表名([字段1,字段2..])VALUES('值1','值2'..),[('值1','值2'..)..];
    //table:  `student`  key (`name`,`pwd`,`sex`) value ('zsr','200024','男'), ('gcc','000421','女');
    void insert(std::string table , std::vector<std::string> key , std::vector<std::string> value);
    // table 表名 map 字段1=值1,[字段2=值2...];
    //UPDATE 表名 SET 字段1=值1,[字段2=值2...] WHERE 条件[];
    void update(std::string table , std::map<std::string , std::string> mp);
    //DELETE FROM `student` WHERE id=1;
    void deldata(std::string table , std::map<std::string , std::string> mp);

    void query_eq(std::string table , std::vector<std::string> key , std::map<std::string , std::string> m);
};
}

#endif