
#include<arpa/inet.h>
#include<stdio.h>
#include <poll.h>
#include <unistd.h>//for close ,read
#include <ctime>

#include <signal.h>
#include<vector>
#include<iostream>
#include<queue>
#include<string.h>
#include<ctype.h>
#include<sstream>
#include<map>
#include <iomanip>
#include<errno.h>

using namespace std;

#define buf_size 4096
#define client_max 12
//#define debug debug

class Base64{
private:
    string _base64_table;
    static const char b64_p = '=';public:
    Base64()
    {
        _base64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    }

    string Encode(const  char * str,int bytes);
    string Decode(const char *str,int bytes);
    void Debug(bool open = true);
};

string Base64::Encode(const  char * str,int bytes) {
    int num = 0,bin = 0,i;
    string en_res;
    const  char * cur;
    cur = str;
    while(bytes > 2) {
        en_res += _base64_table[cur[0] >> 2];
        en_res += _base64_table[((cur[0] & 0x03) << 4) + (cur[1] >> 4)];
        en_res += _base64_table[((cur[1] & 0x0f) << 2) + (cur[2] >> 6)];
        en_res += _base64_table[cur[2] & 0x3f];

        cur += 3;
        bytes -= 3;
    }
    if(bytes > 0)
    {
        en_res += _base64_table[cur[0] >> 2];
        if(bytes%3 == 1) {
            en_res += _base64_table[(cur[0] & 0x03) << 4];
            en_res += "==";
        } else if(bytes%3 == 2) {
            en_res += _base64_table[((cur[0] & 0x03) << 4) + (cur[1] >> 4)];
            en_res += _base64_table[(cur[1] & 0x0f) << 2];
            en_res += "=";
        }
    }
    return en_res;
}
string Base64::Decode(const char *str,int length) {
    const char table_[] =
    {
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -1, -1, -2, -2, -1, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -1, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 62, -2, -2, -2, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -2, -2, -2, -2, -2, -2,
        -2,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -2, -2, -2, -2, -2,
        -2, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2
    };
    int bin = 0,i=0,pos=0;
    std::string de_res;
    const char *cur = str;
    char ch;
    while( (ch = *cur++) != '\0' && length-- > 0 )
    {
        if (ch == b64_p) {
            if (*cur != '=' && (i % 4) == 1) {
                return NULL;
            }
            continue;
        }
        ch = table_[ch];
        if (ch < 0 ) {
            continue;
        }
        switch(i % 4)
        {
            case 0:
                bin = ch << 2;
                break;
            case 1:
                bin |= ch >> 4;
                de_res += bin;
                bin = ( ch & 0x0f ) << 4;
                break;
            case 2:
                bin |= ch >> 2;
                de_res += bin;
                bin = ( ch & 0x03 ) << 6;
                break;
            case 3:
                bin |= ch;
                de_res += bin;
                break;
        }
        i++;
    }
    return de_res;
}

class discript_status{
public:
    bool islogin, is_in_room;
    int user_id,udp_port;

    discript_status(){}
    discript_status(bool is,int id,int udp_port,bool room){
        this->islogin=is;
        this->user_id=id;
        this->udp_port=udp_port;
        this->is_in_room = room;

    }
};
class account{
public:
        string name;
        string password;
        int version;
        int violation;
    account(string n,string p,int version,int violation){
        this->name=n;
        this->password=p;
        this->version=version;
        this->violation=violation;
    }
    account(){}
};



//global var
vector<account>account_arr;
vector<discript_status> status_vec;
//record who(which sockaddr) is in the room
map<int,struct sockaddr_in>sock_in_room;
vector<string>chat_history;
string filter_list[9]={"how","you","or","pek0","tea","ha","kon","pain","Starburst Stream"};
struct pollfd		client[client_max];

