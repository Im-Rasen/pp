#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include <malloc.h>

const int N = 5;
const int GOODNAME = 1;
const int BADNAME = 0;
const int MAXLENOFNAME = 16;
const int htns = 5556;
const int TIMEOUT=5;

struct usr_info{
    char name[16];
    int desk;
    int ifnameisgood;
};

void printchartoallexcept(char* info,int fd,int nclients, struct usr_info ourarr[N]) {
    for (int i = 0; i < nclients; i++) {
        if ((fd != ourarr[i].desk) && (ourarr[i].ifnameisgood == GOODNAME)) {
            write(ourarr[i].desk, info, 1);
        }
    }
    printf("%c", *info);
}

void delusr(struct usr_info ourarr[N], int j,int nclients){
    int i = j;
    for (;i<nclients-1;i++){
        for(int k = 0;k< MAXLENOFNAME;k++){
            ourarr[i].name[k]=ourarr[i+1].name[k];
        }
        ourarr[i].desk = ourarr[i+1].desk;
        ourarr[i].ifnameisgood = ourarr[i+1].ifnameisgood;
    }
    ourarr[nclients-1].ifnameisgood = BADNAME;
}

int lenofname(const char name[MAXLENOFNAME]){
    int i;
    for(i=0; i < MAXLENOFNAME; i++){
        if (name[i] == 0){
            break;
        }
    }
    return i+1;
}

int readname(char NAME[MAXLENOFNAME], int fd){
//читает имя из файлового дескриптора, причем, если клиент ушел возвращает -1, если имя слишком большое затирает остаток, возвращает имя без \n => имя плохое
    int i = 0;
    char b = 0;
    while(b!='\n'){
        if(read(fd,&b,sizeof(char))==0){
            return -1;
        }
        NAME[i]=b;
        i++;
        if ((i == MAXLENOFNAME)&&(b != '\n')){
            while(b!='\n'){
                read(fd,&b,sizeof(char));
            }
        }
    }
    return i;
}

int nameisgood(char NAME[MAXLENOFNAME]){
//проверка имени на годность, убивает \n
    int i = 0;
    for(i = 0; i < MAXLENOFNAME; i++){
        if (((NAME[i] < 'a') | (NAME[i] > 'z')) && ((NAME[i] < 'A') | (NAME[i] > 'Z')) && (NAME[i] != '_') && (NAME[i] != '\n') && (NAME[i] != 13)) {
            return BADNAME;
        }
        if (NAME[i] == '\r') {
            NAME[i] = 0;
            break;
        }

        if (NAME[i] == '\n') {
            NAME[i] = 0;
            break;
        }
    }
    if ((i >= 2)&&(i < MAXLENOFNAME)){
        return GOODNAME;
    }else{
        return BADNAME;
    }
}

int nameisname(struct usr_info ourarr[N],int nclients,int number){
    for(int i = 0; i < nclients; i++){
        if ((strcmp(ourarr[i].name,ourarr[number].name)==0)&&(number!=i)){
            return BADNAME;
        }
    }
    return GOODNAME;
}

