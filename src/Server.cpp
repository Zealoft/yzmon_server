#include "Server.hpp"
#include "encrpty.h"
#include "actions.hpp"
#include <string>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <ctime>

using namespace std;
using namespace logger;
using namespace package;

Server::Server(const string &cfg_path)
{
    // cout << cfg_name << endl;
    debug(Level::Info) << "Server starting";
    record(Level::Info) << "Server starting";
    config_getter.set_path(cfg_path);
    init_config();
    init_servernetwork();
    sql_dealer = new SQL_Dealer(config.server_db_ip, config.server_db_port, config.server_db_name, config.db_username, config.db_passwd);

}


Server::~Server()
{
    try {
        debug(Level::Info) << "Server ending";
        record(Level::Info) << "Server ending";
        delete sql_dealer;
        
    } 
    catch(const char *msg) {
        debug(Level::Error) << msg;
        record(Level::Error) << msg;
    }

    close(connfd);
}

void Server::init_config()
{
    config_getter.parse();
    config.listen_port = atoi(config_getter.Get_Value("监听端口号").c_str());
    config.device_connect_interval = atoi(config_getter.Get_Value("设备连接间隔").c_str());
    config.device_sampling_inverval = atoi(config_getter.Get_Value("设备采样间隔").c_str());
    // config.device_sampling_interval = atoi(config_getter.Get_Value("设备采样间隔").c_str());

    config.server_db_ip = config_getter.Get_Value("服务器IP地址");
    config.server_db_port = atoi(config_getter.Get_Value("服务器端口号").c_str());
    config.server_db_name = config_getter.Get_Value("数据库名");
    config.db_username = config_getter.Get_Value("用户名");
    config.db_passwd = config_getter.Get_Value("用户口令");

    config.no_answer_shuttime = atoi(config_getter.Get_Value("未应答超时").c_str());
    config.transmit_shuttime = atoi(config_getter.Get_Value("传输超时").c_str());

    config.major_log_size = atoi(config_getter.Get_Value("主日志大小").c_str());
    config.minor_log_size = atoi(config_getter.Get_Value("分日志大小").c_str());

    config.screen_show = atoi(config_getter.Get_Value("屏幕显示").c_str());
    memcpy(config.tmp_packet, config_getter.Get_Value("tmp_packet").c_str(), 4);
    memcpy(config.tmp_socket, config_getter.Get_Value("tmp_socket").c_str(), 4);
    memcpy(config.dev_packet, config_getter.Get_Value("dev_packet").c_str(), 4);
    memcpy(config.dev_socket, config_getter.Get_Value("dev_socket").c_str(), 4);

    record(Level::Info) << "read config finish";
}