void register_(FILE* fp,vector<string> token_vec ){
    if(token_vec.size()==3){
        //username available
        bool available=true;
        for(int i=0;i<account_arr.size();i++){
            if( account_arr[i].name==token_vec[1]){
                available=false;
            }
        }

        if(available){
            fprintf(fp,"%s","Register successfully.\n% ");
            account temp(token_vec[1],token_vec[2],-1,0);
            account_arr.push_back(temp);
        }
        //username already token
        else{
            fprintf(fp,"%s","Username is already used.\n% ");
        }
                     //incorrect input format
    }
    else{
        fprintf(fp,"%s","Usage: register <username> <password>\n% ");
    }
    return;
}

void login(FILE* fp,vector<string> token_vec,int cli_id ){
    if(token_vec.size()==3){
        //not login yet
        cout<<"login cliid"<<cli_id<<endl;

        if(!status_vec[cli_id].islogin  ){
            // find match username and correct passord

            bool exist=false;
            int i=0;

            for(;i<account_arr.size();i++){
                if(account_arr[i].name==token_vec[1]){
                    exist=true;
                    break;    //need to break, otherwise i++
                }
            }
            bool other_login=0;
            for(int j=0;j<client_max;j++){
                if(status_vec[j].user_id==i){
                    other_login=1;
                    break;
                }
            }
            if(other_login){
                fprintf(fp,"%s","Please logout first.\n% ");
            }else{
                if(exist){
                    if( account_arr[i].violation>=3){
                        string temp="";
                        temp+=("We don't welcome "+token_vec[1]+"!\n% ");
                        int len=temp.length();
                        int sockfd = client[cli_id].fd;
                        int err=write(sockfd,temp.c_str(),len);
                    }else if(account_arr[i].password==token_vec[2]){
                        fprintf(fp,"%s%s%s","Welcome, ",account_arr[i].name.c_str(),".\n% ");
                        status_vec[cli_id].islogin=true;
                        status_vec[cli_id].user_id=i;
                    }else{
                        fprintf(fp,"%s","Login failed.\n% ");

                    }
                    /* if( account_arr[i].password==token_vec[2] &&account_arr[i].violation<3){
                        fprintf(fp,"%s%s%s","Welcome, ",account_arr[i].name.c_str(),".\n% ");
                        status_vec[cli_id].islogin=true;
                        status_vec[cli_id].user_id=i;
                    } */
                }else{
                    fprintf(fp,"%s","Login failed.\n% ");

                }
            }




        }
        //already login, need to logout first
        else{
            fprintf(fp,"%s","Please logout first.\n% ");
        }
    }
    //incorrect input format
    else{
        fprintf(fp,"%s", "Usage: login <username> <password>\n% ");
    }

    return;
}

void logout(FILE* fp,vector<string> token_vec,int cli_id){
    if(token_vec.size()!=1){
         fprintf(fp,"%s","Usage: logout\n% ");
    }else{
        if(status_vec[cli_id].islogin){
            fprintf(fp,"%s%s%s","Bye, ",account_arr[status_vec[cli_id].user_id].name.c_str(),".\n% ");


            map<int,struct sockaddr_in>::iterator it;
            //leave chat room after logout
            for(it=sock_in_room.begin();it!=sock_in_room.end();it++){
                cout<<"finding "<<endl;
                cout<<status_vec[cli_id].user_id<<"   compare w "<<it->first<<endl;
                if(status_vec[cli_id].user_id==it->first){
                    cout<<"found it erase : "<< it->first<<endl;
                    sock_in_room.erase(it->first);
                            int res=sock_in_room.erase(it->first);
                    cout<<"erase re: "<<res<<endl;
                    break;
                }
            }


            status_vec[cli_id].islogin=false;
            status_vec[cli_id].user_id=-1;

        }
        else{
            fprintf(fp,"%s","Please login first.\n% ");
        }
    }

    return;
}

