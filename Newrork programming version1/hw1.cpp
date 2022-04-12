#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include<string>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <strings.h>
#include<vector>
#include <map>
#include <sys/mman.h>
#include<algorithm>


using namespace std;
class account{
    public:
        char name[2000];
        char password[2000];

        char msg[2000][2000];
        bool valid[2000];
        int time[2000];
        char source[2000][2000];
        int time_cnt;
        int msg_num;
    account(char* n,char* p){
        memset(&name,0,sizeof(name));
        memset(&password,0,sizeof(password));
        strncpy(name,n,strlen(n));  // only zero teminated char[] can use strlen 
        strncpy(password,p,strlen(p));
        memset(&msg,0,sizeof(msg));
        memset(&valid,0,sizeof(valid));
        memset(&time,0,sizeof(time));
        memset(&source,0,sizeof(source));
        time_cnt=0;
        msg_num=0;
        
    }
    account(){}
};


class mymap{
    public:
    char source[2000];
    int cnt;
    mymap(char* s,int n){
        memset(&source,0,sizeof(source));
        strncpy(source,s,strlen(s));
        cnt=n;
    } 
    mymap(){}
};
bool compare(mymap a,mymap b){
    string tempa(a.source,strlen(a.source));
    string tempb(b.source,strlen(b.source));
    return tempa<tempb;
} 
int main(int argc, char *argv[]){
    //create shared memory
    int port;
    if(argc!=2){
        cout<<"server usage: ./hw1 [port number]" <<endl;
        return -1;
    }
    else{
        string temp(argv[1]);
        port = stoi(temp);
    }
    int *user_cnt=static_cast<int*>(mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE,MAP_ANONYMOUS|MAP_SHARED,-1,0));
    account *account_arr= static_cast<account*>(mmap(NULL,sizeof(account)*2000,PROT_READ | PROT_WRITE,MAP_ANONYMOUS|MAP_SHARED,-1,0));
    //cout<<"1: "<<MAP_ANONYMOUS<<endl;
    //cout<<"2: "<<MAP_SHARED<<endl;
    //cout<<"3: "<< (MAP_SHARED|MAP_ANONYMOUS ) <<endl;
    *user_cnt=0;
    struct sockaddr_in server_addr;
    int listenfd,connfd,pid,status;
    char buff[1234];
    //string buff;
    listenfd=socket(AF_INET, SOCK_STREAM, 0);

    if(listenfd<0){
        cout<<"socket error";
    }
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY); //wildcard 32-b zero
    server_addr.sin_port= htons(port);
    status=bind(listenfd, (sockaddr*) &server_addr, sizeof(server_addr));
    if(status<0){
        perror("bind error");
    }
    status=listen(listenfd,15);
    if(status<0){
        cout<<"listen error"<<endl;
    }
    while(1){
        //cout<<"waiting for connection"<<endl;
        connfd=accept(listenfd,(sockaddr*) NULL,NULL); //block accept connection
        //cout<<"connection found"<<endl;
        pid=fork();
        if(pid==0){
             
            //child
            close(listenfd);//child close listening
            // doit 
            write(connfd, "********************************\n", 34);
            write(connfd, "** Welcome to the BBS server. **\n", 34);
            write(connfd, "********************************\n", 34);
            write(connfd, "% ",2);
            bool exit_flg=0;   //for prompt %
            bool islogin=false;   //login state
            int user_id=-1;     //current login user
            memset(&buff,0,sizeof(buff));     //clear buff
            while(read(connfd, buff, sizeof(buff))){
                if(buff[0]=='\n'){
                    write(connfd, "% ",2);
                    continue;
                }
                vector <char*> token_vec;
                token_vec.clear();
                char *token_1=strtok(buff,"\n");    //take out the \n 
                char *token=strtok(token_1," ");   //split by " "
                while(token!=NULL){
                    token_vec.push_back(token);
                    token=strtok(NULL," ");
                }
                //register
                if( strcmp(token_vec[0],"register\0")==0 ){  
                    //correct input
                    if(token_vec.size()==3){
                        //username available
                        bool available=true;
                        for(int i=0;i<(*user_cnt);i++){
                            if(strcmp( (account_arr+i)->name,token_vec[1])==0){
                                available=false;
                            }
                        }
                        if(available){
                            write(connfd, "Register successfully.\n",24);
                            account temp(token_vec[1],token_vec[2]);
                            *(account_arr+(*user_cnt))=temp;
                            (*user_cnt)+=1;
                        }
                        //username already token
                        else{
                            write(connfd, "Username is already used.\n",27);
                        }
                    }
                    //incorrect input format
                    else{
                        write(connfd, "Usage: register <username> <password>\n",39);
                    }
                }
                else if(strcmp(token_vec[0],"login\0")==0){
                    //correct input format
                    if(token_vec.size()==3){
                        //not login yet
                        if(!islogin){
                            // find match username and correct passord
                            bool exist=false;
                            int i=0;
                            for(;i<(*user_cnt);i++){
                                if(strcmp( (account_arr+i)->name,token_vec[1])==0){
                                    exist=true;
                                    break;    //need to break, otherwise i++
                                }
                            }
                            if(exist && strcmp( (account_arr+i)->password,token_vec[2])==0){   
                                int len_token=strlen(token_vec[1]);
                                string temp_token(token_vec[1],len_token);
                                string temp_s="Welcome, "+ temp_token+".\n";
                                int len_final=temp_s.length();
                                const char * c=temp_s.c_str();
                                write(connfd, c,len_final);
                                islogin=true;
                                user_id=i;
                            }
                            //login failed
                            else{
                                write(connfd, "login failed.\n",15);
                            }
                        }
                        //already login, need to logout first
                        else{
                            write(connfd, "Please logout first.\n",22);
                        }
                    }
                    //incorrect input format
                    else{
                        write(connfd, "Usage: login <username> <password>\n",36);
                    }
                }
                //logout
                else if(strcmp(token_vec[0],"logout\0")==0){
                    if(islogin){
                        int len_token=strlen((account_arr+user_id)->name);
                        string temp_token((account_arr+user_id)->name,len_token);
                        string temp_s="Bye, "+ temp_token+".\n";
                        int len_final=temp_s.length();
                        const char * c=temp_s.c_str();
                        write(connfd, c,len_final);
                        islogin=false;
                        user_id=-1;
                    }
                    else{
                        write(connfd,"please login first.\n",21);
                    }
                }
                //whoami
                else if(strcmp(token_vec[0],"whoami\0")==0){
                    if(islogin){
                        int len_token=strlen((account_arr+user_id)->name);//need () outside of account_arr+user_i
                        string temp_token((account_arr+user_id)->name,len_token); //char* to string
                        string temp_s=temp_token+"\n";
                        int len_final=temp_s.length();
                        const char * c=temp_s.c_str();//string to char*
                        write(connfd, c,len_final);
                    }
                    else{
                        write(connfd,"please login first.\n",21);
                    }
                }
                //list-user
                else if(strcmp(token_vec[0],"list-user\0")==0){
                    string temp_s="";
                    vector<string> str_vec;
                    str_vec.clear();
                    for(int i=0;i<*user_cnt;i++){
                        int len_token=strlen((account_arr+i)->name);
                        string temp_token((account_arr+i)->name,len_token);
                        temp_token+="\n";
                        str_vec.push_back(temp_token);
                    }
                    sort(str_vec.begin(),str_vec.end());
                    for(int i=0;i<str_vec.size();i++){
                        temp_s+=str_vec[i];
                    }
                    int len_final=temp_s.length();
                    const char * c=temp_s.c_str();//string to char*
                    write(connfd, c,len_final);
                }
                //exit
                else if(strcmp(token_vec[0],"exit\0")==0){
                    if(islogin){
                        int len_token=strlen((account_arr+user_id)->name);//need () outside of account_arr+user_i
                        string temp_token((account_arr+user_id)->name,len_token); //char* to string
                        string temp_s="Bye, "+temp_token+".\n";
                        int len_final=temp_s.length();
                        const char * c=temp_s.c_str();//string to char*
                        write(connfd, c,len_final);
                    }
                    exit_flg=1;
                    break;
                }
                else if(strcmp(token_vec[0],"send\0")==0){
                    
                    int last_token_index=token_vec.size()-1;
                    int last_quatation_index=strlen(token_vec[last_token_index])-1;

                    if(token_vec.size()<3){ ///wrong format
                         write(connfd, "Usage: send <username> <message>\n",34);
                    }
                    //correct format
                    else if( token_vec[2][0]=='\"' && token_vec[last_token_index][last_quatation_index]=='\"'){
                        //combine token_vec[2],token_vec[3].....
                        string msg_str;
                        msg_str.clear();
                        cout<<"buf:"<<msg_str<<endl;
                        for(int i=0;i<token_vec.size()-2;i++){
                            string temp(token_vec[i+2],strlen(token_vec[i+2]));
                            if(i+2==2 ||i+2==token_vec.size()-1){
                                temp.replace(temp.find("\""),1,"");
                                int index=temp.find("\"");
                                if(index!=-1){
                                    temp.replace(index,1,"");
                                }
                            }
                            
                            temp+=" ";
                            msg_str+=temp;
                            cout<<"buf:"<<msg_str<<endl;
                        }
                        const char* msg=msg_str.c_str();
                        cout<<"buf->msg"<<msg<<endl;
            
                       // cout<<msg<<endl;
                        //need to login first
                        if(!islogin){
                            write(connfd,"please login first.\n",21);
                        }
                        else{//correct format and login
                            bool exist=false;
                            int i=0;
                            for(;i<(*user_cnt);i++){
                                if(strcmp( (account_arr+i)->name,token_vec[1])==0){
                                    exist=true;
                                    break;    //need to break, otherwise i++
                                }
                            }
                        //user doesn't exist
                            if(!exist){
                                write(connfd,"User not existed.\n",19);
                            }
                        //user exist
                            else{
                                //current login user(source): user_id
                                //target userid: i
                                int slot=0;//find available slot in target's msgbox
                                for(;slot<2000;slot++){
                                    if( (account_arr+i)->valid[slot]==0){
                                        break;
                                    }
                                }
                                //send msg to target
                                (account_arr+i)->valid[slot]=1;
                                memset(&(account_arr+i)->msg[slot],0,sizeof((account_arr+i)->msg[slot]));
                                memset(&(account_arr+i)->source[slot],0,sizeof((account_arr+i)->source[slot]));

                                strncpy((account_arr+i)->msg[slot],msg,strlen(msg));
                                strncpy((account_arr+i)->source[slot],(account_arr+user_id)->name,strlen((account_arr+user_id)->name));
                                ((account_arr+i)->msg_num)+=1;
                                (account_arr+i)->time[slot]=(account_arr+i)->time_cnt;
                                ((account_arr+i)->time_cnt)+=1;
                            }
                            //test send
                            for(int j=0;j<(account_arr+i)->msg_num;j++){
                                if((account_arr+i)->valid[j]){
                                    cout<<"msg:"<<(account_arr+i)->msg[j]<<" source: "<<(account_arr+i)->source[j]<<" time: "<<(account_arr+i)->time[j]<<endl;
                                }
                            }  
                            cout<<"-----------------------------"<<endl;
                        }
                    }
                    else{//wrong format
                         write(connfd, "Usage: send <username> <message>\n",34);
                    }
                    
                }
                //list-msg
                else if(strcmp(token_vec[0],"list-msg\0")==0){
                    if(islogin){
                        vector<mymap> map_vec;
                        //current login user:user_id
                        //iterate thru msgbox of current user
                        for(int i=0;i<2000;i++){
                            //slot has msg
                            if((account_arr+user_id)->valid[i]){
                                //if the source of this msg is already in map =>map[source]+=1
                                bool exist=0;
                                for(int k=0;k<map_vec.size();k++){
                                    if(strcmp( (account_arr+user_id)->source[i], map_vec[k].source)==0){
                                        map_vec[k].cnt++;
                                        exist=1;
                                        break;
                                    }
                                }
                                //source not in map yet, create one 
                                if(!exist){
                                    mymap temp((account_arr+user_id)->source[i],1);
                                    map_vec.push_back(temp);
                                }
                            }
                        }
                        //sort the vec alphabetically
                        sort(map_vec.begin(),map_vec.end(),compare);
                        //test
                        /*    for(int i=0;i<map_vec.size();i++){
                            cout<<map_vec[i].cnt<<"msg from "<<map_vec[i].source<<endl;
                        } */
                        if(map_vec.size()==0){
                            write(connfd, "Your message box is empty.\n",28);
                        }
                        else{
                            for(int i=0;i<map_vec.size();i++){
                                int len_token2=strlen(map_vec[i].source);//need () outside of account_arr+user_i
                                string temp_token2(map_vec[i].source,len_token2); //char* to string
                                string temp_s=to_string(map_vec[i].cnt)+" message from "+temp_token2+".\n";
                                int len_final=temp_s.length();
                                const char * c=temp_s.c_str();//string to char*
                                write(connfd, c,len_final);
                            }
                        }
                    }
                    else{
                        write(connfd,"Please login first.\n",21);
                    }
                }
                else if(strcmp(token_vec[0],"receive\0")==0){
                    if(token_vec.size()==2){
                         if(islogin){
                            bool exist=0;
                            int min_time=9999999;
                            int index=0;
                            bool user_exist=0;
                            for(int i=0;i<2000;i++){
                                if( strcmp( (account_arr+i)->name, token_vec[1]) ==0 ){
                                    user_exist=1;
                                }
                            }
                            for(int i=0;i<2000;i++){
                                if( (account_arr+user_id)->valid[i] && strcmp( (account_arr+user_id)->source[i], token_vec[1])==0 && (account_arr+user_id)->time[i]<=min_time){
                                    index=i;
                                    min_time=(account_arr+user_id)->time[i];
                                    exist=1;
                                }
                            }
                            if(exist){
                                int len_token2=strlen((account_arr+user_id)->msg[index]);//need () outside of account_arr+user_i
                                string temp_token2((account_arr+user_id)->msg[index],len_token2); //char* to string
                                string temp_s=temp_token2+"\n";
                                int len_final=temp_s.length();
                                const char * c=temp_s.c_str();//string to char*
                                write(connfd, c,len_final);
                                (account_arr+user_id)->valid[index]=false;
                                for(int j=0;j<(account_arr+user_id)->msg_num;j++){
                                    if((account_arr+user_id)->valid[j]){
                                        cout<<"msg:"<<(account_arr+user_id)->msg[j]<<" source: "<<(account_arr+user_id)->source[j]<<" time: "<<(account_arr+user_id)->time[j]<<endl;
                                    }
                                }  
                                cout<<"-----------------------------"<<endl;
                            }
                            else if(!user_exist){
                                write(connfd,"User not existed.\n",19);
                            }
                        }
                        else{   //  needed????????????????????????????????? 
                            write(connfd,"Please login first.\n",21);
                        }
                    }else{
                        write(connfd,"Usage: receive <username>\n",27);
                    }
                       
                }

                if(!exit_flg){
                    write(connfd, "% ",2);
                }
                memset(&buff,0,sizeof(buff));
            }
            memset(&buff,0,sizeof(buff));
            close(connfd);
            exit(0);
        }
        else{ // parent
           // wait(NULL);
            close(connfd);
        }
    }
}