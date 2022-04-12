
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

using namespace std;

#define buf_size 1000 
#define client_max 100
//#define debug debug

class discript_status{
public:
    bool islogin;
    int user_id;
    discript_status(){}
    discript_status(bool is,int id){
        this->islogin=is;
        this->user_id=id;
    }
};
class account{
public:
        string name;
        string password;
    account(string n,string p){
        this->name=n;
        this->password=p;
    }
    account(){}
};
class comment{
public:
    string comment_content;
    int owner_id;
    comment(){}
    comment(string c,int o){
        this->comment_content=c;
        this->owner_id=o;
    }
};
class post{
public:
    string belong_board,title;
    int post_owner_id,serial_id;
    string content;
    string date;
    //bool delete_;
    vector<comment> comment_vec;
    post(){}
    post(string b,string t,int p,string con,int s,string d){
        this->belong_board=b;
        this->title=t;
        this->post_owner_id=p;
        this->content=con;
        this->serial_id=s;
        this->date=d;
        //this->delete_=0;
    }
};
class board{
public:
    string name;
    int moderator_id;
    vector<post> post_vec;
    board(){}
    board(string n,int m){
        this->name=n;
        this->moderator_id=m;
    }
};
//global var
vector<account>account_arr;
vector<board> board_vec;
int serial_id=1;
vector<discript_status> status_vec;

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
            fprintf(fp,"%s","Register successfully.\n");
            account temp(token_vec[1],token_vec[2]);
            account_arr.push_back(temp);
        }
        //username already token
        else{
            fprintf(fp,"%s","Username is already used.\n");
        }
                     //incorrect input format  
    }
    else{
        fprintf(fp,"%s","Usage: register <username> <password>\n");
    }
    return;
}

void login(FILE* fp,vector<string> token_vec,int cli_id ){
    if(token_vec.size()==3){
                        //not login yet
        if(!status_vec[cli_id].islogin){
            // find match username and correct passord
            
            bool exist=false;
            int i=0;
            
            for(;i<account_arr.size();i++){
                if(account_arr[i].name==token_vec[1]){
                    exist=true;
                    break;    //need to break, otherwise i++
                }
            }
            for(int j=0;j<client_max;j++){
                if(status_vec[j].user_id==i){
                    fprintf(fp,"%s","Please logout first.\n");
                    return;
                }
            }   
            if(exist &&  account_arr[i].password==token_vec[2]){   
                fprintf(fp,"%s%s%s","Welcome, ",account_arr[i].name.c_str(),".\n");
                status_vec[cli_id].islogin=true;
                status_vec[cli_id].user_id=i;
            }
            //login failed
            else{
                fprintf(fp,"%s","Login failed.\n");
            }
        }
        //already login, need to logout first
        else{
            fprintf(fp,"%s","Please logout first.\n");
        }
    }
    //incorrect input format
    else{
        fprintf(fp,"%s", "Usage: login <username> <password>\n");
    }    

    return;
}

void logout(FILE* fp,vector<string> token_vec,int cli_id){
    if(status_vec[cli_id].islogin){
        fprintf(fp,"%s%s%s","Bye, ",account_arr[status_vec[cli_id].user_id].name.c_str(),".\n");
        status_vec[cli_id].islogin=false;
        status_vec[cli_id].user_id=-1;
    }
    else{
        fprintf(fp,"%s","Please login first.\n");
    }
    return;
}

void exit_(FILE* fp,vector<string> token_vec,int cli_id ){
    if(status_vec[cli_id].islogin){
        fprintf(fp,"%s%s%s","Bye, ",account_arr[status_vec[cli_id].user_id].name.c_str(),".\n");
        status_vec[cli_id].islogin=0;
        status_vec[cli_id].user_id=-1;
    }
            cout<<"h3"<<endl;

    return;
}

void createboard(FILE* fp,vector<string> token_vec,int cli_id ){
    if(status_vec[cli_id].islogin){
        if(token_vec.size()!= 2){
            fprintf(fp,"%s","Usage: create-board <name>\n");
        }
        else{
            bool exist=0;
            for(int i=0;i<board_vec.size();i++){
                if(board_vec[i].name==token_vec[1]){
                    exist=1;
                    break;
                }
            }
            if(exist){
                fprintf(fp,"%s","Board already exists.\n");
            }
            else{
                fprintf(fp,"%s","Create board successfully.\n");
                board temp(token_vec[1],status_vec[cli_id].user_id);
                board_vec.push_back(temp);
            }
        }
    }
    else{
        fprintf(fp,"%s","Please login first.\n");
    }
    return;
}

