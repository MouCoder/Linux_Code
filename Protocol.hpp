#pragma once
#include "Log.hpp"
#include "Sock.hpp"
#include "Util.hpp"

#define URI_INDEX "wwwroot/source"

class HttpRequest
{
private:
    std::vector<std::string> request_header; //请求报头
    std::string request_line;   //请求行
    std::string request_blank;  //空行
    std::string request_body;   //请求正文

    std::string method; //请求方法
    std::string uri;    //uri
    std::string version;//http协议版本

    std::unordered_map<std::string,std::string> head_map;
  
    //要请求的文件资源的大小
    size_t file_size = 0;

    //cgi机制：举个例子来说
    //http服务器请求的内容不一定全是网页图片等资源
    //有时候也会是一些要交给服务器的数据
    //比如说，注册页面需要将用户名和密码等信息交给服务器
    //这是，就需要一个外部程序来处理用户名和密码，并保存到数据库
    //因为，http服务器它只负责协议的解析等，并返回给客户端浏览器它们需要的资源
    //因此，像这样的具体的服务就需要有具体的程序处理
    //而http和这个外部程序的交互就叫做CGI机制
    bool cgi = false;
    //将URI分为路径和参数两部分
    std::string path = "wwwroot/source";
    std::string param;
    int content_length = 0;//没有正文或者是get方法，该值就是0
public:
    HttpRequest()
      :request_blank("\n")
    {}
    void SetRequestLine(const std::string &str)
    {
        request_line = str;
    }
    
    void SetRequestHeader(const std::string &str)
    {
        request_header.push_back(str);
    }
    
    int GetContentLength()
    {
        return content_length;
    }
    void SetRequestBody(const std::string &str)
    {
        request_body = str;
    }

    bool IsCgi()
    {
        return cgi;
    }

    size_t GetFileSize()
    {
        return file_size;
    }

    bool IsPost()
    {
        return !strcasecmp(method.c_str(),"POST");
    }
    bool IsGet()
    {
        return !strcasecmp(method.c_str(),"GET");
    }
    std::string GetPath()
    {
        return path;
    }

    std::string GetParam()
    {
        return param;
    }

    std::string GetRequestBody()
    {
        return request_body;
    }

    void SetCgi()
    {
        cgi = true;
    }

    void SetPath(std::string _path)
    {
        path = _path;
    }
    void RequestLineParse()
    {
        //将请求行解析成三部分
        Util::StringPart3(request_line,method,uri,version);
        //LOG(Notice,method);
        //LOG(Notice,uri);
        //LOG(Notice,version);
    }
    void RequestHeaderParse()
    {
        for(auto it = request_header.begin();it != request_header.end();it++)
        {
            //将每一个kv字段保存在map中
            std::string key,val;
            Util::StringPart2(*it,key,val);
            head_map.insert({key,val});
            //注意，如果是get方法就没有该字段，那么body_size值就取默认值0
            if(key == "Content-Length")
            {
                content_length = atoi(val.c_str());
            }
        }
    }
    void HandlerURI(int &code)
    {
        //处理请求URI
        //GET - URI - path & param
        //POST - URI - path
        if(strcasecmp(method.c_str(),std::string("GET").c_str()) == 0)
        {
            //GET方法，分离参数
            //找
            size_t i = uri.find('?');
            if(i != std::string::npos)
            {
                //分离
                path += uri.substr(0,i);
                param = uri.substr(i+1);
                
                //因为get方法并且有参数需要处理，因此需要cgi技术
                cgi = true;
            }
            else 
            {
                //get方法且无参
                path += uri;
            }
        }
        else if(strcasecmp(method.c_str(),std::string("post").c_str()) == 0)
        {
            //如果有请求正文，就需要使用cgi技术
            if(content_length > 0)
                cgi = true;
            //post方法（只处理get和post方法）
            path += uri;
        }
        else      
        {
            //注意：我们的http服务器只处理Get和Post方法
            //如果不是get或者post方法，我们就认为方法错误
            code = 400;
            LOG(Warning,"method is not legal");
        }
    }

    std::string GetMethod()
    {
        return method;
    }

