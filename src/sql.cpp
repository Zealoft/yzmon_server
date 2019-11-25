#include "sql.hpp"
#include "common/Logger.hpp"
#include "common/StringTools.hpp"
#include "package.hpp"

#include <string>
#include <cstring>
#include <sstream>
#include <ctime>
#include <mysql.h>

using namespace logger;
using namespace std;
using namespace package;


SQL_RESULT SQL_Dealer::insert_data()
{
    
}

SQL_Dealer::~SQL_Dealer()
{

}

SQL_RESULT SQL_Dealer::init_db()
{
    mysql = mysql_init(NULL);
    if (mysql == NULL) {
        record(Level::Fatal) << "mysql init error!";
        debug(Level::Fatal) << "mysql init error!";
        return ERR_IN;
    }
    mysql_set_character_set(mysql, "gbk"); 
    
    record(Level::Info) << "mysql init finish";
    return SUC;
}

SQL_RESULT SQL_Dealer::connect_db()
{
    // 连接数据库，失败返回NULL
    // 1、mysqld没运行
    // 2、没有指定名称的数据库存在
    mysql = mysql_init(NULL);
    if (mysql_real_connect(mysql, server_addr.c_str(), db_username.c_str(), 
        db_passwd.c_str(),db_name.c_str(), server_port, NULL, 0) == NULL) {
        record(Level::Fatal) << "mysql connect: " << mysql_error(mysql);
        debug(Level::Fatal) << "mysql connect: " << mysql_error(mysql);
        return ERR_IN;
    }
    string sql_command = "SET NAMES gbk;";
    mysql_real_query(mysql, sql_command.c_str(), sql_command.length());
    return SUC;
}

SQL_RESULT SQL_Dealer::end_connect()
{
    mysql_close(mysql);
    return SUC;
}

SQL_RESULT SQL_Dealer::insert_authen_response_data(const package::authentication_response &ar)
{
    connect_db();
    stringstream ss;
    ss << ar.device_agency_num;
    string base_devid = ss.str();
    // 清空stringstream缓冲区
    ss.clear();
    ss.str("");

    ss << (int)ar.device_inside_num;
    string base_devno = ss.str();
    ss.clear();
    ss.str("");
    // 使用sql内置函数获取当前时间
    string base_time = "now()";

    // 客户端ip地址写死，暂时不进行获取
    string base_ipaddr = "192.168.1.244";

    ss << ar.group_num << "-" << ar.inside_serial;
    string base_sid = ss.str();
    ss.clear();
    ss.str("");

    string base_type = ar.device_type;

    string base_version = ar.device_version;

    ss << ar.cpu_clock;
    string base_cpu = ss.str();
    ss.clear();
    ss.str("");

    ss << ar.ram;
    string base_ram = ss.str();
    ss.clear();
    ss.str("");

    ss << ar.flash;
    string base_flash = ss.str();
    ss.clear();
    ss.str("");

    ss << (int)ar.ethernet_num;
    string base_ethnum = ss.str();
    ss.clear();
    ss.str("");

    ss << (int)ar.syncport_num;
    string base_syncnum = ss.str();
    ss.clear();
    ss.str("");

    ss << (int)ar.asyncport_num;
    string base_asyncnum = ss.str();
    ss.clear();
    ss.str("");

    ss << (int)ar.switchport_num;
    string base_switchnum = ss.str();
    ss.clear();
    ss.str("");

    string base_usbnum;
    if (ar.usbport_num == 1) {
        base_usbnum = "存在";
    }
    else {
        base_usbnum = "不存在";
    }

    string base_prnnum;
    if (ar.printport_num == 1) {
        base_prnnum = "存在";
    }
    else {
        base_prnnum = "不存在";
    }
    char sql_command[1024] = "INSERT INTO devstate_base (devstate_base_devid, devstate_base_devno, "\
    "devstate_base_time, devstate_base_ipaddr, devstate_base_sid, devstate_base_type, "\
    "devstate_base_version, devstate_base_cpu, devstate_base_sdram, devstate_base_flash, "\
    "devstate_base_ethnum, devstate_base_syncnum, devstate_base_asyncnum, devstate_base_switchnum, "\
    "devstate_base_usbnum, devstate_base_prnnum, devstate_base_sendreq ) VALUES ";
    string str_values = "( " + base_devid + ", " + base_devno + ", " + base_time + ", \"" + base_ipaddr + "\", \""
        + base_sid + "\", \"" + base_type + "\", \"" + base_version + "\", " + base_cpu + ", " + base_ram
        + ", " + base_flash + ", " + base_ethnum + ", " + base_syncnum + ", " + base_asyncnum  + ", " 
        + base_switchnum + ", \"" + base_usbnum + "\", \"" + base_prnnum + "\", 0 );";
    strcat(sql_command, str_values.c_str());
    record_sql(Level::Info) << "mysql command: " << sql_command;

    if (mysql_query(mysql, sql_command)) {
        record(Level::Error) << "mysql query failed: " << mysql_error(mysql);
        end_connect();
        return ERR_IN;
    }
    end_connect();
    return SUC;
}

