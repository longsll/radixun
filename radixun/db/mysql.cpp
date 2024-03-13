#include "mysql.h"
#include "../common/log.h"

namespace radixun{

static radixun::Logger::ptr g_logger = RADIXUN_LOG_NAME("system");

void mysql_db::init(){

    m_con = mysql_init(m_con);
    if (m_con == nullptr)
    {
        RADIXUN_LOG_ERROR(g_logger) << "init error";
    }
    m_con = mysql_real_connect(m_con, m_url.c_str(), m_user.c_str(), m_passWord.c_str(), m_dBName.c_str(), m_port, NULL, 0);
    if (m_con == nullptr)
    {
        RADIXUN_LOG_ERROR(g_logger) << "connect error";
    }
    RADIXUN_LOG_INFO(g_logger) << "db init finish";
}

const char* mysql_db::insert(std::string table , std::vector<std::string> key , std::vector<std::string> value){
    std::string insertsql = "INSERT INTO ";
    insertsql +="`" + table + "`";
    if(key.size() == 0 || value.size() == 0){
        return nullptr;
    }
    if(key.size() == 1){
        insertsql +="(`" + key[0] + "`)";
    }else{
        int len = key.size();
        for(int i = 0 ; i < len ; i ++){
            if(i == 0){
                insertsql +="(`" + key[i] + "`";
            }else if(i == len - 1){
                insertsql +=",`" + key[i] + "`)";
            }else{
                insertsql +=",`" + key[i] + "`";
            }
        }
    }
    insertsql += " VALUES ";
    if(key.size() == 1){
        insertsql +="('" + value[0] + "')";
    }else{
        int len = value.size();
        for(int i = 0 ; i < len ; i ++){
            if(i == 0){
                insertsql +="('" + value[i] + "'";
            }else if(i == len - 1){
                insertsql +=",'" + value[i] + "')";
            }else{
                insertsql +=",'" + value[i] + "'";
            }
        }
    }
    insertsql += ";";
    return insertsql.c_str();
}


const char* mysql_db::update(std::string table , std::map<std::string , std::string> mp , std::map<std::string , std::string> m){
    if(mp.size() != 1 || m.size() != 1)return nullptr;
    std::string insertsql = "UPDATE ";
    insertsql +="`" + table + "` SET";
    insertsql += "`" + mp.begin()->first + "`='" + mp.begin()->second + "'";
    insertsql += " WHERE ";
    insertsql += "`" + m.begin()->first + "`='" + m.begin()->second + "'";
    insertsql += ";";
    return insertsql.c_str();
}


const char* mysql_db::deldata(std::string table , std::map<std::string , std::string> mp){
    if(mp.size() != 1)return nullptr;
    std::string insertsql = "DELETE FROM ";
    insertsql +="`" + table + "`";
    insertsql += " WHERE ";
    insertsql += "`" + mp.begin()->first + "`='" + mp.begin()->second + "'";
    insertsql += ";";
    return insertsql.c_str();
}

std::string mysql_db::query_eq(std::string table , std::vector<std::string> key , std::map<std::string , std::string> m){
    if(key.size() == 0 || m.size() == 0){
        return nullptr;
    }    
    std::string insertsql = "SELECT ";
    
    if(key.size() == 1){
        insertsql +="`" + key[0] + "`";
    }else{
        int len = key.size();
        for(int i = 0 ; i < len ; i ++){
            if(i == 0){
                insertsql +="`" + key[i] + "`";
            }else{
                insertsql +=",`" + key[i] + "`";
            }
        }
    }
    insertsql += " FROM ";
    insertsql +="`" + table + "`";
    insertsql += " WHERE ";
    if(m.size() == 1){
        insertsql += "`" + m.begin()->first + "`='" + m.begin()->second + "'";
    }else{
        bool f = false;
        for(auto [x , y] : m){
            if(!f){
                f = true;
                insertsql += "`" + x + "`='" + y + "'";
            }else{
                insertsql += " AND `" + x + "`='" + y + "'";
            }
        }
    }
    insertsql += ";";
    RADIXUN_LOG_INFO(g_logger) << insertsql;
    return insertsql;
}

std::string mysql_db::do_sql(std::string s){
    RADIXUN_LOG_INFO(g_logger) << s;
    std::string res;
    mysql_query(m_con,s.c_str());
    m_res = mysql_use_result(m_con);
    if(m_res == nullptr)RADIXUN_LOG_INFO(g_logger) << "NULLPTR";
    int nums = mysql_num_fields(m_res);  //属于表结构的获取
    RADIXUN_LOG_INFO(g_logger) << nums;
    while( (m_row = mysql_fetch_row(m_res)) != nullptr)
    {
        for(int i = 0; i < nums; i++)
        {
            res += m_row[i];
        }
    }
    RADIXUN_LOG_INFO(g_logger) << res;
    return res;
}

}
