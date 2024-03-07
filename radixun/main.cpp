#include "radixun.h"

static radixun::Logger::ptr g_logger = RADIXUN_LOG_ROOT();

#define XX(...) #__VA_ARGS__


radixun::IOManager::ptr woker;
void run() {
    g_logger->setLevel(radixun::LogLevel::INFO);
    //radixun::http::HttpServer::ptr server(new radixun::http::HttpServer(true, woker.get(), radixun::IOManager::GetThis()));
    radixun::http::HttpServer::ptr server(new radixun::http::HttpServer(true));
    radixun::Address::ptr addr = radixun::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while(!server->bind(addr)) {
        sleep(2);
    }
    auto sd = server->getServletDispatch();
    sd->addServlet("/radixun/xx", [](radixun::http::HttpRequest::ptr req
                ,radixun::http::HttpResponse::ptr rsp
                ,radixun::http::HttpSession::ptr session) {
            rsp->setBody(req->toString());
            return 0;
    });

    sd->addGlobServlet("/radixun/*", [](radixun::http::HttpRequest::ptr req
                ,radixun::http::HttpResponse::ptr rsp
                ,radixun::http::HttpSession::ptr session) {
            rsp->setBody("Glob:\r\n" + req->toString());
            return 0;
    });

    sd->addGlobServlet("/*", [](radixun::http::HttpRequest::ptr req
                ,radixun::http::HttpResponse::ptr rsp
                ,radixun::http::HttpSession::ptr session) {
            rsp->setBody(XX(<html>
            <head><title>404 Not Found</title></head>
            <body>
            <center><h1>404 Not Found</h1></center>
            <hr><center>nginx/1.16.0</center>
            </body>
            </html>
            <!-- a padding to disable MSIE and Chrome friendly error page -->
            <!-- a padding to disable MSIE and Chrome friendly error page -->
            <!-- a padding to disable MSIE and Chrome friendly error page -->
            <!-- a padding to disable MSIE and Chrome friendly error page -->
            <!-- a padding to disable MSIE and Chrome friendly error page -->
            <!-- a padding to disable MSIE and Chrome friendly error page -->
            ));
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
