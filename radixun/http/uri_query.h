#ifndef __RADIXUN_URI_PARSER_H__
#define __RADIXUN_URI_PARSER_H__

#include <map>
#include <vector>
#include <string>
#include <memory>

namespace radixun {

    class Query {
        public:
            typedef std::shared_ptr<Query> ptr;

            static Query::ptr parse(const std::string &raw, const std::string &delimiter = "&");
            Query(){};
            Query(std::map<std::string, std::string> parameter , std::vector<std::string> key):m_parameters(parameter){}
            
            void add(const std::string &key ,const std::string &value);
            void motify(const std::string &key ,const std::string &value);
            bool contains(const std::string &key);
            void erase(const std::string &key);
            int getvalue(std::string& key , std::string &result);
            std::map<std::string, std::string> getmap(){return m_parameters;}
            std::string dump(const std::string &delimiter = "&");
            //转为URL编码
            static std::string Encode(const std::string & url);
            //从url编码转
            static std::string Decode(const std::string & url);
            
        private:
            static const std::string & HEX_2_NUM_MAP();
            static const std::string & ASCII_EXCEPTION();
            static unsigned char NUM_2_HEX(const char h, const char l);
            std::map<std::string, std::string> m_parameters;
    };

}

#endif