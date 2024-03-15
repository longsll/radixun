#include "radixun.h"

static radixun::Logger::ptr g_logger = RADIXUN_LOG_ROOT();
#define XX(...) #__VA_ARGS__

std::string getfile(){
    std::ifstream ifs;
    std::string name = "../static/hello.html";
    radixun::FSUtil::OpenForRead(ifs , name , std::ios_base::in);
    ifs.open(name.c_str());
    std::stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}


radixun::IOManager::ptr woker;
radixun::mysql_db::ptr sq;
void run() {
    
    g_logger->setLevel(radixun::LogLevel::INFO);
    radixun::http::HttpServer::ptr server(new radixun::http::HttpServer(true));
    radixun::Address::ptr addr = radixun::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while(!server->bind(addr)) {
        sleep(2);
    }
    auto sd = server->getServletDispatch();
    sd->addServlet("/xx", [](radixun::http::HttpRequest::ptr req
                ,radixun::http::HttpResponse::ptr rsp
                ,radixun::http::HttpSession::ptr session) {
            rsp->setBody(req->toString());
            return 0;
    });

    sd->addGlobServlet("/login", [](radixun::http::HttpRequest::ptr req
                ,radixun::http::HttpResponse::ptr rsp
                ,radixun::http::HttpSession::ptr session) {
            rsp->setBody(getfile());
            return 0;
    });

    sd->addServlet("/user", [](radixun::http::HttpRequest::ptr req
                ,radixun::http::HttpResponse::ptr rsp
                ,radixun::http::HttpSession::ptr session) {
            //search
            std::string pon;
            auto q = radixun::Query::parse(req->getBody());
            sq->setTable("users");
            std::vector<std::string> key;
            key.push_back("use");
            key.push_back("pwd");
            auto qq = radixun::Query::parse(radixun::Query::Decode(q->dump()));
            pon = sq->do_sql(sq->query_eq(sq->getTable() , key , qq->getmap()));
            if(pon.size() == 0)pon = "no user";
            rsp->setBody(pon);
            return 0;
    });

    server->start();


}



int main(int argc, char** argv) {
    sq.reset(new radixun::mysql_db("localhost" , 3306 , "root" , "123456" , "test_databace"));
    radixun::IOManager iom(1, true, "main");
    woker.reset(new radixun::IOManager(3, false, "worker"));
    iom.schedule(run);
    return 0;
}