void exit_(FILE* fp,vector<string> token_vec,int cli_id ){
    if(token_vec.size()!=1){
        fprintf(fp,"%s","Usage: exit\n% ");

    }else{
        if(status_vec[cli_id].islogin){
            fprintf(fp,"%s%s%s","Bye, ",account_arr[status_vec[cli_id].user_id].name.c_str(),".\n");


            //leave chat room after logout
            map<int,struct sockaddr_in>::iterator it;
            for(it=sock_in_room.begin();it!=sock_in_room.end();it++){
                if(status_vec[cli_id].user_id==it->first){
                    cout<<"found it erase : "<< it->first<<endl;

                    int res=sock_in_room.erase(it->first);
                    cout<<"erase re: "<<res<<endl;
                    break;

                }
            }


            status_vec[cli_id].islogin=0;
            status_vec[cli_id].user_id=-1;

        }
    }


    return;
}

void enter_chat_room_(FILE* fp,vector<string> token_vec,int cli_id){
    if(token_vec.size()!= 3){
        fprintf(fp,"%s","Usage: enter-chat-room <port> <version>\n% ");
    }else{
        stringstream ss;
        stringstream s2;

        ss<<token_vec[1];
        s2<<token_vec[2];
        int port_num,version;
        ss>>port_num;
        s2>>version;
        if(ss.fail()){
            ss.clear();
            fprintf(fp,"%s%s%s","Port ",token_vec[1].c_str(), " is not valid.\n% ");

        }else if(port_num<1 || port_num>65535){
            fprintf(fp,"%s%s%s","Port ",token_vec[1].c_str(), " is not valid.\n% ");
        }else{//correct port
            if(s2.fail()){
                s2.clear();
                fprintf(fp,"%s%s%s","Version ",token_vec[2].c_str() ," is not supported.\n% ");
            }else if(version!= 1&&version!=2){
                fprintf(fp,"%s%s%s","Version ",token_vec[2].c_str() ," is not supported.\n% ");
            }else{ //correct port & version
                if(status_vec[cli_id].islogin){
                    const char* t=token_vec[1].c_str(); //for port atoi
                    string history_temp="";
                    for(int i=0;i<chat_history.size();i++){
                        history_temp+=chat_history[i];
                    }
                    fprintf(fp,"%s%s%s%s%s%s%s","Welcome to public chat room.\nPort:",token_vec[1].c_str(),"\nVersion:",token_vec[2].c_str(),"\n",history_temp.c_str(),"% ");
                    struct sockaddr_in temp;
                    bzero(&temp, sizeof(temp));
                    temp.sin_family      = AF_INET;
                    temp.sin_addr.s_addr = htonl(INADDR_ANY);
                    temp.sin_port        = htons(atoi(t));

                    sock_in_room[status_vec[cli_id].user_id]=temp;
                    account_arr[status_vec[cli_id].user_id].version=version;
                }else{
                    fprintf(fp,"%s","Please login first.\n% ");
                }
            }
        }
    }
}



void handle_cli(FILE* fp, vector<string>token_vec,int cli_id,int sockfd){





    if(token_vec[0]=="register"){
        register_(fp,token_vec);
    }
    else if(token_vec[0]=="login"){
        login(fp,token_vec,cli_id);
    }
    else if(token_vec[0]=="logout"){
        logout(fp,token_vec,cli_id);
    }
    else if(token_vec[0]=="exit"){
        exit_(fp,token_vec,cli_id);
    }else if(token_vec[0]=="enter-chat-room"){
        enter_chat_room_(fp,token_vec,cli_id);
    }

    return;
}

