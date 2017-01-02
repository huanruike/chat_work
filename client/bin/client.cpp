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
  与服务器连接，
  注册登录，
  指定接收者，
  线程1读文字信息(接收文件)，线程2写文字信息(传送文件)

  协议格式：
	文字信息：信息类型 接收者 发送者：信息
	文件信息：信息类型 接收者 文件二进制流
	set信息 : 信息类型 接收者 发送者
	bye信息 ：信息类型 发送者
		(信息类型: msg , send，set, bye)
*/

char user[512];//发送者(自己)
char to_user[512];//接受者,"no_user"表示没有指定接受者
pthread_t thr,thw;// 读，写
int BYE;// bye信号
char filename[512];int fileid;

void init(){
	FILE *fp=fopen("../etc/set.txt","r");
	fscanf(fp,"%s%d",ip,&port);
	fclose(fp);
}

int link(){
	int sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock==-1){
		printf("创建socket失败\n");
		return -1;
	}
	sockaddr_in addr;
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=inet_addr(ip);
	addr.sin_port=port;
	if(connect(sock,(sockaddr*)&addr,sizeof(addr))==-1){
		printf("连接失败\n");
		return -1;
	}
	return sock;
}

/**
	set to_user : 切换接受者
	send filepath : 传送文件
	bye : 退出
*/

void *thrf(void *arg){
	char type[512],buf[1000100];
	char path[512];
	while(1){
		read(sock,type,sizeof(type));
		if(strcmp(type,"msg")==0){
			read(sock,buf,sizeof(buf));
			printf("%s\n",buf);
		}
		else if(strcmp(type,"send")==0){
			read(sock,buf,sizeof(buf));
			strcpy(path,"../file/");
			strcat(path,filename);
			fileid++;
			int len=strlen(path);
			int t=fileid;
			char num[128];int n=0;
			while(t) num[n++]=t%10,t/=10;
			for(int i=n-1;i>=0;i--) path[len++]=num[i]+'0';
			path[len]='\0';
			FILE *fp=fopen(path,"w");
			fwrite(buf,sizeof(char),sizeof(buf),fp);
			fclose(fp);
		}
		//fflush(stdout);
		if(BYE) break;
	}
	puts("bye thr");
}

void *thwf(void *arg){
	char x[512],y[512];
	char type[128];
	char buf[1000100];
	while(1){
		scanf("%s",x);
		if(strcmp(x,"set")==0){//set
			scanf("%s",y);
			strcpy(to_user,y);
			strcpy(type,"set");
			write(sock,type,sizeof(type));
			write(sock,to_user,sizeof(to_user));
		}
		else if(strcmp(x,"send")==0){//send
			// 传文件
			scanf("%s",y);
			if(strcmp(to_user,"no_user")==0){
				printf("未指定接收者\n");
				continue;
			}
			FILE *fp=fopen(y,"r");
			if(fp==NULL){
				puts("文件不存在");
				strcpy(buf,"null");
				write(sock,buf,sizeof(buf));
				continue;
			}
			int n=fread(buf,sizeof(char),sizeof(buf),fp);
			fclose(fp);
			printf("n=%d\n",n);
			strcpy(type,"send");
			write(sock,type,sizeof(type));
			write(sock,to_user,sizeof(to_user));
			write(sock,buf,sizeof(buf));
		}
		else if(strcmp(x,"bye")==0){//bye
			BYE=1;
			strcpy(type,"bye");
			write(sock,type,sizeof(type));
			write(sock,user,sizeof(user));
			break;
		}
		else{
			if(strcmp(to_user,"no_user")==0){
				printf("未指定接收者\n");
				continue;
			}
			strcpy(type,"msg");
			char tmp[512];
			strcpy(tmp,user);
			strcat(tmp,": ");
			strcat(tmp,x);
			write(sock,type,sizeof(type));
			write(sock,to_user,sizeof(to_user));
			write(sock,tmp,sizeof(tmp));
		}
		//fflush(stdin);
	}
	puts("bye thw");
}

void com(){
	BYE=0;
	pthread_create(&thr,NULL,thrf,NULL);
	pthread_create(&thw,NULL,thwf,NULL);
	pthread_join(thr,NULL);
	pthread_join(thw,NULL);
}
	
void sign_regist(){
	char name[128],psd[128],op[5],msg[128];
	while(1){
		printf("请选择：1，登录  2，注册 3,退出\n");
		scanf("%s",op);
		write(sock,op,sizeof(op));
		if(op[0]=='1'){
			printf("请输入用户名和密码：\n");
			scanf("%s%s",name,psd);
			write(sock,name,sizeof(name));
			write(sock,psd,sizeof(psd));
			read(sock,msg,sizeof(msg));
			if(strcmp(msg,"Yes")==0){
				printf("登录成功\n");
				strcpy(user,name);
				strcpy(to_user,"no_user");
				strcpy(filename,"file");
				fileid=0;
				com();
			}
			else printf("登录失败\n");
		}
		else if(op[0]=='2'){
			printf("请输入用户名和密码：\n");
			scanf("%s%s",name,psd);
			write(sock,name,sizeof(name));
			write(sock,psd,sizeof(psd));
			read(sock,msg,sizeof(msg));
			if(strcmp(msg,"Yes")==0) printf("注册成功\n");
			else printf("注册失败\n");
		}
		else break;
	}
}

int main(){
	init();// get port and ip
	sock=link();
	if(sock==-1) return 0;
	sign_regist();
	close(sock);
	return 0;
}



