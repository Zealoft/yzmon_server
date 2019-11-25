#pragma once

#include <arpa/inet.h>

namespace package {

/*
 * detailed package type information
 * same from server and client
 */
const char src_server = 0x11;
const char src_client = 0x91;
const char authen = 0x01;
const char minimum_version = 0x00;
const char sys_info = 0x02;
const char configure_info = 0x03;
const char process_info = 0x04;
const char ethernet_info = 0x05;
const char usb_available = 0x07;
const char usb_filelist = 0x0c;
const char printer_info = 0x08;
const char printer_queue_info = 0x0d;
const char terminal_info = 0x09;
const char deaf_terminal = 0x0a;
const char ip_terminal = 0x0b;

const char end_connect = 0xff;

const short ethernet_0 = 0x0000;
const short ethernet_1 = 0x0001;


struct header {
    char src_end;
    char detailed_type;
    short length;       // 报文的总长度
    short padding_num;  
    short data_length;     // 报文的数据长度

    void ntoh() {
        length = ntohs(length);
        padding_num = ntohs(padding_num);
        data_length = ntohs(data_length);
    }
    void hton() {
        length = htons(length);
        padding_num = htons(padding_num);
        data_length = htons(data_length);
    }
};

struct authentication_request {
    short major_version;
    char minor_version1;
    char minor_version2;
    short device_connect_interval;
    short device_sampling_interval;
    char empty_console_allowed;
    char pad[3];
    char authentication_string[32];
    int random_num;
    int svr_time;

    void hton() {
        major_version = htons(major_version);
        device_connect_interval = htons(device_connect_interval);
        device_sampling_interval = htons(device_sampling_interval);
        random_num = htonl(random_num);
        svr_time = htonl(svr_time);
    }

    void ntoh() {
        major_version = ntohs(major_version);
        device_connect_interval = ntohs(device_connect_interval);
        device_sampling_interval = ntohs(device_sampling_interval);
        random_num = ntohl(random_num);
        svr_time = ntohl(svr_time);
    }
};

struct minimum_version_need {
    ushort major_version;
    char minor_version1;
    char minor_version2;

    void hton() {
        major_version = htons(major_version);
    }

    void ntoh() {
        major_version = ntohs(major_version);
    }
};

struct authentication_response {
    short cpu_clock;
    short ram;
    short flash;
    short inside_serial;
    char group_num[16];
    char device_type[16];
    char device_version[16];
    char ethernet_num;
    char syncport_num;
    char asyncport_num;
    char switchport_num;
    char usbport_num;
    char printport_num;
    short padding0;
    int device_agency_num;
    char device_inside_num;
    char padding1[3];
    char authen_str[32];
    int random_num;

    void hton() {
        cpu_clock = htons(cpu_clock);
        ram = htons(ram);
        flash = htons(flash);
        inside_serial = htons(inside_serial);
        device_agency_num = htonl(device_agency_num);
        // random_num = htonl(random_num);
    }
    void ntoh() {
        cpu_clock = ntohs(cpu_clock);
        ram = ntohs(ram);
        flash = ntohs(flash);
        inside_serial = ntohs(inside_serial);
        device_agency_num = ntohl(device_agency_num);
        // random_num = ntohl(random_num);
    }
};

struct sysinfo_response {
    int user_cpu;
    int nice_cpu;
    int system_cpu;
    int idle_cpu;
    int freed_memory;

    void ntoh() {
        user_cpu = ntohl(user_cpu);
        nice_cpu = ntohl(nice_cpu);
        system_cpu = ntohl(system_cpu);
        idle_cpu = ntohl(idle_cpu);
        freed_memory = ntohl(freed_memory);
    }
};

/*
 * 配置信息包长度不定
 * 因此解析时要先取长度，再取对应的字节数
 */
struct configure_info_response {
    char config_buf[8192];

    void ntoh() {

    }
};

struct process_info_response {
    char process_buf[8192];

    void ntoh() {

    }
};

struct ethernet_info_response {
    char is_exist;
    char is_set;
    char up_down;
    char pad;
    char mac[6];
    short options;
    int ip_addr;
    int subnet_mask;
    int ip_addr_sub1;
    int subnet_mask_sub1;
    int ip_addr_sub2;
    int subnet_mask_sub2;
    int ip_addr_sub3;
    int subnet_mask_sub3;
    int ip_addr_sub4;
    int subnet_mask_sub4;
    int ip_addr_sub5;
    int subnet_mask_sub5;