void handle_UDP(char buf[],struct sockaddr_in cliaddr ,int udpfd,ssize_t n,socklen_t clilen){

    string name="";
    string msg="";
    Base64 *base = new Base64();

if(buf[0]==0x01){


    //extract name and msg from v1, v2
    if(buf[1]==0x1){
        //version 1

        int name_len=buf[2]*16+buf[3]; // 可以直接來乘，因為16進位只是一種表達方式 ex: int i = 0x1
        for(int i=0;i<name_len;i++){
            name+=buf[4+i];
        }

        int msg_len=buf[4+name_len]*16+buf[5+name_len];

        for(int i=0;i<msg_len;i++){
            msg+=buf[6+name_len+i];
        }

    }else{
        //version 2
        //name
        string b64_name="";
        string b64_msg="";
        int i=2;
        for(;i<n;i++){
            if(buf[i]=='\n'){
                break;
            }else{
                b64_name+=buf[i];
            }
        }
        i++;
        //msg
        for(;i<n;i++){
            if(buf[i]=='\n'){
                break;
            }else{
                b64_msg+=buf[i];
            }
        }
        name = base->Decode(b64_name.c_str(),b64_name.length());
        msg = base->Decode(b64_msg.c_str(),b64_msg.length());

    }
    //do filtering
    int violate=0;
    for(int i=0;i<9;i++){ //9 is the number of filtering list
        vector<int> positions;
        int pos = msg.find(filter_list[i]);
        while(pos!=string::npos){
            positions.push_back(pos);
            pos = msg.find(filter_list[i],pos+1);
        }
        for(int j=0;j<positions.size();j++){ //every occur pos of a given filter word
            for(int k=0;k<filter_list[i].size();k++){ //filter_list[i].size() is the len of that word
                msg[positions[j]+k] ='*';
            }
        }
        violate+=positions.size();
        cout<<i<<" position size= "<<positions.size()<<endl;
    }
    cout<<"final violatr: "<<violate<<endl;
    if(violate){
        for(int i=0;i<account_arr.size();i++){

            if(name==account_arr[i].name){
                account_arr[i].violation++;
                cout<<account_arr[i].name<<" violates "<<account_arr[i].violation<<" times"<<endl;
                if(account_arr[i].violation>=3){
                    //force leaving !!!!need to first find its TCP fp
                    int cli_id;
                    for(int j=0;j<client_max; j++){
                        if(status_vec[j].user_id==i){
                            cli_id=j;
                        }
                    }

                    int sockfd = client[cli_id].fd;


                    string temp="";

                    temp+=("Bye, "+name+".\n% ");

                    int len=temp.length();

                    int err=write(sockfd,temp.c_str(),len);


                    //leave chat room after logout
                    map<int,struct sockaddr_in>::iterator it;
                    cout<<"room size: "<<sock_in_room.size()<<endl;
                    for(it=sock_in_room.begin();it!=sock_in_room.end();it++){

                        if(status_vec[cli_id].user_id==it->first){
                            sock_in_room.erase(it->first);
                                        int res=sock_in_room.erase(it->first);
                            cout<<"erase re: "<<res<<endl;
                            break;

                        }
                    }
                                                            cout<<"11"<<endl;
                    cout<<"before status: "<<status_vec[cli_id].islogin<<"  "<<status_vec[cli_id].user_id<<endl;
                    cout<<"cliid"<<cli_id<<endl;
                    status_vec[cli_id].islogin=0;
                                                            cout<<"12"<<endl;

                    status_vec[cli_id].user_id=-1;
                                                            cout<<"13"<<endl;
                    cout<<"final status: "<<status_vec[cli_id].islogin<<"  "<<status_vec[cli_id].user_id<<endl;
                }
                break;
            }
        }
    }
    string final_msg=""; //for chat history (name:msg\n)
    final_msg+=(name+":"+msg+"\n");

    //chat history
    chat_history.push_back(final_msg);
    for(int i=0;i<chat_history.size();i++){
        cout<<chat_history[i]<<endl;
    }


    //forward the detect message to client(stored as sockaddr) that is in the room(sock_in_room vector)

    //prepare v1, v2 UDP package
    int name_len=name.length();
    int msg_len=msg.length();
    // size of two version is different
    ssize_t total_len_v1=6+name_len+msg_len;
    ssize_t total_len_v2 ;

    char final_buf_v1[4096];
    char final_buf_v2[4096];


    //version 1 UDP packet prepare
    final_buf_v1[0]=0x1;
    final_buf_v1[1]=0x1;

    final_buf_v1[2]= name_len/16;
    final_buf_v1[3]= name_len%16;
    for(int i=0; i<name_len;i++){
        final_buf_v1[4+i]=name[i];
    }

    final_buf_v1[4+name_len]=msg_len/16;
    final_buf_v1[5+name_len]= msg_len%16;

    for(int i=0;i<msg_len;i++){
        final_buf_v1[6+name_len+i]=msg[i];
    }

    //version 2 UDP packet prepare
    string name_64=base->Encode(name.c_str(),name.length());
    string msg_64=base->Encode(msg.c_str(),msg.length());

    // !!!!!!!!!!!!!!!!! note that name_64.length() != name_len (since it's converted to base64)
    total_len_v2=4+name_64.length()+msg_64.length();

    final_buf_v2[0]=0x1;
    final_buf_v2[1]=0x2;
    for(int i = 0;i<name_64.length();i++){
        final_buf_v2[2+i]=name_64[i];
    }
    final_buf_v2[2+name_64.length()]='\n';

    for(int j=0;j<msg_64.length(); j++){
        final_buf_v2[3+name_64.length()+j]=msg_64[j];
    }
    final_buf_v2[3+name_64.length()+msg_64.length()]='\n';



    //forward to everyone based on their version

    map<int,sockaddr_in>::iterator it;
    for(it=sock_in_room.begin();it!=sock_in_room.end();it++){
        if(account_arr[it->first].version==1){
            sendto(udpfd, final_buf_v1, total_len_v1, 0, (struct sockaddr *) &it->second, clilen);
        }else{
            sendto(udpfd, final_buf_v2, total_len_v2, 0, (struct sockaddr *) &it->second, clilen);
        }
    }
     //who's in room
    cout<<"member in the chat room: "<<endl;
    for(it=sock_in_room.begin();it!=sock_in_room.end();it++){
        cout<<"name: "<<account_arr[it->first].name<<" port: "<<ntohs(it->second.sin_port)<<endl;
    }
  }

}

