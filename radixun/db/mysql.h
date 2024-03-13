#ifndef __RADIXUN_MYSQL_H__
#define __RADIUXN_MYSQL_H__


#include "db.h"

namespace radixun{

class mysql_db{
public:
    typedef std::shared_ptr<mysql_db> ptr;

    mysql_db(std::string url , unsigned int port , std::string user , std::string pwd , std::string db_name)
    :m_url(url), m_port(port), m_passWord(pwd) ,m_dBName(db_name) 
    {
        init();
    }
    ~mysql_db(){
        if(m_con != nullptr)mysql_close(m_con);
        if(m_res != nullptr)mysql_free_result(m_res);
    }
    //init
    void init();
    //choose table
    void setTable(std::string table){m_table = table;}
    std::string getTable(){return m_table;}
    //插入数据
    //INSERT INTO 表名([字段1,字段2..])VALUES('值1','值2'..),[('值1','值2'..)..];
    //table:  `student`  key (`name`,`pwd`,`sex`) value ('zsr','200024','男'), ('gcc','000421','女');
    const char* insert(std::string table, std::vector<std::string> key , std::vector<std::string> value);
    // table 表名 map 字段1=值1,[字段2=值2...];
    //UPDATE 表名 SET 字段1=值1,[字段2=值2...] WHERE 条件[];
    const char* update(std::string table , std::map<std::string , std::string> mp , std::map<std::string , std::string> m);
    //DELETE FROM `student` WHERE id=1;
    const char* deldata(std::string table , std::map<std::string , std::string> mp);

    std::string query_eq(std::string table , std::vector<std::string> key , std::map<std::string , std::string> m);

    std::string do_sql(std::string s);

private: 
    MYSQL * m_con;
    MYSQL_RES *m_res;
    MYSQL_ROW m_row;

    std::string m_table;
    std::string m_url;			 //主机地址
    unsigned int m_port;		 //数据库端口号
    std::string m_user;		 //登陆数据库用户名
    std::string m_passWord;	 //登陆数据库密码
    std::string m_dBName; //使用数据库名
};

}

#endif