void createpost(FILE* fp,vector<string> token_vec,int cli_id ){
    string post_title="";
    string content="";
    if(token_vec.size()<6 ){ //avoid segmentation fault
        fprintf(fp,"%s","Usage: create-post <board-name> --title <title> --content <content>\n");
        return;
    }
    
    if(token_vec[2]=="--title"){
        int i=3;
        for(;i<token_vec.size();i++){
            if(token_vec[i]=="--content"){
                break;
            }
            else{
                post_title+=token_vec[i];
                post_title+=" ";
            }
        }
        i++; //skip "--content"
        for(;i<token_vec.size();i++){
            content+=token_vec[i];
            content+=" ";
        }
    }
    else if(token_vec[2]=="--content"){    //switching also valid
        int i=3;
        for(;i<token_vec.size();i++){
            if(token_vec[i]=="--title"){
                break;
            }
            else{
                content+=(token_vec[i]+" ");
                content+=" ";
            }
        }
        i++; //skip "--title"
        for(;i<token_vec.size();i++){
            post_title+=(token_vec[i]);
            post_title+=" ";
        }
    }
    if( token_vec[1]=="--title"||token_vec[1]=="--content" || post_title.length()==0 || content.length()==0){
        fprintf(fp,"%s","Usage: create-post <board-name> --title <title> --content <content>\n");
        return;
    }
  
    if(status_vec[cli_id].islogin){
        bool exist=0;
        int i=0;
        for(;i<board_vec.size();i++){
            if(board_vec[i].name==token_vec[1]){
                exist=1;
                break;
            }
        }
        if(exist){  //board exist
            time_t now = time(0);   // get time now
            tm *ltm = localtime(&now);
            string date="";
            date+= to_string(ltm->tm_mon + 1);
            date+="/";
            date+=to_string(ltm->tm_mday);
            cout<<"title: "<<post_title<<endl;
            cout<<"content: "<<content<<endl;
            post temp(token_vec[1],post_title,status_vec[cli_id].user_id,content,serial_id,date);
            serial_id++;
            board_vec[i].post_vec.push_back(temp);
            fprintf(fp,"%s","Create post successfully.\n");
        }
        else{
            fprintf(fp,"%s","Board does not exist.\n");
        }
    }
    else{
        fprintf(fp,"%s","Please login first.\n");
    }
    return;
}

void listboard(FILE* fp,vector<string> token_vec ,int cli_id){
    if(token_vec.size()!=1){
        fprintf(fp,"Usage: list-board\n");
    }else{
        fprintf(fp,"%s","Index    Name    Moderator\n");
        for(int i=0;i<board_vec.size();i++){    
            fprintf(fp,"%d     %s     %s\n",i+1,board_vec[i].name.c_str(),account_arr[board_vec[i].moderator_id].name.c_str());
        }
    }
    
        return;
}

void read_(FILE* fp,vector<string> token_vec ,int cli_id){
    if(token_vec.size()!=2){
        fprintf(fp,"%s", "Usage: read <post-S/N>\n");
    }else{
        bool exist=0;
        for(int i=0;i<board_vec.size();i++){
            for(int j=0;j<board_vec[i].post_vec.size();j++){
                
                if(to_string(board_vec[i].post_vec[j].serial_id)==token_vec[1]){ //find given post
                    exist=1;
                    fprintf(fp,"Author: %s\n",account_arr[board_vec[i].post_vec[j].post_owner_id].name.c_str());
                    fprintf(fp,"Title: %s\n",board_vec[i].post_vec[j].title.c_str());
                    fprintf(fp,"Date: %s\n",board_vec[i].post_vec[j].date.c_str());
                    string con_result="";
                    string con_temp=board_vec[i].post_vec[j].content;
                    cout<<"con temp: "<<con_temp<<endl;
                    for(int k=0;k<con_temp.length();k++){
                        if(con_temp[k]=='<' &&con_temp[k+1]=='b'&&con_temp[k+2]=='r' && con_temp[k+3]=='>'){
                            con_result+="\n";
                            k+=3;
                        }else if(con_temp[k] !=' '){
                            con_result+=con_temp[k];
                            
                        }else if(con_temp[k] ==' '){
                            con_result+=" ";
                        }
                    }
                    fprintf(fp,"--\n%s\n--\n",con_result.c_str()); //content
                    for(int k=0;k<board_vec[i].post_vec[j].comment_vec.size();k++){
                        fprintf(fp,"%s: %s\n"\
                        ,account_arr[board_vec[i].post_vec[j].comment_vec[k].owner_id].name.c_str()\
                        ,board_vec[i].post_vec[j].comment_vec[k].comment_content.c_str());
                    }

                }
            }
        }
        if(!exist){ 
            fprintf(fp,"%s","Post does not exist.\n");
        }
    }
    return;
}

