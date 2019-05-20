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
const int GOOD_NAME = 1;
const int BAD_NAME = 0;
const int MAX_LEN_OF_NAME = 16;
const int htns = 5556;
const int TIMEOUT=5;

struct usr_info{
    char name[16];
    int desk;
    int if_name_is_good;
};

void print_char_to_all_except(char* info,int fd,int n_clients, struct usr_info our_arr[N]) {
    for (int i = 0; i < n_clients; i++) {
        if ((fd != our_arr[i].desk) && (our_arr[i].if_name_is_good == GOOD_NAME)) {
            write(our_arr[i].desk, info, 1);
        }
    }
    printf("%c", *info);
}

void delete_usr(struct usr_info our_arr[N], int j,int n_clients){
    int i = j;
    for (;i<n_clients-1;i++){
        for(int k = 0;k< MAX_LEN_OF_NAME;k++){
            our_arr[i].name[k]=our_arr[i+1].name[k];
        }
        our_arr[i].desk = our_arr[i+1].desk;
        our_arr[i].if_name_is_good = our_arr[i+1].if_name_is_good;
    }
    our_arr[n_clients-1].if_name_is_good = BAD_NAME;
}

int len_of_name(const char name[MAX_LEN_OF_NAME]){
    int i;
    for(i=0; i < MAX_LEN_OF_NAME; i++){
        if (name[i] == 0){
            break;
        }
    }
    return i+1;
}

int read_name(char NAME[MAX_LEN_OF_NAME], int fd){
// reads a name from the file descriptor. if client is absent returns -1, if the name is too big, erases the rest, returns name without \n => bad name
    int i = 0;
    char b = 0;
    while(b!='\n'){
        if(read(fd,&b,sizeof(char))==0){
            return -1;
        }
        NAME[i]=b;
        i++;
        if ((i == MAX_LEN_OF_NAME)&&(b != '\n')){
            while(b!='\n'){
                read(fd,&b,sizeof(char));
            }
        }
    }
    return i;
}