    int bytes_received;
    int pack_received;
    int errorpack_received;
    int throwpack_received;
    int fifopack_received;
    int frame_received;
    int compresspack_received;
    int broadpack_received;

    int bytes_sent;
    int pack_sent;
    int errorpack_sent;
    int throwpack_sent;
    int fifopack_sent;
    int frame_sent;
    int compresspack_sent;
    int broadpack_sent;

    void ntoh() {
        options = ntohs(options);
        ip_addr = ntohl(ip_addr);
        subnet_mask = ntohl(subnet_mask);
        ip_addr_sub1 = ntohl(ip_addr_sub1);
        subnet_mask_sub1 = ntohl(subnet_mask_sub1);
        ip_addr_sub2 = ntohl(ip_addr_sub2);
        subnet_mask_sub2 = ntohl(subnet_mask_sub2);
        ip_addr_sub3 = ntohl(ip_addr_sub3);
        subnet_mask_sub3 = ntohl(subnet_mask_sub3);
        ip_addr_sub4 = ntohl(ip_addr_sub4);
        subnet_mask_sub4 = ntohl(subnet_mask_sub4);
        ip_addr_sub5 = ntohl(ip_addr_sub5);
        subnet_mask_sub5 = ntohl(subnet_mask_sub5);

        bytes_received = ntohl(bytes_received);
        pack_received = ntohl(pack_received);
        errorpack_received = ntohl(errorpack_received);
        throwpack_received = ntohl(throwpack_received);
        fifopack_received = ntohl(fifopack_received);
        frame_received = ntohl(frame_received);
        compresspack_received = ntohl(compresspack_received);
        broadpack_received = ntohl(broadpack_received);
        
        bytes_sent = ntohl(bytes_sent);
        pack_sent = ntohl(pack_sent);
        errorpack_sent = ntohl(errorpack_sent);
        throwpack_sent = ntohl(throwpack_sent);
        fifopack_sent = ntohl(fifopack_sent);
        frame_sent = ntohl(frame_sent);
        compresspack_sent = ntohl(compresspack_sent);
        broadpack_sent = ntohl(broadpack_sent);
    }
};

struct usb_available_response {
    char is_usb_plugin;
    char pad[3];

    void ntoh() {

    }
};

struct usb_filelist_response {
    char usb_filelist_buf[4096];

    void ntoh() {

    }
};


struct printer_info_response {
    char service_started;
    char pad;
    short queue_jobs;
    char printer_name[32];

    void ntoh() {
        queue_jobs = ntohs(queue_jobs);
    }
};

struct printer_queue_response {
    char queue_info;

    void ntoh() {

    }
};

struct terminal_info_response {
    char deaf_terminal_used[16];
    char ip_terminal_used[254];
    short terminal_num;

    void ntoh() {
        terminal_num = ntohs(terminal_num);
    }
};

struct detailed_terminal_info_response {
    char port;
    char configure_port;
    char current_active_screen_num;
    char screen_num;
    int terminal_ip;
    char terminal_type[12];
    char terminal_status[8];

    void ntoh() {
        terminal_ip = ntohl(terminal_ip);
    }
};

struct screen_info {
    char screen_num;
    char pad;
    short remote_server_tcp_port;
    int remote_server_ip;
    char screen_protocol[12];
    char screen_status[8];
    char screen_string[24];
    char screen_terminal_type[12];
    int terminal_connect_time;
    int bytes_terminal_sent;
    int bytes_terminal_received;
    int bytes_remote_server_sent;
    int bytes_remote_server_received;
    int ping_pack_min;
    int ping_pack_avg;
    int ping_pack_max;

    void ntoh() {
        remote_server_tcp_port = ntohs(remote_server_tcp_port);
        remote_server_ip = ntohl(remote_server_ip);
        terminal_connect_time = ntohl(terminal_connect_time);
        bytes_terminal_sent = ntohl(bytes_terminal_sent);
        bytes_terminal_received = ntohl(bytes_terminal_received);
        bytes_remote_server_sent = ntohl(bytes_remote_server_sent);
        bytes_remote_server_received = ntohl(bytes_remote_server_received);
        ping_pack_min = ntohl(ping_pack_min);
        ping_pack_avg = ntohl(ping_pack_avg);
        ping_pack_max = ntohl(ping_pack_max);
    }
};


} // namespace package

