#include "uri_query.h"
#include <vector>
#include <regex>
#include <sstream>

namespace radixun {

    //解析

    static std::vector<std::string> split(const std::string &input, const std::string &delimiter) {
        std::vector<std::string> output;
        std::stringstream rawStream(input);
        std::string match;
        
        while (std::getline(rawStream, match, delimiter.c_str()[0]))
            output.push_back(match);

        return output;
    }

    static std::pair<std::string, std::string> parseComponent(const std::string &component) {
        auto key = component.substr(0, component.find("="));
        if(key == component)
            return std::make_pair(key, "");

        auto value = component.substr(component.find("=") + 1);
        return std::make_pair(key, value);
    }
    
    
    bool Query::contains(const std::string &key) {
        return m_parameters.find(key) != m_parameters.end();
    }

    void Query::add(const std::string &key , const std::string &value) {
        if(contains(key))return ;
        m_parameters[key] = value;
    }

    void Query::motify(const std::string &key ,const std::string &value){
        m_parameters[key] = value;
    }

    void Query::erase(const std::string &key){
        if(!contains(key))return ;
        m_parameters.erase(key);
    }
    
    int Query::getvalue(std::string& key , std::string &result){
        if(!contains(key))return -1;
        result = m_parameters[key];
        return 1;
    }
    
    Query::ptr Query::parse(const std::string& raw, const std::string &delimiter) {
        
        Query::ptr q (new Query);
        if(raw.empty())return q;
        auto components = split(raw, delimiter);

        for(auto &component : components) {
            auto result = parseComponent(component);
            q->m_parameters.emplace(result);
        }

        return q;
    }

    std::string Query::dump(const std::string &delimiter) {
        if(m_parameters.empty())
            return "";
        
        std::stringstream ss;
        bool is_fir = true;
        for(auto &it : m_parameters){
            if(!is_fir){
                ss << "&";
            }else{
                is_fir = false;
            }
            ss << it.first << '=' << it.second;
        }

        return ss.str();
    }

    //编码

    const std::string & Query::HEX_2_NUM_MAP()
    {
        static const std::string str("0123456789ABCDEF");
        return str;
    }

    const std::string & Query::ASCII_EXCEPTION()
    {
        static const std::string str(R"("%<>[\]^_`{|})");
        return str;
    }

    unsigned char Query::NUM_2_HEX(const char h, const char l)
    {
        unsigned char hh = std::find(std::begin(HEX_2_NUM_MAP()), std::end(HEX_2_NUM_MAP()), h) - std::begin(HEX_2_NUM_MAP());
        unsigned char ll = std::find(std::begin(HEX_2_NUM_MAP()), std::end(HEX_2_NUM_MAP()), l) - std::begin(HEX_2_NUM_MAP());
        return (hh << 4) + ll;
    }

    std::string Query::Encode(const std::string & url)
    {
        std::string ret;
        for (auto it = url.begin(); it != url.end(); ++it)
        {
            if (((*it >> 7) & 1) || (std::count(std::begin(ASCII_EXCEPTION()), std::end(ASCII_EXCEPTION()), *it)))
            {
                ret.push_back('%');
                ret.push_back(HEX_2_NUM_MAP()[(*it >> 4) & 0x0F]);
                ret.push_back(HEX_2_NUM_MAP()[*it & 0x0F]);
            }
            else
            {
                ret.push_back(*it);
            }
        }
        return ret;
    }

    std::string Query::Decode(const std::string & url)
    {
        std::string ret;
        for (auto it = url.begin(); it != url.end(); ++it)
        {
            if (*it == '%')
            {
                if (std::next(it++) == url.end())
                {
                    throw std::invalid_argument("url is invalid");
                }
                ret.push_back(NUM_2_HEX(*it, *std::next(it)));
                if (std::next(it++) == url.end())
                {
                    throw std::invalid_argument("url is invalid");
                }
            }
            else
            {
                ret.push_back(*it);
            }
        }
        return ret;
    }
}