SQL_RESULT SQL_Dealer::insert_sysinfo_response_data(const package::sysinfo_response &sr, 
    const Connection &conn)
{
    string base_cpu_used, base_sdram_used;
    if (connect_db() != SUC) {
        record(Level::Fatal) << "from fd: " << conn.sockfd << ", mysql connect failed";
        return ERR_IN;
    }
    float cpu_used = (float)(sr.user_cpu + sr.system_cpu) / (sr.user_cpu + sr.nice_cpu + sr.idle_cpu);
    stringstream ss;
    ss << cpu_used;
    base_cpu_used = ss.str();
    float ram_used = (float)sr.freed_memory / conn.ram / 1024;
    ss.clear();
    ss.str("");
    ss << ram_used;
    base_sdram_used = ss.str();
    char sql_command[1024] = "UPDATE devstate_base SET devstate_base_cpu_used = ";
    strcat(sql_command, base_cpu_used.c_str());
    strcat(sql_command, ", devstate_base_sdram_used = ");
    strcat(sql_command, base_sdram_used.c_str());
    strcat(sql_command, " WHERE devstate_base_devid = ");
    ss.clear();
    ss.str("");
    ss << conn.devid;
    strcat(sql_command, ss.str().c_str());
    strcat(sql_command, ";");
    record_sql(Level::Info) << "sql command: " << sql_command;
    if (mysql_query(mysql, sql_command)) {
        record(Level::Error) << "mysql query failed: " << mysql_error(mysql);
        end_connect();
        return ERR_IN;
    }
    record(Level::Info) << "mysql query succeed.";
    // record(Level::Info) << "query finished.";
    end_connect();
    return SUC;
}

SQL_RESULT SQL_Dealer::insert_configure_response_data(const package::configure_info_response &cr, 
    const Connection &conn)
{
    string base_config = cr.config_buf;
    if (connect_db() != SUC) {
        record(Level::Fatal) << "from fd: " << conn.sockfd << ", mysql connect failed";
        return ERR_IN;
    }
    stringstream ss;
    ss << "UPDATE devstate_base SET devstate_base_config = \"" << base_config <<  "\" WHERE devstate_base_devid = ";
    ss << conn.devid << ";";
    string sql_command = ss.str();
    record_sql(Level::Info) << "sql command: " << sql_command;
    if (mysql_query(mysql, sql_command.c_str())) {
        record(Level::Error) << "mysql query failed: " << mysql_error(mysql);
        end_connect();
        return ERR_IN;
    }
    record(Level::Info) << "mysql query succeed.";
    end_connect();
    return SUC;
}

SQL_RESULT SQL_Dealer::insert_process_response_data(const package::process_info_response &pr, 
    const Connection &conn)
{
    string base_process = pr.process_buf;
    if (connect_db() != SUC) {
        record(Level::Fatal) << "from fd: " << conn.sockfd << ", mysql connect failed";
        return ERR_IN;
    }
    stringstream ss;
    ss << "UPDATE devstate_base SET devstate_base_process = \"" << base_process <<  "\" WHERE devstate_base_devid = ";
    ss << conn.devid << ";";
    string sql_command = ss.str();
    record_sql(Level::Info) << "sql command: " << sql_command;
    if (mysql_query(mysql, sql_command.c_str())) {
        record(Level::Error) << "mysql query failed: " << mysql_error(mysql);
        end_connect();
        return ERR_IN;
    }
    record(Level::Info) << "mysql query succeed.";
    end_connect();
    return SUC;
}

