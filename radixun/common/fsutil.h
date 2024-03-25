#ifndef __RADIXUN_FDUTIL_H__
#define __RADIXUN_FDUTIL_H__

#include <string>
#include <vector>
#include <ios>

namespace radixun{

//文件管理类
class FSUtil {
public:
    //  获取文件夹下的所有文件
    //  path: 文件夹路径  subfix: 检索文件后缀
    static void ListAllFile(std::vector<std::string>& files
                            , const std::string& path
                            , const std::string& subfix);
    //创建文件夹
    static bool Mkdir(const std::string& dirname);
    //判断该进程是否运行
    static bool IsRunningPidfile(const std::string& pidfile);
    //删除文件
    static bool Rm(const std::string& path);
    //移动文件
    static bool Mv(const std::string& from, const std::string& to);
    //真正的文件路径
    static bool Realpath(const std::string& path,std::string& rpath);
    //创建软连接
    static bool Symlink(const std::string& frm, const std::string& to);
    //删除软连接
    static bool Unlink(const std::string& filename, bool exist= false);
    static std::string Dirname(const std::string& filename);
    static std::string Basename(const std::string& filename);
    static bool OpenForRead(std::ifstream& ifs, const std::string& filename
                            , std::ios_base::openmode mode);
    static bool OpenForWrite(std::ofstream& ofs, const std::string& filename
                            , std::ios_base::openmode mode);
};


}

#endif