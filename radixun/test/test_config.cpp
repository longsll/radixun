#include "log.h"
#include "config.h"
#include <iostream>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <assert.h>

radixun::ConfigVar<int>::ptr g_int_value_config = 
    radixun::Config::Lookup("system.port" , (int)8080 , "system port");
radixun::ConfigVar<std::vector<int>>::ptr g_vec_value_config = 
    radixun::Config::Lookup("system.int_vec" , std::vector<int>{1,2,3} , "system vec");
radixun::ConfigVar<std::list<int>>::ptr g_list_value_config = 
    radixun::Config::Lookup("system.int_list" , std::list<int>{1,2,3} , "system list");
radixun::ConfigVar<std::set<int>>::ptr g_set_value_config = 
    radixun::Config::Lookup("system.int_set" , std::set<int>{1,2,3} , "system set");
radixun::ConfigVar<std::unordered_set<int>>::ptr g_uset_value_config = 
    radixun::Config::Lookup("system.int_uset" , std::unordered_set<int>{1,2,3} , "system uset");
radixun::ConfigVar<std::map<std::string, int> >::ptr g_map_value_config = 
    radixun::Config::Lookup("system.int_map" , std::map<std::string, int>{{"k",2},{"a",3}} , "system map");
radixun::ConfigVar<std::unordered_map<std::string, int> >::ptr g_umap_value_config = 
    radixun::Config::Lookup("system.int_umap" , std::unordered_map<std::string, int>{{"k",2}} , "system umap");

void test_yaml() {
    YAML::Node root = YAML::LoadFile("./test.yaml");
    RADIXUN_LOG_INFO(RADIXUN_LOG_ROOT()) << root["test"].IsDefined();
    RADIXUN_LOG_INFO(RADIXUN_LOG_ROOT()) << root;
}

void test_config() {

    RADIXUN_LOG_INFO(RADIXUN_LOG_ROOT()) << "before: " << g_int_value_config->getValue();
    RADIXUN_LOG_INFO(RADIXUN_LOG_ROOT()) << "before: " << g_int_value_config->toString();

    
    auto v = g_set_value_config->getValue();
    for(auto& i : v){
        RADIXUN_LOG_INFO(RADIXUN_LOG_ROOT()) << "before-->: " << i;
    }
    RADIXUN_LOG_INFO(RADIXUN_LOG_ROOT()) << "before string: " << g_set_value_config->toString();
    //
    YAML::Node root = YAML::LoadFile("./test.yaml");
    radixun::Config::LoadFromYaml(root);
    v = g_set_value_config->getValue();
    for(auto& i : v){
        RADIXUN_LOG_INFO(RADIXUN_LOG_ROOT()) << "after: " << i;
    }
    // RADIXUN_LOG_INFO(RADIXUN_LOG_ROOT()) << root;
}

class Person {
public:
    Person() {};
    std::string m_name;
    int m_age = 0;
    bool m_sex = 0;

    std::string toString() const {
        std::stringstream ss;
        ss << "[Person name=" << m_name
           << " age=" << m_age
           << " sex=" << m_sex
           << "]";
        return ss.str();
    }

    bool operator==(const Person& oth) const {
        return m_name == oth.m_name
            && m_age == oth.m_age
            && m_sex == oth.m_sex;
    }
};

namespace radixun{

template<>
class LexicalCast<std::string, Person> {
public:
    Person operator() (const std::string& v){
        YAML::Node node = YAML::Load(v);
        Person p;
        p.m_name = node["name"].as<std::string>();
        p.m_age = node["age"].as<int>();
        p.m_sex = node["sex"].as<bool>();
        return p;
    }
};

template<>
class LexicalCast<Person, std::string> {
public:
    std::string operator()(const Person& p) {
        YAML::Node node;
        node["name"] = p.m_name;
        node["age"] = p.m_age;
        node["sex"] = p.m_sex;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

}


radixun::ConfigVar<Person>::ptr g_person = 
    radixun::Config::Lookup("class.person", Person(), "system person");

void test_class()
{
    RADIXUN_LOG_INFO(RADIXUN_LOG_ROOT()) << "before: " << g_person->getValue().toString() << " - " << g_person->toString();
    // std::cout<< g_person->getValue().toString() << " - " << g_person->toString()<<std::endl;
    YAML::Node root = YAML::LoadFile("./test.yaml");
    radixun::Config::LoadFromYaml(root);
    RADIXUN_LOG_INFO(RADIXUN_LOG_ROOT()) << "after: " << g_person->getValue().toString() << " - " << g_person->toString();
}

int main()
{
    // test_config();
    // test_class();
    // radixun::Config::Visit([](radixun::ConfigVarBase::ptr var) {
    //     RADIXUN_LOG_INFO(RADIXUN_LOG_ROOT()) << "name=" << var->getName()
    //                 << " description=" << var->getDescription()
    //                 << " typename=" << var->getTypeName()
    //                 << " value=" << var->toString();
    // });
    return 0;
}