void listpost(FILE* fp,vector<string> token_vec,int cli_id ){
    if(token_vec.size()!=2){
        fprintf(fp,"%s","Usage: list-post <board-name>\n");
    }
    else{
        bool boa_exist=0;
        for(int i=0;i<board_vec.size();i++){
            if(board_vec[i].name==token_vec[1]){ //find the given board
                boa_exist=1;
                fprintf(fp,"%s","S/N     Title     Author     Date\n");
                for(int j=0;j<board_vec[i].post_vec.size();j++){
                     
                    
                    post temp=board_vec[i].post_vec[j];
                    fprintf(fp,"%d     %s    %s   %s\n",\
                            temp.serial_id, temp.title.c_str(), account_arr[temp.post_owner_id].name.c_str(), temp.date.c_str());
                }
            }
        }
        if(!boa_exist){
            fprintf(fp,"%s","Board does not exist.\n");
        }
    }
    return;
}

void updatepost(FILE* fp,vector<string> token_vec ,int cli_id){
    if(token_vec.size() < 4){
        fprintf(fp,"Usage: update-post <post-S/N> --title/content <new>\n");
    }else if(token_vec[2]!= "--title" && token_vec[2]!= "--content"){
        fprintf(fp,"Usage: update-post <post-S/N> --title/content <new>\n");
    }else{//correct format
        if(status_vec[cli_id].islogin){
            
            for(int i=0;i<board_vec.size();i++){
                for(int j=0;j<board_vec[i].post_vec.size();j++){
                    if(to_string(board_vec[i].post_vec[j].serial_id)==token_vec[1] ){  //find given post
                        
                        //is the owner
                        if(board_vec[i].post_vec[j].post_owner_id==status_vec[cli_id].user_id){ 
                            if(token_vec[2]=="--title"){
                                string temp="";
                                for(int i=3;i<token_vec.size();i++){
                                    temp+=(token_vec[i]+" ");
                                }
                                board_vec[i].post_vec[j].title=temp;
                            }else if(token_vec[2]=="--content"){
                                string temp="";
                                for(int i=3;i<token_vec.size();i++){
                                    temp+=(token_vec[i]+" ");
                                }
                                board_vec[i].post_vec[j].content=temp;
                            }
                            fprintf(fp,"Update successfully.\n");
                            return;
                        }
                        else{ //not the owner
                            fprintf(fp,"Not the post owner.\n");
                            return;
                        }
                    }
                }
            }          
            //if doesn't return after checking all post => noe exist
            fprintf(fp,"Post does not exist.\n"); 
        }
        else{
            fprintf(fp,"Please login first.\n");
        }
    }
    return;
}


void deletepost(FILE* fp,vector<string> token_vec ,int cli_id){
    if(token_vec.size()!=2){
        fprintf(fp,"Usage: delete-post <post-S/N>\n");
    }else{
        if(status_vec[cli_id].islogin){
            
            for(int i=0;i<board_vec.size();i++){
                for(int j=0;j<board_vec[i].post_vec.size();j++){
                    if(to_string(board_vec[i].post_vec[j].serial_id)==token_vec[1] ){  //find given post
                        
                        //is the owner
                        if(board_vec[i].post_vec[j].post_owner_id==status_vec[cli_id].user_id){ 
                            board_vec[i].post_vec.erase(board_vec[i].post_vec.begin()+j);
                            fprintf(fp,"Delete successfully.\n");
                            return;
                        }
                        else{ //not the owner
                            fprintf(fp,"Not the post owner.\n");
                            return;
                        }
                    }
                }
            }
            //if doesn't return after checking all post => noe exist
            fprintf(fp,"Post does not exist.\n"); 
        }
        else{
            fprintf(fp,"Please login first.\n");
        }
    }
    return;
}