SQL_RESULT SQL_Dealer::insert_ethernet_response_data(const package::ethernet_info_response &er,
    const Connection &conn, short ether_num)
{
    stringstream ss;
    if (connect_db() != SUC) {
        record(Level::Fatal) << "from fd: " << conn.sockfd << ", mysql connect failed";
        return ERR_IN;
    }
    string base_eth_ip, base_eth_mask, base_eth_mac, base_eth_state, base_eth_speed, base_eth_duplex, 
        base_eth_autonego, base_eth_txbytes, base_eth_txpackets, base_eth_rxbytes, base_eth_rxpackets;
    string sql_command = "UPDATE devstate_base SET ";
    string eth_num_str;
    base_eth_ip = String_Dealer::int_to_ipaddr(er.ip_addr);
    base_eth_mask = String_Dealer::int_to_ipaddr(er.subnet_mask);
    // base_eth_mask = er.subnet_mask;
    for (int i = 0; i < 6; i++) {
        ss << (int)er.mac[i];
        if (i != 5)
            ss << ":";
    }
    base_eth_mac = ss.str();
    ss.clear();
    ss.str("");
    if (er.up_down == 1)
        base_eth_state = "up";
    else
        base_eth_state = "down";
    base_eth_speed = (er.options & 0x0001 == 0x0001) ? "100MB" : "10MB";
    base_eth_duplex = (er.options & 0x0002 == 0x0002) ? "全双工" : "半双工";
    base_eth_autonego = (er.options & 0x0004 == 0x0004) ? "是" : "否";
    ss << er.bytes_sent;
    base_eth_txbytes = ss.str();
    ss.clear();
    ss.str("");
    ss << er.pack_sent;
    base_eth_txpackets = ss.str();
    ss.clear();
    ss.str("");
    ss << er.bytes_received;
    base_eth_rxbytes = ss.str();
    ss.clear();
    ss.str("");
    ss << er.pack_received;
    base_eth_rxpackets = ss.str();
    ss.clear();
    ss.str("");
    string devid;
    ss << conn.devid;
    devid = ss.str();
    if (ether_num == package::ethernet_0) {
        eth_num_str = "devstate_base_eth0_";
    }
    else {
        eth_num_str = "devstate_base_eth1_";
    }

    sql_command += eth_num_str + "ip = \"" + base_eth_ip + "\", " + eth_num_str + "mask = \"";
    sql_command += base_eth_mask + "\", " + eth_num_str + "mac = \"" + base_eth_mac + "\", ";
    sql_command += eth_num_str + "state = \"" + base_eth_state + "\", " + eth_num_str + "speed = \"";
    sql_command += base_eth_speed + "\", " + eth_num_str + "duplex = \"" + base_eth_duplex + "\",";
    sql_command += eth_num_str + "autonego = \"" + base_eth_autonego + "\", " + eth_num_str;
    sql_command += "txbytes = " + base_eth_txbytes + ", " + eth_num_str + "txpackets = ";
    sql_command += base_eth_txpackets + ", " + eth_num_str + "rxbytes = " + base_eth_rxbytes;
    sql_command += ", " + eth_num_str + "rxpackets = " + base_eth_rxpackets + " where ";
    sql_command += "devstate_base_devid = " + devid + ";";

    record_sql(Level::Info) << "sql command: " << sql_command;
    if (mysql_query(mysql, sql_command.c_str())) {
        record(Level::Error) << "mysql query failed: " << mysql_error(mysql);
        end_connect();
        return ERR_IN;
    }
    record(Level::Info) << "mysql query succeed.";
    end_connect();

    return SUC;
}

SQL_RESULT SQL_Dealer::insert_usb_available_response_data(const package::usb_available_response &ur,
    const Connection &conn, bool &is_plugin)
{
    
    char is_usb_available = ur.is_usb_plugin;
    string usb_state, devid;
    stringstream ss;
    if (is_usb_available == 1) {
        usb_state = "已插入";
        is_plugin = true;
    }
    else {
        usb_state = "未插入";
        is_plugin = false;
    }
    ss << conn.devid;
    devid = ss.str();
    string sql_command = "UPDATE devstate_base SET devstate_base_usbstate = \"" + usb_state;
    sql_command += "\" WHERE devstate_base_devid = " + devid + ";";
    record_sql(Level::Info) << "sql command: " << sql_command;
    if (connect_db() != SUC) {
        record(Level::Fatal) << "from fd: " << conn.sockfd << ", mysql connect failed";
        return ERR_IN;
    }
    if (mysql_query(mysql, sql_command.c_str())) {
        record(Level::Error) << "mysql query failed: " << mysql_error(mysql);
        end_connect();
        return ERR_IN;
    }
    record(Level::Info) << "mysql query succeed.";
    end_connect();
    return SUC;
}