    void HandlerPath(int& code)
    {
        //结尾是'/'，也是一个目录，加上index.html
        if(path[path.size()-1] == '/')
        {
            //访问的是一个目录
            path += "index.html"; 
        }
        // /... -> ./wwwroot/... 
        // / -> ./wwwroot/index.html
        // /.../.. -> ./wwwroot/.../.../index.html
        //判断路径是否存在
        struct stat st;
        if(stat(path.c_str(),&st) >= 0)
        {
            //是一个目录，给他加上该目录的index.html
            if(S_ISDIR(st.st_mode))
            {
                path += "/index.html";
            }
            else if((st.st_mode & S_IXOTH) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXUSR))
            {
                //可执行文件
                cgi = true;
            }
            //不是cgi程序，就需要设置要请求的文件的大小
            if(!cgi)
            {
                file_size = st.st_size;
            }
        }
        else 
        {
            //路径不存在
            LOG(Warning,"uri not exit!!!");
            LOG(Notice,path);
            code = 404;
        }
    }
};

class HttpReponse
{
private:
    std::string response_line;
    std::string response_header;
    std::string response_blank;
    std::string response_body;

    static std::unordered_map<std::string,std::string> content_type;
    static std::unordered_map<int,std::string> status_code;
public:
    HttpReponse(){}

    static void InitStatusCodeMap()
    {
        //将常见的状态码和描述保存在map中
        //2xx表示请求正常处理
        status_code[200] = "OK";
        //请求被正常处理，但不包含响应正文
        status_code[204] = "No Content";
        //范围请求，并执行了请求的Get方法
        status_code[206] = "Partial content";

        //3XX重定向
        status_code[301] = "Moved Permanently";
        status_code[302] = "Found";

        //4XX客户端错误
        //400请求存在语法错误
        status_code[400] = "Bad Request";
        //403服务器阻止了请求（一般是没有权限）
        status_code[403] = "Forbidden";
        //404
        status_code[404] = "Not Found";

        //5XX服务器错误
        //500服务器在执行请求时发生了错误
        status_code[500] = "Internal Server Error";
        //503服务器超载或无法处理请求
        status_code[503] = "Service Unavailable";
    }
   
    static void InitContenTypeMap()
    {
        //从./wwwroot/contype.txt文件读取保存在map中
        std::ifstream Read("./wwwroot/contype.txt");

        if(Read)
        {
            std::string line;
            while(getline(Read,line))
            {
                //以：分离
                size_t pos = line.find(':');
                if(pos != std::string::npos)
                {
                    std::string key = line.substr(0,pos);
                    std::string val = line.substr(pos+1);
                    content_type[key] = val;
                }
            }
        }
        else 
        {
            LOG(Fatil,"InitContenTypeMap Fatil");
            exit(-1);
        }
    }
    std::string CodeToDescription(int code)
    {
        //通过状态码获取状态描述
        auto it = status_code.find(code); 
        if(it != status_code.end())
            return it->second;
        return "";
    }
    
    void SetResponseLine(std::string& line)
    {
        response_line = line;
    }
    
    void SetResponseHeader(std::string& str)
    {
        response_header = str;
    }

    std::string GetContentType(std::string& key)
    {
        auto val = content_type.find(key);
        if(val != content_type.end())
            return val->second; 
        return "";
    }

    std::string GetResponseLine()
    {
        return response_line;
    }

    std::string GetResponseHead()
    {
        return response_header;
    }
    ~HttpReponse(){}
};

std::unordered_map<int,std::string> HttpReponse::status_code;
std::unordered_map<std::string,std::string> HttpReponse::content_type;

class EndPoint
{
private:
    int sock; //客户端套接字
    HttpRequest req;
    HttpReponse resp;

    int code = 200;
private:
    void GetRequestLine()
    {
        std::string request_line;
        //从sock中读取一行内容
        Sock::GetLine(sock,request_line);
        req.SetRequestLine(request_line);
        //将请求行解析成三部分
        req.RequestLineParse();
    }
    
    void GetRequestHeader()
    {
        //按行读取
        std::string line;
        do{
            line = "";

            //按行读取
            Sock::GetLine(sock,line);
            //将读取到的内容保存到request_header中
            req.SetRequestHeader(line);
        }while(!line.empty());
        
        //解析请求报头
        req.RequestHeaderParse();
    }

