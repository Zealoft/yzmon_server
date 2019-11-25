#pragma once

#include "common/ConfigReader.hpp"
#include "common/StringTools.hpp"
#include "common/Logger.hpp"
#include "package.hpp"
#include "Connection.hpp"
#include "sql.hpp"
#include <netinet/in.h>             // sockaddr_in
#include <string>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

#include <vector>

#define TCP_BUFFER_SIZE 65536
#define MAX_CONTENT_SIZE 1024

struct config_information {
    int listen_port;
    int device_connect_interval;
    int device_sampling_inverval;

    std::string server_db_ip;
    int server_db_port;
    std::string server_db_name;
    std::string db_username;
    std::string db_passwd;

    int no_answer_shuttime;
    int transmit_shuttime;

    int major_log_size;
    int minor_log_size;

    int screen_show;
    char tmp_packet[4];
    char tmp_socket[4];
    char dev_packet[4];
    char dev_socket[4];
};

class Server {
private:
    const std::string server_version = "2.0.0";
    const std::string authen_str = "yzmond:id*str&to!tongji@by#Auth^";
    const std::string& cfg_name = "yzmond.conf";
    config_reader config_getter;
    config_information config;
    SQL_Dealer* sql_dealer;
    int connfd;
    struct sockaddr_in my_addr; // 本机地址信息

    std::vector<Connection> conn_vec;

    void init_config();
    void init_servernetwork();
    int init_connect();
    void setSockNonBlock(int sock);

    void generate_authen_string(package::authentication_request &ar);
    bool check_authen_string(package::authentication_response &ar);
    void decrypt_response(package::authentication_response &ar);
    void Authentication_Request(const std::vector<Connection>::iterator &iter);
    void Getinfo_Request(const std::vector<Connection>::iterator &iter);

    void SendPackage(const std::vector<Connection>::iterator &iter);
    void RecvPackage(std::vector<Connection>::iterator &iter);
    void ResolvePackage(std::vector<Connection>::iterator &iter, const Package& p);
    void MakePackage(const std::vector<Connection>::iterator &iter, const char *content, 
        int size, char type, short additional_num = 0x0000);
    void PrintContent(const char *content, int size, bool is_receive = true);
    void EndConnection(std::vector<Connection>::iterator &iter);
public:
    Server(const std::string &cfg_path = "yzmond.conf");
    ~Server();
    
    void Process_Connections();
};