SQL_RESULT SQL_Dealer::insert_usb_filelist_response_data(const package::usb_filelist_response &ur,
    const Connection &conn)
{
    string base_usbfiles;
    if (connect_db() != SUC) {
        record(Level::Fatal) << "from fd: " << conn.sockfd << ", mysql connect failed";
        return ERR_IN;
    }
    stringstream ss;
    ss << "UPDATE devstate_base SET devstate_base_usbfiles = \"" << ur.usb_filelist_buf << 
        "\" WHERE devstate_base_devid = " << conn.devid << ";";
    string sql_command = ss.str();
    record_sql(Level::Info) << "sql command: " << sql_command;
    if (mysql_query(mysql, sql_command.c_str())) {
        record(Level::Error) << "mysql query failed: " << mysql_error(mysql);
        end_connect();
        return ERR_IN;
    }
    record(Level::Info) << "mysql query succeed.";
    end_connect();
    return SUC;
}

SQL_RESULT SQL_Dealer::insert_printer_info_response_data(const package::printer_info_response &pr,
    const Connection &conn, bool &is_running)
{
    stringstream ss;
    string is_starting;
    if (pr.service_started == 1) {
        is_starting = "已启动";
        is_running = true;
    }
    else {
        is_starting = "未启动";
        is_running = false;
    }
    ss << "UPDATE devstate_base SET devstate_base_prnname = \"" << pr.printer_name;
    ss << "\", devstate_base_prnstate = \"" << is_starting << "\" WHERE devstate_base_devid = ";
    ss << conn.devid << ";";
    string sql_command = ss.str();
    record_sql(Level::Info) << "sql command: " << sql_command;
    if (connect_db() != SUC) {
        record(Level::Fatal) << "from fd: " << conn.sockfd << ", mysql connect failed";
        return ERR_IN;
    }
    if (mysql_query(mysql, sql_command.c_str())) {
        record(Level::Error) << "mysql query failed: " << mysql_error(mysql);
        end_connect();
        return ERR_IN;
    }
    record(Level::Info) << "mysql query succeed.";
    end_connect();
    return SUC;
}

SQL_RESULT SQL_Dealer::insert_printer_queue_response_data(const package::printer_queue_response &pr,
    const Connection &conn)
{
    string base_prnfiles;
    if (connect_db() != SUC) {
        record(Level::Fatal) << "from fd: " << conn.sockfd << ", mysql connect failed";
        return ERR_IN;
    }
    stringstream ss;
    ss << "UPDATE devstate_base SET devstate_base_prnfiles = \"" << pr.queue_info <<
        "\" WHERE devstate_base_devid = " << conn.devid << ";";
    string sql_command = ss.str();
    record_sql(Level::Info) << "sql command: " << sql_command;
    if (mysql_query(mysql, sql_command.c_str())) {
        record(Level::Error) << "mysql query failed: " << mysql_error(mysql);
        end_connect();
        return ERR_IN;
    }
    record(Level::Info) << "mysql query succeed.";
    end_connect();
    return SUC;
}

SQL_RESULT SQL_Dealer::insert_tty_config_response_data(const package::terminal_info_response &tr,
    const Connection &conn)
{
    stringstream ss;
    ss << "UPDATE devstate_base SET devstate_base_ttyconfiged = " << tr.terminal_num
       << " WHERE devstate_base_devid = " << conn.devid << " ;";
    string sql_command = ss.str();
    record_sql(Level::Info) << "sql command: " << sql_command;
    if (connect_db() != SUC) {
        record(Level::Fatal) << "from fd: " << conn.sockfd << ", mysql connect failed";
        return ERR_IN;
    }
    if (mysql_query(mysql, sql_command.c_str())) {
        record(Level::Error) << "mysql query failed: " << mysql_error(mysql);
        end_connect();
        return ERR_IN;
    }
    record(Level::Info) << "mysql query succeed.";
    end_connect();
    return SUC;
}

