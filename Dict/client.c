/*===============================================
 *   文件名称：client.c
 *   创 建 者：zhouxin     
 *   创建日期：2022年01月12日
 *   描    述：
 ================================================*/
#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
/*函数声明*/
void think(int flag, int *symbol);
void sign(int flag, int sockfd);
int login(int flag, int sockfd);
int check(int flag, int sockfd);
void history(int flag);
void clean(int flag);
void help(int flag);
int change(int flag, int sockfd);
char *hiddlen(char password[20]);
/*信息结构体*/
struct message 
{
    int flag;             //登录标志位
    int type;             //操作选项
    char account[20];     //帐号
    char password[20];    //密码
    char word[20];        //单词
    char ret[1024];        //返回单词释义和操作消息
    char newpassword[20]; //修改密码所需的新密码
};
/*主函数*/
int main(int argc, char *argv[])
 { 
    /*创建流式套接字，协议族选择ipv4*/
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    { 
        perror("socket");
        exit(-1);
    } 
    /*向服务器发起连接*/
    struct sockaddr_in saddr;          //结构体saddr用于存放<协议族> <服务器地址> <端口号>
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    saddr.sin_port = htons(8888);
    int ret = connect(sockfd, (struct sockaddr*)&saddr, sizeof(saddr));   //发起连接
    if(ret < 0)
    { 
        perror("connect");
        exit(-1);
    }
    if(ret == 0)
    {    
        printf("连接服务器成功!\n");
    }
    /*进入初始化界面*/
    help(1);
    /*获取操作选项，执行对应功能函数*/
    int opt;
    struct message msg;
    char buf[100];
    int symbol = 0;                          //符号标志位用于判断查询操作，以便进行多次查询
    while(1)
    {  
        if(msg.flag == 1 && symbol == 0)    //当登录标志位为1代表已登录，登录后立即进入查询界面
        { 
            symbol = check(msg.flag, sockfd);
        } 
        else
        {  
            memset(buf, 0, sizeof(buf));
            printf("\n#操作选项:");
            if(scanf("%d", &opt) != 1)      //输入非法字符，立即退出
            {
                break;
            }
            if(opt == 0)                   //操作选项0退出
            { 
                break;
            } 
            switch(opt)
            {
                case 1:sign(msg.flag, sockfd);break;
                case 2:msg.flag = login(msg.flag, sockfd);break;
                case 3:think(msg.flag, &symbol);break;
                case 4:history(msg.flag);break;
                case 5:clean(msg.flag);break;
                case 6:help(msg.flag);break;
                case 7:msg.flag = change(msg.flag, sockfd);break;
                default:printf("操作选项错误!\n");
            }  
        }
    }       
    close(sockfd);
    return 0;

}
//注册
void sign(int flag, int sockfd)
{
    if(flag == 1)
    {
        printf("已登录，无法注册!\n");
        return;
    } 
    printf("\n/*******************注册界面*******************/\n");
    struct message msg;
    char buf[100] = {0};
    char password[20] = {0};
    char password1[20] = {0};
    char password2[20] = {0};
    msg.type = 1;
    printf("请输入帐号:");
    scanf("%s", msg.account);
    printf("请输入密码:");
    strcpy(password1,hiddlen(password));
    memset(password, 0, sizeof(password));
    printf("请确认密码:");
    strcpy(password2,hiddlen(password));

    if(strcmp(password1,password2) != 0)
    {
        printf("两次输入的密码不匹配！注册失败\n\n");
        return;
    }
    strcpy(msg.password, password2);
    write(sockfd, (struct message*)&msg, sizeof(msg));
    memset(buf, 0, sizeof(buf));
    read(sockfd, buf, sizeof(buf));
    printf("%s\n",buf);
}
//登录
int login(int flag, int sockfd)
{
    if(flag == 1)
     {
        printf("已登录!无法登录\n");
        return 1;
    }
    printf("\n/*******************登录界面*******************/\n");
    struct message msg;
    char buf[100];
    char password[20];
    msg.type = 2;
    printf("请输入帐号:");
    scanf("%s", msg.account);
    printf("请输入密码:");
    strcpy(msg.password,hiddlen(password));
    int ret = write(sockfd, (struct message*)&msg, sizeof(msg));
    if(ret < 0)
    {
        perror("write");
        exit(-1);
    }
    memset(buf, 0, sizeof(buf));
    ret = read(sockfd, buf, sizeof(buf));
    if(ret < 0)
    {
        perror("read");
        exit(-1);
    }
    printf("%s\n", buf);
    if(strcmp(buf, "登录成功!\n") == 0)
    { 
        return 1;
    } 
    return 0;
}
//查询
int check(int flag, int sockfd)
{
    if(flag != 1)
    {
        printf("未登录，请先登录后执行操作！\n");
        return 0;
    }
    printf("\n/*******************查询界面*******************/\n");
    char buf[100];
    struct message msg;
    memset(msg.word, 0, sizeof(msg.word));
    msg.type = 3;
    msg.flag = 1;
    printf("输入#退出查询\n");
    printf("输入单词:");
    scanf("%s", msg.word);
    if(strcmp(msg.word,"#") == 0)
    {
        return 1;
    }  
    int ret = write(sockfd, (struct message*)&msg, sizeof(msg));
    if(ret < 0)
    {
        perror("write");
        exit(-1);
    }
    memset(buf, 0, sizeof(buf));
    ret = read(sockfd, buf, sizeof(buf));
    if(ret < 0)
    {
        perror("read");
        exit(-1);
    }
    printf("查询结果：%s\n", buf);
    FILE * fp = fopen("history.txt", "a");
    if(fp == NULL)
    {
        perror("fopen");
        exit(-1);
    }
    fputs(buf, fp);
    fclose(fp);
    return 0;
}
//修改密码
int change(int flag, int sockfd)
{
    if(flag != 1)
    {
        printf("未登录，请先登录后执行操作！\n");
        return 0;
    }
    printf("\n/*****************修改密码界面*****************/\n");
    struct message msg;
    char buf[100];
    char password1[20];
    char password2[20];
    char password[20];
    msg.type = 7;
    printf("请输入帐号:");
    scanf("%s", msg.account);
    printf("请输入原密码:");
    memset(password, 0, sizeof(password));
    strcpy(msg.password,hiddlen(password));
    printf("输入新密码:");
    memset(password, 0, sizeof(password));
    strcpy(password1,hiddlen(password));
    printf("确认新密码:");
    memset(password, 0, sizeof(password));
    strcpy(password2,hiddlen(password));

    if(strcmp(password1,password2) != 0)
    {
        printf("两次输入的密码不匹配！修改密码失败\n");
        return 1;
    } 
    strcpy(msg.newpassword, password2);
    int ret = write(sockfd, (struct message*)&msg, sizeof(msg));
    if(ret < 0)
    {
        perror("write");
        exit(-1);
    } 
    memset(buf, 0, sizeof(buf));
    ret = read(sockfd, buf, sizeof(buf));
    if(ret < 0)
    {
        perror("read");
        exit(-1);
    } 
    printf("%s\n", buf);
    if(strcmp(buf, "修改密码成功!请重新登录\n") == 0)
    {
        return 0;
    }
    return 1;
}

