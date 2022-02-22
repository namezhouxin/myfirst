/*===============================================
 *   文件名称：server.c
 *   创 建 者：zhouxin     
 *   创建日期：2022年01月12日
 *   描    述：
 ================================================*/
#include <stdio.h>
#include <pthread.h>
#include <strings.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sqlite3.h>
void *pthread_handle(void *client);
char *order_check(char *word, int* flag);
char *order_register(char *account, char *password);
void order_exit(int fd);
char *order_error();
char *order_login(char *account, char *password);
char *order_change(char *account, char *password, char *newpassword);
struct message 
{
    int flag ;
    int type;
    char account[20];
    char password[20];
    char word[20];
    char ret[1024];
    char newpassword[20];
};
typedef struct client_info
{
    int fd;
    char addr[20];
    int port;
}client_info;
int flag = 0;
int main(int argc, char *argv[])
{ 
    //创建套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {  
        perror("socket");
        exit(-1);
    }
    //绑定本机IP和端口
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    saddr.sin_port = htons(8888);
    int ret = bind(sockfd, (struct sockaddr*)&saddr, sizeof(saddr));
    if(ret < 0)
    {
        perror("bind");
        exit(-1);
    }
    //设置监听套接字
    ret = listen(sockfd, 5);
    if(ret < 0)
    {
        perror("listen");
        exit(-1);
    }
    printf("等待客户端连接中...\n");
    //等待连接
    int connfd ;
    struct sockaddr_in caddr;
    int c_len = sizeof(caddr);
    client_info c_info;
    while(1)
    {
        connfd = accept(sockfd, (struct sockaddr*)&caddr, &c_len);
        if(connfd < 0)
        {
            perror("accept");
            exit(-1);
        }
        pthread_t thread;
        c_info.fd = connfd;
        strcpy(c_info.addr , inet_ntoa(caddr.sin_addr));
        c_info.port = ntohs(caddr.sin_port);
        ret = pthread_create(&thread, NULL, pthread_handle, &c_info);
        if(ret < 0)
        {
            perror("pthread_create");
            exit(-1);
        }
        pthread_detach(thread);
    }  
    close(sockfd);
    return 0;
} 
//线程处理函数
void *pthread_handle(void *client)
{
    int ret;
    client_info c_info = *(client_info *)client;
    struct message mes;
    char * p = NULL;
    printf("ip = %s, port = %d, fd = %d\n", c_info.addr, c_info.port, c_info.fd);
    while(1)
    {     
        memset(mes.ret, 0, sizeof(mes.ret));
        memset(mes.word, 0, sizeof(mes.word));
        memset(mes.password, 0, sizeof(mes.password));
        memset(mes.account, 0, sizeof(mes.account));
        ret = read(c_info.fd,(struct message*)&mes, sizeof(mes));
        memset(mes.ret, 0, sizeof(mes.ret));
        if(ret < 0)
        {
            perror("read");
            break;
        }
        if(ret == 0)
        {
            printf("ip为%s的客户端已断开连接!\n",c_info.addr);
            break;
        }
        switch(mes.type)
        {
            case 0:order_exit(c_info.fd);break;
            case 1:p = order_register( mes.account, mes.password),strcpy(mes.ret,p),p = NULL;break;
            case 2:p = order_login(mes.account, mes.password), strcpy(mes.ret,p), p = NULL;break;
            case 3:p = order_check(mes.word, &mes.flag), strcpy(mes.ret, p), p = NULL;break;
            case 7:p = order_change(mes.account, mes.password, mes.newpassword), strcpy(mes.ret, p), p = NULL;break;
            default:p =  order_error(), strcpy(mes.ret, p), p = NULL;
        } 
        ret = write(c_info.fd, mes.ret, strlen(mes.ret));
        if(ret < 0)
        {
            perror("write");
            break;
        }
    } 
    close(c_info.fd);
    pthread_exit;
}
//查询单词功能函数
char *order_check(char *word, int* flag)
{
    if(*flag != 1)
    {
        char *ret = "未登录，请登录后再查询!\n";
        return ret;
    }
    sqlite3 *db = NULL;
    char *errmsg = NULL;
    sqlite3_open("dict.db", &db);
    char sql[1024] ;
    char **result = NULL;
    char *a = NULL;
    char *b = NULL;
    char *ret = NULL;
    memset(sql, 0, sizeof(sql));
    int hang,lie;
    sprintf(sql,"select mean from dict where word='%s';", word);
    sqlite3_get_table(db, sql, &result, &hang, &lie, &errmsg);
    sqlite3_close(db);
    if(hang == 0 && lie == 0)
    {
        a = strcat(word," | ");
        ret = strcat(a, "无法查询该单词!\n");
        return ret;
    }
    a = strcat(word,"|");
    b = strcat(a, result[1]);
    ret = strcat(b, "\n");
    return ret;

}
//错误命令函数
char *order_error()
{
    char *ret = "错误命令!\n";
    return ret;
}
//注册功能函数
char *order_register(char *account, char *password)
{
    sqlite3 *db = NULL;
    char *errmsg = NULL;
    sqlite3_open("dict.db", &db);
    char sql[1024] ;
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "select password from user where account='%s';",account);
    int hang,lie;
    char **result = NULL;
    sqlite3_get_table(db, sql, &result, &hang, &lie, &errmsg);
    if(hang == 0 && lie == 0)
    {
        memset(sql, 0, sizeof(sql));
        sprintf(sql,"insert into user values('%s', '%s');", account, password);
        sqlite3_exec(db, sql, NULL, NULL, &errmsg);
        sqlite3_close(db);
        char *ret = "注册成功!\n";
        return ret;
    }
    else
    {
        sqlite3_close(db);
        char *ret = "该用户名已被注册!\n";
        return ret;

    }
}
//登录功能函数
char *order_login(char *account,char *password)
{   
    sqlite3 *db = NULL;
    char *errmsg = NULL;
    sqlite3_open("dict.db", &db);
    char sql[1024] ;
    char **result = NULL;
    memset(sql, 0, sizeof(sql));
    sprintf(sql,"select password from user where account='%s';", account);
    int hang,lie;
    int ret1 = sqlite3_get_table(db, sql, &result, &hang, &lie, &errmsg); 
    if(hang == 0 && lie == 0)
    {
        char * ret = "登录失败，没有这个用户!\n";
        return ret;
    }
    int ret2 = strcmp(password,result[1]);
    if(ret2 != 0)
    {
        char * ret = "密码错误!\n";
        return ret;
    } 
    sqlite3_close(db);
    char *ret = "登录成功!\n";
    return ret;

}
//修改密码功能函数
char *order_change(char *account,char *password,char *newpassword)
{   
    sqlite3 *db = NULL;
    char *errmsg = NULL;
    sqlite3_open("dict.db", &db);
    char sql[1024] ;
    memset(sql, 0, sizeof(sql));
    sprintf(sql,"select password from user where account='%s';", account);
    char **result = NULL;
    int hang,lie;
    int ret1 = sqlite3_get_table(db, sql, &result, &hang, &lie, &errmsg); 
    if(hang == 0 && lie == 0)
    {
        char * ret = "修改失败，用户名不正确!\n";
        return ret;
    }
    int ret2 = strcmp(password,result[1]);
    if(ret2 != 0)
    {
        char * ret = "修改失败，密码错误!\n";
        return ret;
    }
    memset(sql, 0, sizeof(sql));
    sprintf(sql,"update user set password='%s' where account='%s';", newpassword, account);
    sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    sqlite3_close(db);
    char *ret = "修改密码成功!请重新登录\n";
    return ret;

}
//退出功能函数
void order_exit(int fd)
{
    close(fd);
    pthread_exit;
}