    void GetRequestBody()
    {
        //content-length字段默认是0，
        //如果是POST方法没有正文或者是GET方法
        //就没有content-length字段或者说该字段值就是0
        //这里就可以判断是否有request_body
        std::string body;
        int i = req.GetContentLength();
        while(i)
        {
            char ch;
            recv(sock,&ch,1,0);

            body += ch;
            i--; 
        }
        req.SetRequestBody(body);
    }

    void ResponseStatusLine()
    {
        std::string line;
        //协议版本 http/1.0
        line = "HTTP/1.0";
        line += " ";
        //状态码
        line += std::to_string(code);
        //状态描述
        line += " ";
        line += resp.CodeToDescription(code);

        //制作的相应，统一以'\r\n作为换行'
        line += "\r\n";

        resp.SetResponseLine(line);
        LOG(Notice,line);
    }

    void ResponseHeaderLine()
    {
        std::string rsp_head;
        //最少有两个字段
        //content-length和conten-type
        //if(req.IsPost())
        //{
           //post方法可能会有content-length
           //size_t file_size = req.GetFileSize();
           //if(file_size > 0)
           //{
                //rsp_head += "Content-Length: ";
                //rsp_head += std::to_string(file_size);
           //}
        //}
        //设置content-type字段
        std::string path = req.GetPath();
        //从path中提取文件后缀
        std::string val;
        size_t pos = path.rfind('.');
        if(pos != std::string::npos)
        {
            val = path.substr(pos);
        }
        val = resp.GetContentType(val);
        rsp_head += "Content-Type: ";
        if(!val.empty())
        {
            rsp_head += val;
            rsp_head += "\r\n";
        }
        else 
        {
            rsp_head += "text/html";
        }
        resp.SetResponseHeader(rsp_head);
    }
public:
    EndPoint(int _sk):sock(_sk)
    {}

    void HandlerRequest()
    {
        //读取请求行 & 分离 --- 按行读取
        GetRequestLine();
        //读取请求报头 & 分离 & 存储到map 
        GetRequestHeader();
        //读取请求正文---post方法，且BodySize > 0
        GetRequestBody(); 
    }
    void MakeResponse()
    {
        //分析请求
        //处理请求报头---将URI分成path和参数两部分
        req.HandlerURI(code);
        //如果请求方法错误，HandlerURI中会将code设置为404
        //这里就在往下走了，直接制作响应
        //解析路径---将特殊路径转为合法路径
        if(code == 200)
            req.HandlerPath(code);
        //制作响应
        //响应行
        ResponseStatusLine();
        ResponseHeaderLine();
    }
    void SendResponse()
    {
        //发送响应行
        std::string line = resp.GetResponseLine();
        send(sock,line.c_str(),line.size(),0);
        //发送响应报头
        std::string header = resp.GetResponseHead();

        if(code == 404)
        {
            header = "Content-Type: text/html";
            header += "\r\n\r\n";
            LOG(Notice,header);
            
            send(sock,header.c_str(),header.size(),0);
            std::string path = "error path ";
            path += req.GetPath();
            LOG(Notice,path);
            req.SetPath("wwwroot/NotFound/zhanlingyueqiu/index.html");
            ExecNonCgi();
        }
        else 
        {
            LOG(Notice,header);
            send(sock,header.c_str(),header.size(),0);

            //分流执行cgi和非cgi
            if(req.IsCgi())
            {
                //执行cgi
                LOG(Notice,"exec cgi");
                ExecCgi();
            }
            else 
            {
                LOG(Notice,"exec non cgi");
                //非cgi
                ExecNonCgi();
            }
       }
    }

