#include "radixun.h"

radixun::Logger::ptr g_logger = RADIXUN_LOG_ROOT();

int sock = 0;

std::string getfile(){
    std::ifstream ifs;
    std::string name = "../static/hello.html";
    radixun::FSUtil::OpenForRead(ifs , name , std::ios_base::in);
    ifs.open(name.c_str());
    std::stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}
void run() {
    radixun::http::HttpServer::ptr server(new radixun::http::HttpServer(true));
    radixun::Address::ptr addr = radixun::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while(!server->bind(addr)) {
        sleep(2);
    }
    auto sd = server->getServletDispatch();
    sd->addServlet("/xx", [](radixun::http::HttpRequest::ptr req
                ,radixun::http::HttpResponse::ptr rsp
                ,radixun::http::HttpSession::ptr session) {
            rsp->setBody(req->toString() + radixun::BacktraceToString());
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
            rsp->setBody(req->toString());
            return 0;
    });
    server->start();
}

void test_lis() {
    auto r_addr = radixun::Address::LookupAny("www.baidu.com:80");
    // auto addr = radixun::Address::LookupAny("0.0.0.0:8025");
    // radixun::http::HttpServer::ptr ser (new radixun::http::HttpServer(true));
    // radixun::TcpServer::ptr ser(new radixun::TcpServer());
    // ser->bind(addr);
    // ser->start();
    radixun::Socket::ptr ser = radixun::Socket::CreateTCPSocket();
    fcntl(ser->getSocket(), F_SETFL, O_NONBLOCK);
    RADIXUN_LOG_DEBUG(g_logger) << "conn";
    ser->connect(r_addr);
    //     if(errno == EINPROGRESS) {
    //     RADIXUN_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
    //     radixun::IOManager::GetThis()->addEvent(ser->getSocket(), radixun::IOManager::READ, [](){
    //         RADIXUN_LOG_INFO(g_logger) << "read callback----------------------------------";
    //     });     
    //    radixun::IOManager::GetThis()->addEvent(ser->getSocket(), radixun::IOManager::WRITE, [](){
    //         RADIXUN_LOG_INFO(g_logger) << "write callback---------------------------------";
    //         radixun::IOManager::GetThis()->cancelEvent(sock, radixun::IOManager::READ);    
    //     });   
    // }
    ser->close();
}

void test_con() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "36.155.132.3", &addr.sin_addr.s_addr);
    RADIXUN_LOG_DEBUG(g_logger) << "sock = " << sock;
    connect(sock, (const sockaddr*)&addr, sizeof(addr));   
      
    if(errno == EINPROGRESS) {
        RADIXUN_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        radixun::IOManager::GetThis()->addEvent(sock, radixun::IOManager::READ, [](){
            RADIXUN_LOG_INFO(g_logger) << "read callback----------------------------------";
        });     
       radixun::IOManager::GetThis()->addEvent(sock, radixun::IOManager::WRITE, [](){
            RADIXUN_LOG_INFO(g_logger) << "write callback---------------------------------";
            radixun::IOManager::GetThis()->cancelEvent(sock, radixun::IOManager::READ);    
        });   
        close(sock);
    }
}

void test_iom(){
    RADIXUN_LOG_DEBUG(g_logger) << "test";
    radixun::IOManager iom(3, false , "test_io"); 
    iom.schedule(&run);
}

radixun::Timer::ptr s_timer;
void test_timer() {
    radixun::IOManager iom(2 , true , "test_timer");
    s_timer = iom.addTimer(1000, [](){
        static int i = 0;
        RADIXUN_LOG_INFO(g_logger) << "hello timer i=" << i;
        if(++i == 5) {
            // s_timer->reset(2000, true);
            s_timer->cancel();
        }
    }, true);
}

void fun()
{
    std::cout << radixun::BacktraceToString();
}

// template <typename T>
// radixun::LexicalCast<std::string, std::vector<T>> lex();

int main()
{
    test_timer();
    // test_iom();
    // fun();
    // std::string s = "[1, 2, 3, 4]";
    // auto v = lex<int>()(s);
    // for(auto x : v){
    //     std::cout << x << " ";
    // }


    // radixun::LexicalCast<std::string, std::vector<int>> intVectorConverter;
    // // 使用实例化后的模板类
    // std::string yamlString = "[1, 2, 3, 4]";
    // std::vector<int> convertedInts = intVectorConverter(yamlString);
    // for(auto& v : convertedInts){
    //     std::cout << v << " ";
    // }
    // int k = radixun::LexicalCast<std::string , int> ()("5");
    // std::cout << k << "\n";
    return 0;

}