void comment_(FILE* fp,vector<string> token_vec ,int cli_id){
    if(token_vec.size()<3){
        fprintf(fp,"%s","Usage: comment <post-S/N> <comment>\n");
        return;
    }else{
        //fprintf(fp,"correct for");
        if(status_vec[cli_id].islogin){
            bool exist=0;
            for(int i=0;i<board_vec.size();i++){
                for(int j=0;j<board_vec[i].post_vec.size();j++){
                    post temp=board_vec[i].post_vec[j];
                    
                    if(to_string(temp.serial_id)==token_vec[1]){ //find given post
                        exist=1;
                        string com="";
                        for(int i=2;i<token_vec.size();i++){
                            com+=token_vec[i]+" ";
                        }
                        comment com_temp(com,status_vec[cli_id].user_id);
                        board_vec[i].post_vec[j].comment_vec.push_back(com_temp);
                        fprintf(fp,"Comment successfully.\n");
                        
                    }
                }
            }
            if(!exist){
                //if doesn't return after checking all post => post not exist
                fprintf(fp,"Post does not exist.\n");
            }
            
        }else{
            fprintf(fp,"%s","Please login first.\n");
        }
    }    
    return;
}



void handle_cli(FILE* fp, vector<string>token_vec,int cli_id,int sockfd){
    

#ifdef	debug
    for(int i=0;i<token_vec.size();i++){
        cout<<token_vec[i]<<endl;
    }
#endif

    
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
    }
    else if(token_vec[0]=="create-board"){
        createboard(fp,token_vec,cli_id);
    }
    else if(token_vec[0]=="create-post"){
        createpost(fp,token_vec,cli_id);
    }
    else if(token_vec[0]=="list-board"){
        listboard(fp,token_vec,cli_id);
    }
    else if(token_vec[0]=="list-post"){
        listpost(fp,token_vec,cli_id);
    }
    else if(token_vec[0]=="read"){
        read_(fp,token_vec,cli_id);
    }
    else if(token_vec[0]=="delete-post"){
        deletepost(fp,token_vec,cli_id);
    }
    else if(token_vec[0]=="update-post"){
        updatepost(fp,token_vec,cli_id);
    }
    else if(token_vec[0]=="comment"){
        comment_(fp,token_vec,cli_id);
    }


    return;
}

int main(int argc, char **argv)
{   

    signal(SIGPIPE, SIG_IGN);
    if(argc!=2){
        cout<<"server usage: ./hw1 [port number]" <<endl;
        return -1;
    }
	int					i, maxi, listenfd, connfd, sockfd;
	int					nready;
	ssize_t				n;
	char				buf[buf_size];
	socklen_t			clilen;
	struct pollfd		client[client_max];
	struct sockaddr_in	cliaddr, servaddr;
 
	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(atoi(argv[1]));

	if(bind(listenfd, (struct sockaddr  *) &servaddr, sizeof(servaddr))<0){
        cout<<"bind error"<<endl;
    }
	listen(listenfd, 15);//LISTENQ

    //set listenfd
	client[0].fd = listenfd;
	client[0].events = POLLRDNORM;
	for (i = 1; i < 100; i++)
		client[i].fd = -1;		/* -1 indicates available entry */
	maxi = 0;					/* max index into client[] array */

    for(int i=0;i<client_max;i++){
        discript_status temp(0,-1);
        status_vec.push_back(temp);
    }
	for ( ; ; ) {
		nready = poll(client, maxi+1, 1000000); //INFTIM

		if (client[0].revents & POLLRDNORM) {	/* new client connection */
			clilen = sizeof(cliaddr);
			connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
			//printf("new client: %s\n", Sock_ntop((SA *) &cliaddr, clilen));
            cout<<"new client found"<<endl;
             write(connfd, "********************************\n", 34);
            write(connfd, "** Welcome to the BBS server. **\n", 34);
            write(connfd, "********************************\n", 34);
            write(connfd, "% ",2);

			for (i = 1; i < client_max; i++)
				if (client[i].fd < 0) {
					client[i].fd = connfd;	/* save descriptor */
					break;
				}
			if (i == client_max)
				printf("too many clients");

			client[i].events = POLLRDNORM;
			if (i > maxi)
				maxi = i;				/* max index in client[] array */

			if (--nready <= 0)
				continue;				/* no more readable descriptor, thus can skip rest of the while loop and go do another poll*/
		}

		for (i = 1; i <= maxi; i++) {	/* check all clients for data */
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
                        if(token_vec[0]=="exit"){  // close(sockfd);  if didn't set client[i].fd to -1 => crash!!!!!!!!!!!!!! 
                            close(sockfd);
					        client[i].fd = -1;          //segmentation fault
                            printf("client[%d] closed connection\n", i);
                        }
                    }
                    fprintf(fp,"%s","% ");

                }

                    
				if (--nready <= 0)
					break;				/* no more readable descriptors, go to next poll loop*/
			}
		}
	}
}
