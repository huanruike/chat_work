#include<bits/stdc++.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

int port;
char ip[512];
int sock;

/**
  接收来自客户端的连接
	每个客户端开个线程去处理
	每个线程：
		登录注册
		接收信息，找到接受者的sock，转发信息(文字/文件)
				(如果接受者不在线，sock为-1)
			set:设置to_user为no_user，bye:关闭sock并设置sock为-1
	
*/	
struct User{
	char name[128],psd[128];
	char to_user[128];
	int sock;
};User user[1000100];int usern;

void init(){
	FILE *fp=fopen("../etc/set.txt","r");
	fscanf(fp,"%s%d",ip,&port);
	fclose(fp);
}

int get_sock(){
	int sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock==-1){
		printf("创建socket失败\n");
		return -1;
	}
	sockaddr_in addr;
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=inet_addr(ip);
	addr.sin_port=port;
	int r=bind(sock,(sockaddr*)&addr,sizeof(addr));
	if(r==-1){
		puts("绑定端口失败");
		return -1;
	}
	listen(sock,30);
	return sock;
}

bool user_regist(char *name,char *psd){
	for(int i=1;i<=usern;i++){
		if(strcmp(user[i].name,name)==0) return 0;
	}
	++usern;
	strcpy(user[usern].name,name);
	strcpy(user[usern].psd,psd);
	strcpy(user[usern].to_user,"no_user");
	user[usern].sock=-1;
	return 1;
}

bool user_sign(char *name,char *psd,int sock){
	for(int i=1;i<=usern;i++){
		if(strcmp(user[i].name,name)==0&&strcmp(user[i].psd,psd)==0){
			user[i].sock=sock;
			return 1;
		}
	}
	return 0;
}
			
void user_set(char *name,char *touser){
	for(int i=1;i<=usern;i++){
		if(strcmp(user[i].name,name)==0){
			strcpy(user[i].to_user,touser);
			return;
		}
	}
}

int user_sock(char *name){
	for(int i=1;i<=usern;i++)
		if(strcmp(user[i].name,name)==0)
			return user[i].sock;
}

void user_bye(char *name){
	for(int i=1;i<=usern;i++){
		if(strcmp(user[i].name,name)==0){
			user[i].sock=-1;
			strcpy(user[i].to_user,"no_user");
		}
	}
}

/**
  服务端不需要主动输出，只需要给客户端响应即可
  */
void com(int sock_ce){
	char type[128],mode[128],buf[1000100];
	while(1){
		read(sock_ce,type,sizeof(type));
		read(sock_ce,mode,sizeof(mode));
		if(strcmp(type,"set")==0){
			read(sock_ce,buf,sizeof(buf));
			user_set(buf,mode);
		}
		else if(strcmp(type,"bye")==0){
			user_bye(mode);
			break;
		}
		else if(strcmp(type,"msg")==0){
			read(sock_ce,buf,sizeof(buf));
			int sock=user_sock(mode);
			if(~sock){
				write(sock,type,sizeof(type));
				write(sock,buf,sizeof(buf));
			}
		}
		else{
			read(sock_ce,buf,sizeof(buf));
//			if(strcmp(buf,"null")==0) continue;
			int sock=user_sock(mode);
			if(~sock){
				write(sock,type,sizeof(type));
				write(sock,buf,sizeof(buf));
			}
		}
	}
}

void *thf(void *arg){// 先登录注册
	int sock_ce=(*((int*)arg));
	char name[128],psd[128],op[5],msg[128];
	while(1){
		read(sock_ce,op,sizeof(op));
		if(op[0]=='1'){
			read(sock_ce,name,sizeof(name));
			read(sock_ce,psd,sizeof(psd));
			if(user_sign(name,psd,sock_ce)){
				strcpy(msg,"Yes");
				write(sock_ce,msg,sizeof(msg));
				com(sock_ce);
			}
			else strcpy(msg,"No"),write(sock_ce,msg,sizeof(msg));
		}
		else if(op[0]=='2'){
			read(sock_ce,name,sizeof(name));
			read(sock_ce,psd,sizeof(psd));
			if(user_regist(name,psd)) strcpy(msg,"Yes");
			else strcpy(msg,"No");
			write(sock_ce,msg,sizeof(msg));
		}
		else break;
	}
	close(sock_ce);
}
				

void Listen(){
	usern=0;
	while(1){
		socklen_t len_ce;
		sockaddr_in addr_ce;
		int sock_ce=accept(sock,(sockaddr*)&addr_ce,&len_ce);
		if(sock_ce==-1){
			printf("服务器socket被设成非阻塞，有误，应该为阻塞\n");
			break;
		}
		pthread_t th;
		pthread_create(&th,NULL,thf,(void*)&sock_ce);
	}
}

int main(){
	init();
	sock=get_sock();
	if(sock==-1) return 0;
	Listen();
	close(sock);
	return 0;
}