void Server::init_servernetwork()
{
    int port = config.listen_port;
    int queue_size = 1024;
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_addr.sin_port = htons(port);

    if ((connfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        debug(Level::Fatal) << "socket init error:" << strerror(errno);
        record(Level::Fatal) << "socket init error:" << strerror(errno);
        exit(EXIT_FAILURE);
    }
    setSockNonBlock(connfd);
    // record(Level::Info) << "connfd is " << connfd;
    long flag = 1;
    setsockopt(connfd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag));
    if (bind(connfd, (sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
        debug(Level::Fatal) << "socket bind error:" << strerror(errno);
        record(Level::Fatal) << "socket bind error:" << strerror(errno);
        exit(EXIT_FAILURE);
    }
    if (listen(connfd, queue_size) < 0) {
        debug(Level::Fatal) << "socket listen error:" << strerror(errno);
        record(Level::Fatal) << "socket listen error:" << strerror(errno);
        exit(EXIT_FAILURE);
    }
    debug(Level::Info) << "Server network established. Listening port: "
        << port << ", connfd: " << connfd; 
    record(Level::Info) << "Server network established. Listening port: "
        << port << ", connfd: " << connfd;

}

int Server::init_connect()
{
    int sin_size = sizeof(sockaddr_in);
    sockaddr_in remote_addr;
    int client_fd = accept(connfd, (sockaddr *)&remote_addr, (socklen_t *)&sin_size);
    if (client_fd < 0) {
        record(Level::Error) << "accept error: " << strerror(errno);
        return -1;
    }
    record(Level::Info) << "receive a connection: " << inet_ntoa(remote_addr.sin_addr)
        << ": " << ntohs(remote_addr.sin_port);
    setSockNonBlock(client_fd);
    Connection client_conn;
    client_conn.sockfd = client_fd;
    conn_vec.push_back(client_conn);
    return client_fd;
}

void Server::setSockNonBlock(int sock)
{
    int flags;
    flags = fcntl(sock, F_GETFL, 0);
    if(flags < 0) {
        logger::record(logger::Level::Error) << "fcntl(F_GETFL) failed!";
        exit(EXIT_FAILURE);
    }
    if(fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
        logger::record(logger::Level::Error) << "fcntl(F_SETFL) failed!";
        exit(EXIT_FAILURE);
    }
}

void Server::generate_authen_string(authentication_request &ar)
{
    u_int random_num, svr_time;
    int pos;
    stringstream ss;
    random_num = (u_int)rand();
    svr_time = (u_int)time(0);
    svr_time = svr_time ^ (u_int)0xFFFFFFFF;
    pos = (random_num % 4093);
    for (int i = 0;i < 32;i++) {
        ar.authentication_string[i] = secret[pos] ^ authen_str[i];
        pos = ++pos % 4093;
        ss << ar.authentication_string[i];
    }
    // debug(Level::Info) << "authen str: " << ss;
    record(Level::Info) << "gen authen str: " << ss;
    ar.random_num = random_num;
    ar.svr_time = svr_time;
}

bool Server::check_authen_string(package::authentication_response &ar)
{
    stringstream ss;
    bool res = true;
    u_int random_num = ar.random_num;
    int pos = random_num % 4093;
    for (int i = 0; i < 32; i++) {
        if (ar.authen_str[i] != authen_str[i])
            res = false;
        pos = ++pos % 4093;
        ss << ar.authen_str[i];
    }
    
    // debug(Level::Info) << "authen str: " << ss;
    record(Level::Info) << "check authen str: " << ss;
    return res;
}

void Server::decrypt_response(package::authentication_response &ar)
{
    char *p = (char *)&ar;
    u_int random_num = ar.random_num;
    int pos = random_num % 4093;
    for (int i = 0; i < 104; i++) {
        p[i] = p[i] ^ secret[pos];
        pos = ++pos % 4093;
    }
}

void Server::Authentication_Request(const std::vector<Connection>::iterator &iter)
{
    authentication_request ar;
    vector<string> str_vec = String_Dealer::split(server_version, ".");
    ar.major_version = (short)atoi(str_vec[0].c_str());
    ar.minor_version1 = (char)atoi(str_vec[1].c_str());
    ar.minor_version2 = (char)atoi(str_vec[2].c_str());
    ar.device_connect_interval = (short)config.device_connect_interval;
    ar.device_sampling_interval = (short)config.device_sampling_inverval;
    ar.empty_console_allowed = 1;
    generate_authen_string(ar);
    ar.hton();
    record(Level::Info) << "to fd: " << iter->sockfd << ", authentication request sending, length: " << sizeof(ar);
    MakePackage(iter, (char *)&ar, sizeof(ar), package::authen);
}

/*
 * 发送基本的信息获取请求包
 * 根据后续收到的信息回复包再继续发包
 */
void Server::Getinfo_Request(const std::vector<Connection>::iterator &iter)
{
    MakePackage(iter, NULL, 0, package::sys_info);
    MakePackage(iter, NULL, 0, package::configure_info);
    MakePackage(iter, NULL, 0, package::process_info);
    MakePackage(iter, NULL, 0, package::ethernet_info, package::ethernet_0);
    MakePackage(iter, NULL, 0, package::ethernet_info, package::ethernet_1);
    MakePackage(iter, NULL, 0, package::usb_available);
    MakePackage(iter, NULL, 0, package::printer_info);
    MakePackage(iter, NULL, 0, package::terminal_info);
}

/*
 * 取指定连接的队首数据包发出
 * 数据包头在此处转化为网络序
 * 数据内容需要提前转网络序
 */
void Server::SendPackage(const vector<Connection>::iterator &iter)
{
    Package p = iter->wqueue.front();
    int len = p.head.length, wlen;
    p.head.hton();
    wlen = write(iter->sockfd, &p, len);
    if (wlen < 0) {
        record(Level::Error) << "to fd:" << iter->sockfd << strerror(errno);
        return;
    }
    else if (wlen < len) {
        record(Level::Warning) << "to fd: " << iter->sockfd 
            << ", actual write length less than package length";
    }
    else {
        record(Level::Info) << "to fd: " << iter->sockfd
            << ", write success. length of package: " << len;
    }
    iter->wqueue.pop();
}

void Server::RecvPackage(vector<Connection>::iterator &iter)
{
    char buffer[TCP_BUFFER_SIZE];
    Package p;
    char *cur;
    int rlen = read(iter->sockfd, buffer, TCP_BUFFER_SIZE);
    // record(Level::Info) << 
    if (rlen < 0) {
        record(Level::Error) << "from fd: " << iter->sockfd << "receive error: " 
            << strerror(errno);
        return;
    }
    else if (rlen == 0) {
        // TCP连接的情况下，read返回0表示对方关闭了连接
        record(Level::Info) << "from fd: " << iter->sockfd
            << ", peer shutdown the connection.";
        EndConnection(iter);
    }
    else {
        record(Level::Info) << "from fd: " << iter->sockfd 
            << "Receive a package. rlen: " << rlen;
    }
    cur = buffer;
    while (cur < buffer + rlen - 1) {
        // 解析报文头
        memcpy((char *)&p.head, cur, sizeof(package::header));
        cur += sizeof(package::header);
        p.head.ntoh();
        if (p.head.length < sizeof(package::header)) {
            record(Level::Error) << "from client: " << iter->sockfd
                << "receive a package shorted than header";
            return;
        }
        // if (cur == buffer + rlen - 1) {
        //     if (p.head.detailed_type == package::end_connect) {
        //         record(Level::Info) << "收到了来自客户端: " << iter->sockfd
        //             << "的结束连接请求";
        //         EndConnection(iter);
        //         return;
        //     }
            
        //     // 所有包均收到，做相应处理
            
            
        // }
        memcpy(p.content, cur, p.head.length - sizeof(package::header));
        cur += p.head.length - sizeof(package::header);
        PrintContent((char *)&p, p.head.length);
        ResolvePackage(iter, p);

        if (conn_vec.size() == 0) {
            // record(Level::Info) << "empty vector!";
            return;
        }
            
    }
    
}

void Server::ResolvePackage(vector<Connection>::iterator &iter, const Package& p)
{
    char src = p.head.src_end;
    char type = p.head.detailed_type;
    if (src != package::src_client) {
        record(Level::Error) << "from fd: " << iter->sockfd << ": receive an invalid package";
        return;
    }
    /*
     * resolve detailed message
     */
    
    if (type == package::authen) {
        // 收到了认证请求的应答报文
        record(Level::Info) << "from fd: " << iter->sockfd << ": receive an authen package.";
        authentication_response ar;
        memcpy(&ar, p.content, sizeof(ar));
        ar.random_num = ntohl(ar.random_num);
        decrypt_response(ar);
        ar.ntoh();
        // record(Level::Info) << "decrypted flash: " << ar.flash;
        if (!check_authen_string(ar)) {
            // 未通过认证，不接受数据，关闭TCP连接
            record(Level::Info) << "from fd: " << iter->sockfd << ": authentication failed";
            debug(Level::Info) << "from fd: " << iter->sockfd << ": authentication failed";
            EndConnection(iter);
        }
        else {
            // 通过了认证，将数据存入数据库
            record(Level::Info) << "from fd: " << iter->sockfd << ": authentication success";
            iter->ram = ar.ram;
            iter->devid = ar.device_agency_num;
            iter->devno = ar.device_inside_num;
            SQL_RESULT res = sql_dealer->insert_authen_response_data(ar);
            // 准备开始向设备发送取信息请求
            Getinfo_Request(iter);
        }
    }
    else if(type == package::end_connect) {
        // 客户端发来的断开连接回复
        record(Level::Info) << "resolve: 收到了来自客户端：" << iter->sockfd << "的断开连接请求。";
        EndConnection(iter);
        return;
    }

    else if (type == package::sys_info) {
        // 收到了取系统信息的回复报文
        record(Level::Info) << "from fd: " << iter->sockfd << ": receive an system info package.";
        sysinfo_response sr;
        memcpy(&sr, p.content, sizeof(sr));
        sr.ntoh();
        SQL_RESULT res = sql_dealer->insert_sysinfo_response_data(sr, *iter);
        if (res == SUC) {
            record(Level::Info) << "System info package deal done.";
        }
        else {
            record(Level::Error) << "from fd: " << iter->sockfd << ", system info package error ";
        }
        // record(Level::Info) << "insert finished.";
    }
    else if (type == package::configure_info) {
        record(Level::Info) << "from fd: " << iter->sockfd << ": receive a configure info package.";
        configure_info_response cr;
        memcpy(&cr, p.content, p.head.data_length);
        cr.ntoh();
        SQL_RESULT res = sql_dealer->insert_configure_response_data(cr, *iter);
        if (res == SUC) {
            record(Level::Info) << "config info package deal done.";
        }
        else {
            record(Level::Error) << "from fd: " << iter->sockfd << ", config info package error ";
        }
    }
    else if (type == package::process_info) {
        record(Level::Info) << "from fd: " << iter->sockfd << ": receive a process info package.";
        process_info_response pr;
        memcpy(&pr, p.content, p.head.data_length);
        pr.ntoh();
        SQL_RESULT res = sql_dealer->insert_process_response_data(pr, *iter);
        if (res == SUC) {
            record(Level::Info) << "process info package deal done.";
        }
        else {
            record(Level::Error) << "from fd: " << iter->sockfd << ", process info package error ";
        }
    }
    else if (type == package::ethernet_info) {
        record(Level::Info) << "from fd: " << iter->sockfd << ": receive an ethernet info package.";
        ethernet_info_response er;
        memcpy(&er, p.content, p.head.data_length);
        er.ntoh();
        SQL_RESULT res = sql_dealer->insert_ethernet_response_data(er, *iter, p.head.padding_num);
        if (res == SUC) {
            record(Level::Info) << "ethernet info package deal done.";
        }
        else {
            record(Level::Error) << "from fd: " << iter->sockfd << ", ethernet info package error ";
        }
    }
    else if (type == package::usb_available) {
        record(Level::Info) << "from fd: " << iter->sockfd << ": receive a usb port info package.";
        usb_available_response ur;
        memcpy(&ur, p.content, p.head.data_length);
        ur.ntoh();
        bool is_plugin;
        SQL_RESULT res = sql_dealer->insert_usb_available_response_data(ur, *iter, is_plugin);
        if (res == SUC) {
            record(Level::Info) << "usb plugin info package deal done.";
            if (is_plugin) {
                // u盘已经插入，请求文件信息
                record(Level::Info) << "请求U盘的文件信息";
                MakePackage(iter, NULL, 0, package::usb_filelist);
            }
        }
        else {
            record(Level::Error) << "from fd: " << iter->sockfd << ", usb plugin info package error ";
        }
        

    }
    else if (type == package::printer_info) {
        record(Level::Info) << "from fd: " << iter->sockfd << ": receive a printer info package.";
        printer_info_response pr;
        memcpy(&pr, p.content, p.head.data_length);
        pr.ntoh();
        bool is_running;
        SQL_RESULT res = sql_dealer->insert_printer_info_response_data(pr, *iter, is_running);
        if (res == SUC) {
            record(Level::Info) << "printer info package deal done.";
            if (is_running) {
                // 打印队列中有任务，请求打印信息
                record(Level::Info) << "请求打印机的队列信息";
                MakePackage(iter, NULL, 0, package::printer_queue_info);
            }
        }
        else {
            record(Level::Error) << "from fd: " << iter->sockfd << ", printer info package error ";
        }
    }
    else if (type == package::terminal_info) {
        record(Level::Info) << "from fd: " << iter->sockfd << ": receive a terminal info package.";
        terminal_info_response tr;
        memcpy(&tr, p.content, p.head.data_length);
        tr.ntoh();
        SQL_RESULT res = sql_dealer->insert_tty_config_response_data(tr, *iter);
        if (res == SUC) {
            record(Level::Info) << "tty config info package deal done.";
        }
        else {
            record(Level::Error) << "from fd: " << iter->sockfd << ", tty config info package error ";
        }
        // 逐个发送取配置信息的请求
        for (int i = 0; i < 16; i++) {
            if (tr.deaf_terminal_used[i] == 0)
                continue;
            iter->tty_num++;
            MakePackage(iter, NULL, 0, package::deaf_terminal, i + 1);
        }
        for (int i = 0; i < 254; i++) {
            if (tr.ip_terminal_used[i] == 0)
                continue;
            iter->tty_num++;
            MakePackage(iter, NULL, 0, package::ip_terminal, i + 1);
        }
    }
    else if (type == package::usb_filelist) {
        record(Level::Info) << "from fd: " << iter->sockfd << ": receive a usb filelist package.";
        usb_filelist_response ur;
        memcpy(&ur, p.content, p.head.data_length);
        ur.ntoh();
        SQL_RESULT res = sql_dealer->insert_usb_filelist_response_data(ur, *iter);
        if (res == SUC) {
            record(Level::Info) << "usb filelist info package deal done.";
        }
        else {
            record(Level::Error) << "from fd: " << iter->sockfd << ", usb filelist info package error ";
        }
    }
    else if (type == package::printer_queue_info) {
        record(Level::Info) << "from fd: " << iter->sockfd << ": receive a printer queue info package.";
        printer_queue_response pr;
        memcpy(&pr, p.content, p.head.data_length);
        pr.ntoh();
        SQL_RESULT res = sql_dealer->insert_printer_queue_response_data(pr, *iter);
        if (res == SUC) {
            record(Level::Info) << "printer file info package deal done.";
        }
        else {
            record(Level::Error) << "from fd: " << iter->sockfd << ", printer file info package error ";
        }
    }
    else if (type == package::deaf_terminal) {
        record(Level::Info) << "from fd: " << iter->sockfd << ": 收到一个哑终端配置信息包";
        int pointer = 0;
        detailed_terminal_info_response dtr;
        memcpy(&dtr, p.content, sizeof(dtr));
        pointer += sizeof(dtr);
        dtr.ntoh();
        SQL_RESULT res = sql_dealer->insert_detailed_tty_info_data(dtr, *iter, p.head);
        if (res == SUC) {
            record(Level::Info) << "detailed tty info package deal done.";
        }
        else {
            record(Level::Error) << "from fd: " << iter->sockfd << ", detailed tty info package error ";
        }
        for (int i = 0; i < dtr.screen_num; i++) {
            screen_info si;
            memcpy(&si, &p.content[pointer], sizeof(si));
            pointer += sizeof(si);
            si.ntoh();
            SQL_RESULT res = sql_dealer->insert_screen_info_data(si, *iter, p.head, dtr);
            if (res == SUC) {
                record(Level::Info) << "screen info package deal done.";
            }
            else {
                record(Level::Error) << "from fd: " << iter->sockfd << ", screen info package error ";
            }
        }
        iter->tty_num_received++;
        if (iter->tty_num_received == iter->tty_num) {
            SQL_RESULT res = sql_dealer->insert_tty_connected_data(iter->tty_num, *iter);
            if (res == SUC) {
                record(Level::Info) << "tty num insert deal done.";
            }
            else {
                record(Level::Error) << "from fd: " << iter->sockfd << ", tty num insert error ";
            }
        }
    }
    else if (type == package::ip_terminal) {
        record(Level::Info) << "from fd: " << iter->sockfd << ": 收到一个IP终端配置信息包";
        int pointer = 0;
        detailed_terminal_info_response dtr;
        memcpy(&dtr, p.content, sizeof(dtr));
        pointer += sizeof(dtr);
        dtr.ntoh();
        SQL_RESULT res = sql_dealer->insert_detailed_tty_info_data(dtr, *iter, p.head);
        if (res == SUC) {
            record(Level::Info) << "detailed tty info package deal done.";
        }
        else {
            record(Level::Error) << "from fd: " << iter->sockfd << ", detailed tty info package error ";
        }
        for (int i = 0; i < dtr.screen_num; i++) {
            screen_info si;
            memcpy(&si, &p.content[pointer], sizeof(si));
            pointer += sizeof(si);
            si.ntoh();
            SQL_RESULT res = sql_dealer->insert_screen_info_data(si, *iter, p.head, dtr);
            if (res == SUC) {
                record(Level::Info) << "screen info package deal done.";
            }
            else {
                record(Level::Error) << "from fd: " << iter->sockfd << ", screen info package error ";
            }
        }
        iter->tty_num_received++;
        if (iter->tty_num_received == iter->tty_num) {
            SQL_RESULT res = sql_dealer->insert_tty_connected_data(iter->tty_num, *iter);
            if (res == SUC) {
                record(Level::Info) << "tty num insert deal done.";
            }
            else {
                record(Level::Error) << "from fd: " << iter->sockfd << ", tty num insert error ";
            }
        }
    }
    iter->response_num++;
    // 在这里判断是否已经收到所有包
    if (iter->response_num != 0 && iter->request_num == iter->response_num) {
        // 已经收到所有包，发送结束连接请求
        record(Level::Info) << "已经收到了来自fd为" << iter->sockfd << "的客户端的所有包";
        MakePackage(iter, NULL, 0, package::end_connect);
    }
    
}

void Server::PrintContent(const char *content, int size, bool is_receive)
{
    const char *p = content;
    stringstream ss;

    for (int i = 1;i <= size; i++) {
        int temp = p[i - 1] & 0x000000ff;
        ss << setw(2) << setfill('0') << hex << temp << " ";
        if (i != 1 && i % 16 != 0 && i % 8 == 0) 
            ss << " -  ";
        if (i != 1 && i % 16 == 0)
            ss << endl;
    }
    ss << dec << "";
    if (is_receive)
        record(Level::Info) << "收到包（网络序显示）， " << "长度为： " << size << endl << ss.str();
    else
        record(Level::Info) << "发送包（网络序显示）， " << "长度为： " << size << endl << ss.str();
}

/*
 * 将content构造成包并放在写队列的队尾
 */
void Server::MakePackage(const std::vector<Connection>::iterator &iter, 
    const char *content, int content_size, char type, short additional_num)
{
    Package p;
    if (content == NULL)
        p.head.length = 8;
    else {
        p.head.length = (content_size > MAX_CONTENT_SIZE) ? (sizeof(package::header) + MAX_CONTENT_SIZE)
            : sizeof(package::header) + content_size;
    }
    p.head.data_length = content_size;
    p.head.detailed_type = type;
    p.head.src_end = package::src_server;
    p.head.padding_num = additional_num;
    // p.head.hton();
    if (content == NULL) {
        
    }
    else {
        memcpy(p.content, content, content_size);
    }
    PrintContent((char *)&p, p.head.length, false);
    iter->wqueue.push(p);
    iter->request_num++;
}

void Server::EndConnection(std::vector<Connection>::iterator &iter)
{
    record(Level::Info) << "close the connection whose fd is " << iter->sockfd;
    close(iter->sockfd);
    // conn_vec.erase(iter);
    iter = conn_vec.erase(iter);
    iter--;
}

void Server::Process_Connections()
{
    // signal(SIGALRM, )
    int maxfd, result;
    fd_set rfds, wfds;


    while (true) {
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_SET(connfd, &rfds);
        maxfd = connfd;
        // debug(Level::Info) << "begin select...";
        for (auto i = conn_vec.begin(); i != conn_vec.end(); ++i) {
            FD_SET(i->sockfd, &rfds);
            if (!i->wqueue.empty())
                FD_SET(i->sockfd, &wfds);
            if (i->sockfd > maxfd)
                maxfd = i->sockfd;
        }
        result = select(maxfd + 1, &rfds, &wfds, NULL, NULL);
        if (result > 0) {
            if (FD_ISSET(connfd, &rfds)) {
                // establish new connections
                int fd = init_connect();
                record(Level::Info) << "back from init connect";
                if (fd > 0) {
                    for (auto i = conn_vec.begin(); i != conn_vec.end(); ++i) {
                        if (i->sockfd == fd) {
                            record(Level::Info) << "ready to send requests";
                            Authentication_Request(i);
                            break;
                        }
                    }
                }

            }
            for (auto i = conn_vec.begin(); i != conn_vec.end(); ++i) {
                /*
                 * 由于先读后写的方式会导致迭代器指向空指针
                 * 进而引起总线错误，因此使用先写后读的方式
                 * 确保RecvPackage之后不会再有对迭代器的操作
                 */
                if (FD_ISSET(i->sockfd, &wfds)) {
                    SendPackage(i);
                }
                if (FD_ISSET(i->sockfd, &rfds)) {
                    RecvPackage(i);
                }
                
            }
        }
        else if (result < 0) {
            record(Level::Fatal) << "select返回值小于0，结束所有连接！";
            for (auto i = conn_vec.begin();i != conn_vec.end();++i) {
                if (FD_ISSET(i->sockfd, &rfds)) {
                    EndConnection(i);
                }
            }
        }
    }
}