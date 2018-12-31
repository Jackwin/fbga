#include <arpa/inet.h>
#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "network.h"

int EthernetClient(int * data_ptr)
{

    char *IP_addr = "192.168.3.28";
    printf("ehternet.\n");
    //设置一个socket地址结构client_addr,代表客户机internet地址, 端口
    struct sockaddr_in client_addr;
    bzero(&client_addr, sizeof(client_addr)); //把一段内存区的内容全部设置为0
    client_addr.sin_family = AF_INET;    //internet协议族
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);//INADDR_ANY表示自动获取本机地址
    client_addr.sin_port = htons(0);    //0表示让系统自动分配一个空闲端口
    //创建用于internet的流协议(TCP)socket,用client_socket代表客户机socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if ( client_socket < 0)
    {
        printf("Create Socket Failed!\n");
        exit(1);
    }
    //把客户机的socket和客户机的socket地址结构联系起来
    if ( bind(client_socket, (struct sockaddr*)&client_addr, sizeof(client_addr)))
    {
        printf("Client Bind Port Failed!\n");
        exit(1);
    }

    //设置一个socket地址结构server_addr,代表服务器的internet地址, 端口
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if (inet_aton(IP_addr, &server_addr.sin_addr) == 0) //服务器的IP地址来自程序的参数
    {
        printf("Server IP Address Error!\n");
        exit(1);
    }
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);
    socklen_t server_addr_length = sizeof(server_addr);
    //向服务器发起连接,连接成功后client_socket代表了客户机和服务器的一个socket连接
    if (connect(client_socket, (struct sockaddr*)&server_addr, server_addr_length) < 0)
    {
        printf("Can Not Connect To %s!\n", IP_addr);
        exit(1);
    }

    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
//    char *string = "Hello, server.\n";
    memcpy(buffer, data_ptr, BUFFER_SIZE);
    //  strncpy(buffer, file_name, strlen(file_name)>BUFFER_SIZE?BUFFER_SIZE:strlen(file_name));
    //向服务器发送buffer中的数据
    send(client_socket, buffer, BUFFER_SIZE, 0);

    char file_name[FILE_NAME_MAX_SIZE + 1];
    bzero(file_name, FILE_NAME_MAX_SIZE + 1);
    char *name = "data.txt";
    memcpy(file_name, name, strlen(name));
    printf("File name is %s.\n", file_name);
//    file_name = "data.txt";
    FILE * fp = fopen(file_name, "w");
    if (NULL == fp )
    {
        printf("File:\t%s Can Not Open To Write\n", file_name);
        exit(1);
    }

    //从服务器接收数据到buffer中
    bzero(buffer, BUFFER_SIZE);
    int length = 0;
    while (length = recv(client_socket, buffer, BUFFER_SIZE, 0))
    {
        if (length < 0)
        {
            printf("Recieve Data From Server %s Failed!\n", IP_addr);
            break;
        }
//        int write_length = write(fp, buffer,length);
        int write_length = fwrite(buffer, sizeof(char), length, fp);
        if (write_length < length)
        {
            printf("File:\t%s Write Failed\n", file_name);
            break;
        }
        bzero(buffer, BUFFER_SIZE);
    }
    printf("Recieve File:\t %s From Server[%s] Finished\n", file_name, IP_addr);

    fclose(fp);
    //关闭socket
    close(client_socket);
    return 0;
}
