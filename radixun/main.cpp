#include "radixun.h"


radixun::Logger::ptr logger = RADIXUN_LOG_ROOT();


void test_request() {
    radixun::http::HttpRequest::ptr req(new radixun::http::HttpRequest);
    req->setHeader("host" , "www.sylar.top");
    req->setBody("hello sylar");
    req->dump(std::cout) << std::endl;
}

void test_response() {
    radixun::http::HttpResponse::ptr rsp(new radixun::http::HttpResponse);
    rsp->setHeader("X-X", "sylar");
    rsp->setBody("hello sylar");
    rsp->setStatus((radixun::http::HttpStatus)400);
    rsp->setClose(false);

    rsp->dump(std::cout) << std::endl;
}

int main(int argc, char** argv) {
    test_request();
    // test_response();
    return 0;
}