SQL_RESULT SQL_Dealer::insert_detailed_tty_info_data(const package::detailed_terminal_info_response &dtr,
    const Connection &conn, const package::header &head)
{

    stringstream ss;
    ss << "INSERT INTO devstate_ttyinfo (devstate_ttyinfo_devid, devstate_ttyinfo_devno, "
        << "devstate_ttyinfo_ttyno, devstate_ttyinfo_time, devstate_ttyinfo_readno, "
        << "devstate_ttyinfo_type, devstate_ttyinfo_state, devstate_ttyinfo_ttyip, "
        << "devstate_ttyinfo_scrnum) VALUES (";
    ss << conn.devid << ", " << conn.devno << ", ";
    if (head.detailed_type == package::deaf_terminal) {
        ss << (int)(head.padding_num + 900);
    }
    else {
        ss << (int)(head.padding_num);
    }
    ss << ", now(), " << (int)dtr.configure_port << ", \"" << dtr.terminal_type << "\", ";
    ss << "\"" << dtr.terminal_status << "\", ";
    string ip = String_Dealer::int_to_ipaddr(dtr.terminal_ip);
    ss << "\"" << ip << "\", " << (int)dtr.screen_num << ");";
    string sql_command = ss.str();
    record_sql(Level::Info) << "sql command: " << sql_command;
    if (connect_db() != SUC) {
        record(Level::Fatal) << "from fd: " << conn.sockfd << ", mysql connect failed";
        return ERR_IN;
    }
    if (mysql_query(mysql, sql_command.c_str())) {
        record(Level::Error) << "mysql query failed: " << mysql_error(mysql);
        end_connect();
        return ERR_IN;
    }
    record(Level::Info) << "mysql query succeed.";
    end_connect();
    return SUC;
}

SQL_RESULT SQL_Dealer::insert_screen_info_data(const package::screen_info &si, 
    const Connection &conn, const package::header &head, 
    const package::detailed_terminal_info_response &tr)
{
    stringstream ss;
    ss << "INSERT INTO devstate_scrinfo (devstate_scrinfo_devid, devstate_scrinfo_devno, "
        << "devstate_scrinfo_ttyno, devstate_scrinfo_scrno, devstate_scrinfo_time, "
        << "devstate_scrinfo_is_current, devstate_scrinfo_protocol, devstate_scrinfo_serverip, "
        << "devstate_scrinfo_serverport, devstate_scrinfo_state, devstate_scrinfo_ttytype, "
        << "devstate_scrinfo_tx_server, devstate_scrinfo_rx_server, devstate_scrinfo_tx_terminal, "
        << "devstate_scrinfo_rx_terminal, devstate_scrinfo_ping_min, devstate_scrinfo_ping_avg, "
        << "devstate_scrinfo_ping_max) VALUES (";
    ss << conn.devid << ", " << conn.devno << ", ";
    if (head.detailed_type == package::deaf_terminal) {
        ss << (int)(head.padding_num + 900);
    }
    else {
        ss << (int)(head.padding_num);
    }
    ss << ", " << (int)si.screen_num << ", now(), ";
    if (si.screen_num == tr.current_active_screen_num + 1) {
        ss << "\"*\", "; 
    }
    else {
        ss << "NULL, ";
    }
    ss << "\"" << si.screen_protocol << "\", ";
    string remote_ip = String_Dealer::int_to_ipaddr(si.remote_server_ip);
    ss << "\"" << remote_ip << "\", " << si.remote_server_tcp_port << ", "
       << "\"" << si.screen_status << "\", \"" << si.screen_terminal_type << "\", "
       << si.bytes_remote_server_sent << ", " << si.bytes_remote_server_received << ", "
       << si.bytes_terminal_sent << ", " << si.bytes_terminal_received << ", "
       << si.ping_pack_min << ", " << si.ping_pack_avg << ", " << si.ping_pack_max << ");";
    
    string sql_command = ss.str();
    record_sql(Level::Info) << "sql command: " << sql_command;
    if (connect_db() != SUC) {
        record(Level::Fatal) << "from fd: " << conn.sockfd << ", mysql connect failed";
        return ERR_IN;
    }
    if (mysql_query(mysql, sql_command.c_str())) {
        record(Level::Error) << "mysql query failed: " << mysql_error(mysql);
        end_connect();
        return ERR_IN;
    }
    record(Level::Info) << "mysql query succeed.";
    end_connect();
    return SUC;
}

SQL_RESULT SQL_Dealer::insert_tty_connected_data(int tty_connected, const Connection &conn)
{
    stringstream ss;
    ss << "UPDATE devstate_base SET devstate_base_tty_connected = " << tty_connected
        << " WHERE devstate_base_devid = " << conn.devid << " ;";
    string sql_command = ss.str();
    record_sql(Level::Info) << "sql_command: " << sql_command;
    if (connect_db() != SUC) {
        record(Level::Fatal) << "from fd: " << conn.sockfd << ", mysql connect failed";
        return ERR_IN;
    }
    if (mysql_query(mysql, sql_command.c_str())) {
        record(Level::Error) << "mysql query failed: " << mysql_error(mysql);
        end_connect();
        return ERR_IN;
    }
    record(Level::Info) << "mysql query succeed.";
    end_connect();
    return SUC;
}