    void ExecCgi()
    {
        //思路：创建子进程，程序替换，子进程执行xgi程序
        
        //因为父子进程都要可以进行读写，而管道只能进行单向通信，因此需要创建两个管道
        //站在被调用进程的角度
        int pipe_in[2] = {0};
        int pipe_out[2] = {0};

        //将get方法中的参数设置到环境变量param_env中
        std::string param;
        std::string param_env;
        
        //post方法需要将content-length传过去，方便cgi程序读
        std::string content_length;
        std::string content_length_env;
        //path要执行的程序
        std::string path = req.GetPath();
        //创建管道
        pipe(pipe_in);
        pipe(pipe_out);
        std::string method_env;
        std::string method = req.GetMethod();
        std::string body;

        method_env = "METHOD=";
        method_env += method;
        LOG(Notice,method_env);
        putenv((char*)method_env.c_str());
        pid_t pid = fork();
        if(pid == 0)
        {
            //child
            //子进程从这里获取参数，并且执行完cgi程序后要将处理结果给父进程
            //因此，父子进程之间需要通信
            //思路：如果是get方法，它的参数通过uri传递，一般较短
            //因此我们可以使用环境变量将参数传给子进程
            //如果是post方法，请求正文一般较长，因此我们使用管道通信
            //子进程使用pipe_in读取，因此关闭pipe_int[1]
            //使用pipe_out写入，因此关闭pipe_out[0]
            close(pipe_in[1]);
            close(pipe_out[0]);
            //重定向
            dup2(pipe_in[0], 0);
            dup2(pipe_out[1], 1);
            if(req.IsGet())
            {
                //环境变量
                param = req.GetParam();
                param_env += "PARAM=";
                param_env += param;
                //设置环境变量
                putenv((char*)param_env.c_str());
            }
            if(req.IsPost())
            {
                //管道
                content_length = std::to_string(req.GetContentLength());
                content_length_env = "CONTENT-LENGTH=";
                content_length_env += content_length;
                putenv((char*)content_length_env.c_str());
            }
            
            //进程程序替换
            execl(path.c_str(),path.c_str(),nullptr);

            //替换成功不会返回，替换失败退出进程
            exit(0);
        }

        //父进程使用pipe_in写，关闭pipe_in[0]
        //使用pipe_out读，关闭pipe_out[1]
        close(pipe_in[0]);
        close(pipe_out[1]);

        //通过管道将post方法的数据传给子进程
        if(req.IsPost())
        {
            //向管道中写
            body = req.GetRequestBody();
            //将请求正文写到管道中
            size_t i = 0;
            for(; i < body.size(); i++)
            {
                write(pipe_in[1], &body[i], 1);
            }
        }
       
        //read from pipe
        char c = 'X';
        std::string ret = "\r\n\r\n";
        ssize_t s = 0;
        do{
            s = read(pipe_out[0], &c, 1);
            if(c >= 'a' && c <= 'z')
                ret += c;             
        }while(s > 0);
        
        send(sock,ret.c_str(),ret.size(),0);
        LOG(Notice,ret);
    }
    void ExecNonCgi()
    {
        ssize_t size = req.GetFileSize();

        std::string path = req.GetPath();
        LOG(Notice,path);
       if(path == "wwwroot/NotFound/zhanlingyueqiu/index.html")
        {
            struct stat st;
            stat(path.c_str(),&st);
            size = st.st_size;
        }
        int fd = open(path.c_str(), O_RDONLY);
        if(fd < 0)
        {
            LOG(Error, "path is not exists bug!!!");
            return;
        }
        //LOG(Notice,path);
        //std::ifstream Read(path);

        //if(Read)
        //{
            //std::stringstream line;
            //line << Read.rdbuf();
            //LOG(Notice,line.str());
            //send(sock,line.str().c_str(),line.str().size(),0);
        //}
        
        sendfile(sock, fd, nullptr, size);
        close(fd);
    }

    ~EndPoint()
    {
        if(sock > 0)
          close(sock);
    }
};

class Entry
{
public:
    static void* HandlerHttp(void* arg)
    {
        int sock = *(int*)arg;
#ifdef DEBUG 
        char buffer[10240] = {0};

        recv(sock,buffer,sizeof(buffer),0);

        std::cout<<buffer<<std::endl;
        close(sock);
#else 
        EndPoint* ep = new EndPoint(sock);
        //处理请求
        ep->HandlerRequest();
        ep->MakeResponse();
        ep->SendResponse();
        delete ep;
#endif
    }
    static void HandlerHttp(int sock)
    {
#ifdef DEBUG 
        char buffer[10240] = {0};

        recv(sock,buffer,sizeof(buffer),0);

        std::cout<<buffer<<std::endl;
        close(sock);
#else 
        EndPoint* ep = new EndPoint(sock);
        //处理请求
        ep->HandlerRequest();
        ep->MakeResponse();
        ep->SendResponse();
        delete ep;
#endif
    }
};
