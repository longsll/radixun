#include "radixun.h"


static radixun::Logger::ptr g_logger = RADIXUN_LOG_ROOT();

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
            //数据库查找
            rsp->setBody(req->toString());
            auto q = radixun::Query::parse(req->getBody());
            RADIXUN_LOG_INFO(g_logger) << req->getBody();
            RADIXUN_LOG_INFO(g_logger) <<  radixun::Query::Decode(q->dump());
            return 0;
    });

    server->start();
}



int main(int argc, char** argv) {
    radixun::IOManager iom(1, true, "main");
    woker.reset(new radixun::IOManager(3, false, "worker"));
    iom.schedule(run);
    return 0;
}