int main(int argc, char **argv)
{
    //sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL);
    //sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL);
    signal(SIGPIPE, SIG_IGN);
    if(argc!=2){
        cout<<"server usage: ./hw1 [port number]" <<endl;
        return -1;
    }
	int					i, maxi, listenfd, connfd, sockfd, udpfd; //udpfd
	int					nready;
	ssize_t				n;
	char				buf[buf_size];
	socklen_t			clilen;
	//struct pollfd		client[client_max];  !!!!need to be global
	struct sockaddr_in	cliaddr, servaddr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
  int flg=1,len=sizeof(int);
  setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&flg,len);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(atoi(argv[1]));

	if(bind(listenfd, (struct sockaddr  *) &servaddr, sizeof(servaddr))<0){
        cout<<"bind error"<<endl;
    }
	listen(listenfd, 15);//LISTENQ


    //for udp bind (server's udp listening port is the same as TCP port ,i,e argv[1])
    udpfd = socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(atoi(argv[1]));
	bind(udpfd, (struct sockaddr  *) &servaddr, sizeof(servaddr));

    //set tcp listenfd
	client[0].fd = listenfd;
	client[0].events = POLLRDNORM;
    //set udpfd
    client[1].fd = udpfd;
	client[1].events = POLLRDNORM;

	for (i = 2; i < 100; i++)
		client[i].fd = -1;		/* -1 indicates available entry */
	maxi = 1;					/* max index into client[] array */

    //to record wether user is log in or not
    for(int i=0;i<client_max;i++){
        discript_status temp(0,-1,-1,0);
        status_vec.push_back(temp);
    }
    cout<<"outside loop"<<endl;

	for ( ; ; ) {
		nready = poll(client, maxi+1, 1000000); //INFTIM

        // check 1. new TCP client connection
		if (client[0].revents & POLLRDNORM) {
			clilen = sizeof(cliaddr);
			connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
			//printf("new client: %s\n", Sock_ntop((SA *) &cliaddr, clilen));
            FILE* fp;
            fp=fdopen(connfd,"r+");
            setvbuf(fp,NULL,_IONBF,0);

            cout<<"new TCP client found"<<endl;

            fprintf(fp,"%s","********************************\n");
            fprintf(fp,"%s","** Welcome to the BBS server. **\n");
            fprintf(fp,"%s","********************************\n");
            fprintf(fp,"%s","% ");


			for (i = 2; i < client_max; i++){
			    if (client[i].fd < 0) {
					client[i].fd = connfd;	/* save descriptor */
					break;
				}
            }

			if (i == client_max)
				printf("too many clients");

			client[i].events = POLLRDNORM;
			if (i > maxi)
				maxi = i;				/* max index in client[] array */

			if (--nready <= 0)
				continue;				/* no more readable descriptor, thus can skip rest of the while loop and go do another poll*/
		}
        // check 2.  detect UTP message  (messsage sent via udp)
        if(client[1].revents & POLLRDNORM){
            clilen = sizeof(cliaddr);
            memset(&buf,0,sizeof(buf));
			n = recvfrom(udpfd, buf, buf_size, 0,(struct sockaddr *) &cliaddr, &clilen);

            handle_UDP(buf,cliaddr,udpfd,n,clilen);


        }

        // check all clients for TCP data (messsage sent via TCP)
		for (i = 2; i <= maxi; i++) {
            sockfd = client[i].fd;
			if ( sockfd < 0) //find available slot
				continue;

            FILE* fp;
            fp=fdopen(sockfd,"r+");
            setvbuf(fp,NULL,_IONBF,0);

			if (client[i].revents & (POLLRDNORM | POLLERR)) { //descriptor is ready
                memset(&buf,0,sizeof(buf));
				if ( (n = read(sockfd, buf, buf_size)) < 0) {
					if (errno == ECONNRESET) {
							/*4connection reset by client */
						printf("client[%d] aborted connection\n", i);
                        //reset client status
                        status_vec[i].islogin=0;
                        status_vec[i].user_id=-1;
						close(sockfd);
						client[i].fd = -1;
					}
				} else if (n == 0) {
						/*connection closed by client */
                    //reset client status
                    status_vec[i].islogin=0;
                    status_vec[i].user_id=-1;
					printf("client[%d] closed connection\n", i);
					close(sockfd);
					client[i].fd = -1;
				} else{
                    //do it
                    string b(buf);
                    memset(&buf,0,sizeof(buf));
                    if(b!="\n"){ //enter \n won't crash
                        string buf(b);
                        vector<string> token_vec;
                        istringstream inn(buf);
                        string t;
                        while(inn>>t){
                            token_vec.push_back(t);
                        }

                        handle_cli(fp,token_vec ,i,sockfd);
                        if(token_vec[0]=="exit" && token_vec.size()==1){  // close(sockfd);  if didn't set client[i].fd to -1 => crash!!!!!!!!!!!!!!
                            close(sockfd);
					        client[i].fd = -1;          //segmentation fault
                            printf("client[%d] closed connection\n", i);
                        }
                        //who's in room
                        map<int,struct sockaddr_in>::iterator it;
                        cout<<"member in the chat room: "<<endl;
                        for(it=sock_in_room.begin();it!=sock_in_room.end();it++){
                            cout<<"name: "<<account_arr[it->first].name<<" port: "<<ntohs(it->second.sin_port)<<endl;
                        }
                    }


                }


				if (--nready <= 0)
					break;				/* no more readable descriptors, go to next poll loop*/
			}
		}
	}
}