int main(){

    char b;
    char buff[7];
    char bye[6]="bye!.\0";
    char privet[11] = "Даров\n";
    char writename[28] = "Как тебя звать?\n";
    char badwrite[89] = "Простите, многовато клиентов, всех не обслужить\n\0";
    char Dobrop[51] = "Добро пожаловать на сервер\n\0";
    char New[59]="#Пришел новенький, знакомьтесь: \0";
    char Old[92]=" не выдержал давления с вашей стороны и решил уйти\0";
    char nameis[36] = "Прости, имя занято!!\n";
    char Doubledots[3]=": \0";

    struct usr_info ourarr[N];
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;
    for(int i = 0;i<N;i++){
        ourarr[i].ifnameisgood = BADNAME;
    }
    int nclients = 0;
    fd_set fds;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1){
        perror("socket error");
        return 1;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(htns);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sockfd,(struct sockaddr*) &addr, sizeof(addr)) == -1){
        perror("bind error");
        return 1;
    }
    if (listen(sockfd,5) == -1){
        perror("listen error");
        return 1;
    }
    int maxsockfd = sockfd;
    int badsockfd;
    FD_ZERO(&fds);
    FD_SET(sockfd, &fds);
    while(1) {
        while (select(maxsockfd + 1, &fds, 0, 0, &timeout) == 0) {
            printf("%s%d\n","#Все ок, количество подключенных: \0",nclients);
            timeout.tv_sec = TIMEOUT;
            timeout.tv_usec = 0;
            FD_ZERO(&fds);
            FD_SET(sockfd,&fds);
            for(int i=0;i < nclients;i++){
                FD_SET(ourarr[i].desk,&fds);
            }
        }
        if FD_ISSET(sockfd, &fds){
            if (nclients < N){
                ourarr[nclients].desk = accept(sockfd, NULL, NULL);
                if (ourarr[nclients].desk == -1) {
                    perror("accept error");
                }
                if (ourarr[nclients].desk > maxsockfd) {
                    maxsockfd = ourarr[nclients].desk;
                }
                write(ourarr[nclients].desk,privet,11);
                write(ourarr[nclients].desk,writename,28);
                nclients+=1;
            }else{
                badsockfd = accept(sockfd,NULL,NULL);
                write(badsockfd,badwrite,90);
                shutdown(badsockfd,2);
                close(badsockfd);
            }
        }
        for (int j = 0; j < nclients; j++) {
            if FD_ISSET(ourarr[j].desk, &fds) {
                if (ourarr[j].ifnameisgood == GOODNAME) {
                    if (read(ourarr[j].desk, &b, 1) == 0) {
//рассылка про то что челик сбежал
                        for (int i = 0; i < nclients; i++) {
                            if ((i != j) && (ourarr[i].ifnameisgood = GOODNAME)) {
                                write(ourarr[i].desk, ourarr[j].name, lenofname(ourarr[j].name));
                                write(ourarr[i].desk, Old, 91);
                                write(ourarr[i].desk, privet + 10, 1);//тупо \n
                            }
                        }
                        printf("%s%s\n", ourarr[j].name, Old);
//полное удаление пользователя j
                        shutdown(ourarr[j].desk, 2);
                        close(ourarr[j].desk);
                        delusr(ourarr, j, nclients);
                        nclients -= 1;
                        j--;
                    }else {
                        if (b != '\r') {
                            if (b != '\n') {
                                for (int i = 0; i < 6; i++) {
                                    buff[i] = b;
                                    read(ourarr[j].desk, &b, 1);
                                    if ((b == '\r') && (i + 1 < 6)) {
                                        buff[i + 1] = 0;
                                        break;
                                    }
                                }
                                if (strcmp(buff, bye) == 0) {
//рассылка про побег челика
                                    for (int i = 0; i < nclients; i++) {
                                        if ((i != j) && (ourarr[i].ifnameisgood == GOODNAME)) {
                                            write(ourarr[i].desk, ourarr[j].name, lenofname(ourarr[j].name));
                                            write(ourarr[i].desk, Old, 91);
                                            write(ourarr[i].desk, privet + 10, 1);//тупо \n
                                        }
                                    }
                                    printf("%s%s\n", ourarr[j].name, Old);
//удаление убогого
                                    shutdown(ourarr[j].desk, 2);
                                    close(ourarr[j].desk);
                                    delusr(ourarr, j, nclients);
                                    nclients -= 1;
                                    j--;
                                } else {
                                    printf("%s: ", ourarr[j].name);
                                    for (int i = 0; i < nclients; i++) {
                                        if ((i != j) && (ourarr[i].ifnameisgood == GOODNAME)) {
                                            write(ourarr[i].desk, ourarr[j].name, lenofname(ourarr[j].name));
                                            write(ourarr[i].desk, Doubledots, 2);
                                        }
                                    }
                                    buff[6] = 0;
                                    int i = 0;
                                    while (buff[i] != 0) {
                                        printchartoallexcept(&buff[i], ourarr[j].desk, nclients, ourarr);
                                        i++;
                                    }
                                    while (b != '\n') {
                                        if (b != '\r') {
                                            printchartoallexcept(&b, ourarr[j].desk, nclients, ourarr);
                                        }
                                        read(ourarr[j].desk, &b, 1);
                                    }
                                    printchartoallexcept(&b, ourarr[j].desk, nclients, ourarr);
                                }
                            }
                        }
                    }
                } else {
                    if (readname(ourarr[j].name, ourarr[j].desk) == -1) {
//рассылка про то что челик сбежал
                        if (ourarr[j].ifnameisgood == GOODNAME) {
                            for (int i = 0; i < nclients; i++) {
                                if ((i != j) && (ourarr[i].ifnameisgood == GOODNAME)) {
                                    write(ourarr[i].desk, ourarr[j].name, lenofname(ourarr[j].name));
                                    write(ourarr[i].desk, Old, 91);
                                    write(ourarr[i].desk, privet + 10, 1);//тупо \n
                                }
                            }
                            printf("%s%s\n", ourarr[j].name, Old);
                        }
//полное удаление клиента j
                        shutdown(ourarr[j].desk, 2);
                        close(ourarr[j].desk);
                        delusr(ourarr, j, nclients);
                        nclients -= 1;
                        j--;
                    } else {
                        ourarr[j].ifnameisgood = nameisgood(ourarr[j].name);
                        if (ourarr[j].ifnameisgood == BADNAME) {
                            write(ourarr[j].desk, writename, 28);
                        } else {
                            if (nameisname(ourarr,nclients,j) == GOODNAME) {
                                write(ourarr[j].desk, Dobrop, 50);
//разослать всем, что пришел новенький
                                for (int i = 0; i < nclients; i++) {
                                    if ((i != j) && (ourarr[i].ifnameisgood == GOODNAME)) {
                                        write(ourarr[i].desk, New, 58);
                                        write(ourarr[i].desk, ourarr[j].name, lenofname(ourarr[j].name));
                                        write(ourarr[i].desk, privet + 10, 1);//тупо \n
                                    }
                                }
                                printf("%s%s\n", New, ourarr[j].name);
                            }else{
                                ourarr[j].ifnameisgood = BADNAME;
                                write(ourarr[j].desk,nameis,36);
                            }
                        }
                    }
                }
            }
        }
        timeout.tv_sec = TIMEOUT;
        timeout.tv_usec = 0;
        FD_ZERO(&fds);
        FD_SET(sockfd,&fds);
        for(int i=0;i < nclients;i++){
            FD_SET(ourarr[i].desk,&fds);
        }
    }
    return 0;
}