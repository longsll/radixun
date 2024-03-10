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

    sd->addGlobServlet("/xx/*", [](radixun::http::HttpRequest::ptr req
                ,radixun::http::HttpResponse::ptr rsp
                ,radixun::http::HttpSession::ptr session) {
            rsp->setBody(getfile());
            return 0;
    });
    sd->addServlet("/login", [](radixun::http::HttpRequest::ptr req
                ,radixun::http::HttpResponse::ptr rsp
                ,radixun::http::HttpSession::ptr session) {
            //数据库查找
            rsp->setBody(req->toString());
            RADIXUN_LOG_INFO(g_logger) << "\n" << *req;
            auto m = req->getParams();
            RADIXUN_LOG_INFO(g_logger) << req->getBody();
            return 0;
    });

    server->start();
}

int main(int argc, char** argv) {
    radixun::IOManager iom(1, true, "main");
    woker.reset(new radixun::IOManager(3, false, "worker"));
    iom.schedule(run);

    // auto ur = radixun::Uri::Create("https://0.0.0.0:8020/user?username=aaa&password=acx");
    // if(ur != nullptr){
    //     RADIXUN_LOG_DEBUG(g_logger) << "success";
    //     RADIXUN_LOG_DEBUG(g_logger) <<"path :"<< ur->getPath();
    //     RADIXUN_LOG_DEBUG(g_logger) <<"host: "<< ur->getHost();
    //     RADIXUN_LOG_DEBUG(g_logger) << "query: "<< ur->getQuery();
    //     RADIXUN_LOG_DEBUG(g_logger) << "fg: "<< ur->getFragment();
    // }else{
    //     RADIXUN_LOG_DEBUG(g_logger) << "faiel";
    // }
    return 0;
}
