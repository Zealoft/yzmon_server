#pragma once

#include <string>
#include <mysql.h>

#include "package.hpp"
#include "Connection.hpp"

enum SQL_RESULT
{
    SUC,    // 成功
    ERR_NE, // 用户不存在
    ERR_WP, // 密码错误
    ERR_IN, // 内部错误
};

class SQL_Dealer {
private:
    std::string server_addr;
    int server_port;
    std::string db_name;
    std::string db_username;
    std::string db_passwd;
    MYSQL *mysql;

    SQL_RESULT connect_db();
    SQL_RESULT end_connect();
public:
    SQL_Dealer() = delete;
    SQL_Dealer(SQL_Dealer &&) = delete;
    SQL_Dealer(const SQL_Dealer &) = delete;
    SQL_Dealer(std::string addr, int port, std::string db_name, std::string db_username, std::string db_passwd) {
        this->server_addr = addr;
        this->server_port = port;
        this->db_name = db_name;
        this->db_username = db_username;
        this->db_passwd = db_passwd;
        init_db();
    }
    ~SQL_Dealer();
    SQL_RESULT insert_data();
    
    SQL_RESULT init_db();

    SQL_RESULT insert_authen_response_data(const package::authentication_response &ar);

    SQL_RESULT insert_sysinfo_response_data(const package::sysinfo_response &sr, 
        const Connection &conn);

    SQL_RESULT insert_configure_response_data(const package::configure_info_response &cr, 
        const Connection &conn);

    SQL_RESULT insert_process_response_data(const package::process_info_response &pr, 
        const Connection &conn);

    SQL_RESULT insert_ethernet_response_data(const package::ethernet_info_response &er,
        const Connection &conn, short ether_num);

    SQL_RESULT insert_usb_available_response_data(const package::usb_available_response &ur,
        const Connection &conn, bool &is_plugin);

    SQL_RESULT insert_usb_filelist_response_data(const package::usb_filelist_response &ur,
        const Connection &conn);

    SQL_RESULT insert_printer_info_response_data(const package::printer_info_response &pr,
        const Connection &conn, bool &is_running);

    SQL_RESULT insert_printer_queue_response_data(const package::printer_queue_response &pr,
        const Connection &conn);
    
    SQL_RESULT insert_tty_config_response_data(const package::terminal_info_response &tr,
        const Connection &conn);
    
    SQL_RESULT insert_detailed_tty_info_data(const package::detailed_terminal_info_response &dtr,
        const Connection &conn, const package::header &head);

    SQL_RESULT insert_screen_info_data(const package::screen_info &si, 
        const Connection &conn, const package::header &head, 
        const package::detailed_terminal_info_response &tr);
    
    SQL_RESULT insert_tty_connected_data(int tty_connected, const Connection &conn);
};