int name_is_good(char NAME[MAX_LEN_OF_NAME]){
    int i = 0;
    for(i = 0; i < MAX_LEN_OF_NAME; i++){
        if (((NAME[i] < 'a') | (NAME[i] > 'z')) && ((NAME[i] < 'A') | (NAME[i] > 'Z')) && (NAME[i] != '_') && (NAME[i] != '\n') && (NAME[i] != 13)) {
            return BAD_NAME;
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
    if ((i >= 2)&&(i < MAX_LEN_OF_NAME)){
        return GOOD_NAME;
    }else{
        return BAD_NAME;
    }
}

int name_is_name(struct usr_info our_arr[N],int n_clients,int number){
    for(int i = 0; i < n_clients; i++){
        if ((strcmp(our_arr[i].name,our_arr[number].name)==0)&&(number!=i)){
            return BAD_NAME;
        }
    }
    return GOOD_NAME;
}

int main(){

    char b;
    char buff[7];
    char bye[6]="Bye!\n\0";
    char hello[4] = "Hi!\n";
    char writename[20] = "& What's your name?\n";
    char badwrite[52] = " Too.. too many clients for me to handle, senpai..\n\0";
    char Welcome[32] = "& Welcome to the server-party!\n\0";
    char New[25]=" (^.^)/ Newfag is here!\n\0";
    char Old[31]=" (~.~) zzz Gnight. have to go\n\0";
    char name_is[43] = " That is NOT your name! (please change it)\n";
    char Doubledots[3]=": \0";

    struct usr_info our_arr[N];
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;
    for(int i = 0;i<N;i++){
        our_arr[i].if_name_is_good = BAD_NAME;
    }
    int n_clients = 0;
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
            printf("%s%d\n","& Everything is OK. Connected clients = \0",n_clients);
            timeout.tv_sec = TIMEOUT;
            timeout.tv_usec = 0;
            FD_ZERO(&fds);
            FD_SET(sockfd,&fds);
            for(int i=0;i < n_clients;i++){
                FD_SET(our_arr[i].desk,&fds);
            }
        }
        if FD_ISSET(sockfd, &fds){
            if (n_clients < N){
                our_arr[n_clients].desk = accept(sockfd, NULL, NULL);
                if (our_arr[n_clients].desk == -1) {
                    perror("accept error");
                }
                if (our_arr[n_clients].desk > maxsockfd) {
                    maxsockfd = our_arr[n_clients].desk;
                }
                write(our_arr[n_clients].desk,hello,4);
                write(our_arr[n_clients].desk,writename,20);
                n_clients+=1;
            }else{
                badsockfd = accept(sockfd,NULL,NULL);
                write(badsockfd,badwrite,52);
                shutdown(badsockfd,2);
                close(badsockfd);
            }
        }
        for (int j = 0; j < n_clients; j++) {
            if FD_ISSET(our_arr[j].desk, &fds) {
                if (our_arr[j].if_name_is_good == GOOD_NAME) {
                    if (read(our_arr[j].desk, &b, 1) == 0) {
// message about the client exit
                        for (int i = 0; i < n_clients; i++) {
                            if ((i != j) && (our_arr[i].if_name_is_good = GOOD_NAME)) {
                                write(our_arr[i].desk, our_arr[j].name, len_of_name(our_arr[j].name));
                                write(our_arr[i].desk, Old, 31);
                                write(our_arr[i].desk, hello + 10, 1);//\n
                            }
                        }
                        printf("%s%s\n", our_arr[j].name, Old);
// delete user j
                        shutdown(our_arr[j].desk, 2);
                        close(our_arr[j].desk);
                        delete_usr(our_arr, j, n_clients);
                        n_clients -= 1;
                        j--;
                    }else {
                        if (b != '\r') {
                            if (b != '\n') {
                                for (int i = 0; i < 6; i++) {
                                    buff[i] = b;
                                    read(our_arr[j].desk, &b, 1);
                                    if ((b == '\r') && (i + 1 < 6)) {
                                        buff[i + 1] = 0;
                                        break;
                                    }
                                }
                                if (strcmp(buff, bye) == 0) {
// message about the client exit
                                    for (int i = 0; i < n_clients; i++) {
                                        if ((i != j) && (our_arr[i].if_name_is_good == GOOD_NAME)) {
                                            write(our_arr[i].desk, our_arr[j].name, len_of_name(our_arr[j].name));
                                            write(our_arr[i].desk, Old, 31);
                                            write(our_arr[i].desk, hello + 10, 1);//\n
                                        }
                                    }
                                    printf("%s%s\n", our_arr[j].name, Old);
// delete him!
                                    shutdown(our_arr[j].desk, 2);
                                    close(our_arr[j].desk);
                                    delete_usr(our_arr, j, n_clients);
                                    n_clients -= 1;
                                    j--;
                                } else {
                                    printf("%s: ", our_arr[j].name);
                                    for (int i = 0; i < n_clients; i++) {
                                        if ((i != j) && (our_arr[i].if_name_is_good == GOOD_NAME)) {
                                            write(our_arr[i].desk, our_arr[j].name, len_of_name(our_arr[j].name));
                                            write(our_arr[i].desk, Doubledots, 3);
                                        }
                                    }
                                    buff[6] = 0;
                                    int i = 0;
                                    while (buff[i] != 0) {
                                        print_char_to_all_except(&buff[i], our_arr[j].desk, n_clients, our_arr);
                                        i++;
                                    }
                                    while (b != '\n') {
                                        if (b != '\r') {
                                            print_char_to_all_except(&b, our_arr[j].desk, n_clients, our_arr);
                                        }
                                        read(our_arr[j].desk, &b, 1);
                                    }
                                    print_char_to_all_except(&b, our_arr[j].desk, n_clients, our_arr);
                                }
                            }
                        }
                    }
                } else {
                    if (read_name(our_arr[j].name, our_arr[j].desk) == -1) {
// message about the client exit
                        if (our_arr[j].if_name_is_good == GOOD_NAME) {
                            for (int i = 0; i < n_clients; i++) {
                                if ((i != j) && (our_arr[i].if_name_is_good == GOOD_NAME)) {
                                    write(our_arr[i].desk, our_arr[j].name, len_of_name(our_arr[j].name));
                                    write(our_arr[i].desk, Old, 31);
                                    write(our_arr[i].desk, hello + 10, 1);//\n
                                }
                            }
                            printf("%s%s\n", our_arr[j].name, Old);
                        }
// delete user j
                        shutdown(our_arr[j].desk, 2);
                        close(our_arr[j].desk);
                        delete_usr(our_arr, j, n_clients);
                        n_clients -= 1;
                        j--;
                    } else {
                        our_arr[j].if_name_is_good = name_is_good(our_arr[j].name);
                        if (our_arr[j].if_name_is_good == BAD_NAME) {
                            write(our_arr[j].desk, writename, 20);
                        } else {
                            if (name_is_name(our_arr,n_clients,j) == GOOD_NAME) {
                                write(our_arr[j].desk, Welcome, 32);
// tell everyone: new user is here
                                for (int i = 0; i < n_clients; i++) {
                                    if ((i != j) && (our_arr[i].if_name_is_good == GOOD_NAME)) {
                                        write(our_arr[i].desk, New, 25);
                                        write(our_arr[i].desk, our_arr[j].name, len_of_name(our_arr[j].name));
                                        write(our_arr[i].desk, hello + 10, 1);//\n
                                    }
                                }
                                printf("%s%s\n", New, our_arr[j].name);
                            }else{
                                our_arr[j].if_name_is_good = BAD_NAME;
                                write(our_arr[j].desk,name_is,43);
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
        for(int i=0;i < n_clients;i++){
            FD_SET(our_arr[i].desk,&fds);
        }
    }
    return 0;
}