//历史查询
void history(int flag)
{
    if(flag != 1)
    {
        printf("未登录，请先登录后执行操作！\n");
        return;
    }
    printf("\n/*****************历史查询界面*****************/\n");
    FILE *fp = fopen("history.txt", "r");
    char buf2[100];
    while(1)
    {
        memset(buf2, 0, sizeof(buf2));
        if(fgets(buf2, sizeof(buf2), fp) != NULL)
        {
            fputs(buf2, stdout);
        }
        else
        {
            break;
        }
    } 
    fclose(fp);

}
//清空历史
void clean(int flag)
{
    if(flag != 1)
    {
        printf("未登录，请先登录后执行操作！\n");
        return;
    }
    FILE *fp = fopen("history.txt", "w+");
    if(fp == NULL)
    {
        perror("fopen");
        exit(-1);
    }
    int ret = fseek(fp, 0, SEEK_SET);
    if (ret < 0)
    {
        perror("fseek");
        exit(-1);
    }
    fclose(fp);
    printf("已清空历史查询记录！\n");
}
//重复查询
void think(int flag, int *symbol)
{
    if(flag != 1)
    {
        printf("未登录，请先登录后执行操作！\n");
        return;
    }
    else
    {
        *symbol = 0;
    }
}
//帮助
void help(int flag)
{
    if(flag != 1)
    {
        printf("未登录，请先登录后执行操作！\n");
        return;
    }
    printf("/*************************************/\n");
    printf("*-------------*词典查询系统*----------*\n");
    printf("*-------------**************----------*\n");
    printf("*操作选项如下：                       *\n");
    printf("*---------------- 0 :退出-------------*\n");
    printf("*---------------- 1 :注册-------------*\n");
    printf("*---------------- 2 :登录-------------*\n");
    printf("*---------------- 3 :查询-------------*\n");
    printf("*---------------- 4 :查询历史记录-----*\n");
    printf("*---------------- 5 :清空历史记录-----*\n");
    printf("*---------------- 6 :帮助手册---------*\n");
    printf("*---------------- 7 :修改密码---------*\n");
    printf("/*************************************/\n");
}
//隐藏输入
char * hiddlen(char password[20])
{
    system("stty -echo");
    scanf("%s", password);
    system("stty echo");
    printf("\n");
    return password;
}
