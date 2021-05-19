#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <fstream>
#include <unistd.h>
#include <pthread.h>
using namespace std;

int create_socket(int);
int accept_conn(int);

#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#define MAXLINE 4096 /*max text line length*/
#define LISTENQ 8	 /*maximum number of client connections*/
int listenfd, connfd, n;
pid_t childpid;
socklen_t clilen;
char buf[MAXLINE];
struct sockaddr_in cliaddr, servaddr;
int data_port = 1024;

int numberClients = 8;
void *multipleThread(void *var)
{
	using namespace std;

	cout << "new thread was create for client:  " << endl;
	int conf = *((int *)var);

	data_port += 10000;
	while ((n = recv(conf, buf, MAXLINE, 0)) > 0)
	{
		cout << "String received from client: " << buf;
		char *token, *dummy;
		dummy = buf;
		token = strtok(dummy, " ");

		if (strcmp("quit\n", buf) == 0)
		{
			cout << "The client has quit\n";
		}

		if (strcmp("ls\n", buf) == 0)
		{
			FILE *in;
			char temp[MAXLINE], port[MAXLINE];
			int datasock;
			data_port = data_port + 1;
			if (data_port == 8080)
			{
				data_port = data_port + 1;
			}
			sprintf(port, "%d", data_port);
			datasock = create_socket(data_port); //creating socket for data connection
			cout << datasock;
			send(conf, port, MAXLINE, 0); //sending data connection port no. to client
			datasock = accept_conn(datasock);
			cout << datasock; //accepting connection from client
			if (!(in = popen("ls", "r")))
			{
				cout << "error" << endl;
			}
			while (fgets(temp, sizeof(temp), in) != NULL)
			{
				send(datasock, "1", MAXLINE, 0);
				send(datasock, temp, MAXLINE, 0);
			}
			send(datasock, "0", MAXLINE, 0);
			pclose(in);
			//cout<<"file closed\n";
		}

		if (strcmp("pwd\n", buf) == 0)
		{
			char curr_dir[MAXLINE];

			GetCurrentDir(curr_dir, MAXLINE - 1);
			send(conf, curr_dir, MAXLINE, 0);
			//cout<<curr_dir<<endl;
		}

		if (strcmp("cd", token) == 0)
		{
			token = strtok(NULL, " \n");
			cout << "Path given is: " << token << endl;
			if (chdir(token) < 0)
			{
				send(conf, "0", MAXLINE, 0);
			}
			else
			{
				send(conf, "1", MAXLINE, 0);
			}
		}

		if (strcmp("put", token) == 0)
		{
			char port[MAXLINE], buffer[MAXLINE], char_num_blks[MAXLINE], char_num_last_blk[MAXLINE], check[MAXLINE];
			int datasock, num_blks, num_last_blk, i;
			FILE *fp;
			token = strtok(NULL, " \n");
			cout << "Filename given is: " << token << endl;
			data_port = data_port + 1;
			if (data_port == 8080)
			{
				data_port = data_port + 1;
			}
			sprintf(port, "%d", data_port);
			datasock = create_socket(data_port); //creating socket for data connection
			send(conf, port, MAXLINE, 0);		 //sending data connection port to client
			datasock = accept_conn(datasock);	 //accepting connection
			recv(conf, check, MAXLINE, 0);
			cout << check;
			if (strcmp("1", check) == 0)
			{
				if ((fp = fopen(token, "w")) == NULL)
					cout << "Error in creating file\n";
				else
				{
					recv(conf, char_num_blks, MAXLINE, 0);
					num_blks = atoi(char_num_blks);
					for (i = 0; i < num_blks; i++)
					{
						recv(datasock, buffer, MAXLINE, 0);
						fwrite(buffer, sizeof(char), MAXLINE, fp);
						//cout<<buffer<<endl;
					}
					recv(conf, char_num_last_blk, MAXLINE, 0);
					num_last_blk = atoi(char_num_last_blk);
					if (num_last_blk > 0)
					{
						recv(datasock, buffer, MAXLINE, 0);
						fwrite(buffer, sizeof(char), num_last_blk, fp);
						//cout<<buffer<<endl;
					}
					fclose(fp);
					cout << "File download done.\n";
				}
			}
		}

		if (strcmp("get", token) == 0)
		{
			char port[MAXLINE], buffer[MAXLINE], char_num_blks[MAXLINE], char_num_last_blk[MAXLINE];
			int datasock, lSize, num_blks, num_last_blk, i;
			FILE *fp;
			token = strtok(NULL, " \n");
			cout << "Filename given is: " << token << endl;
			data_port = data_port + 1;
			if (data_port == 8080)
			{
				data_port = data_port + 1;
			}
			sprintf(port, "%d", data_port);
			datasock = create_socket(data_port); //creating socket for data connection
			send(conf, port, MAXLINE, 0);		 //sending port no. to client
			datasock = accept_conn(datasock);	 //accepting connnection by client
			if ((fp = fopen(token, "r")) != NULL)
			{
				//size of file
				send(conf, "1", MAXLINE, 0);
				fseek(fp, 0, SEEK_END);
				lSize = ftell(fp);
				rewind(fp);
				num_blks = lSize / MAXLINE;
				num_last_blk = lSize % MAXLINE;
				sprintf(char_num_blks, "%d", num_blks);
				send(conf, char_num_blks, MAXLINE, 0);
				//cout<<num_blks<<"	"<<num_last_blk<<endl;

				for (i = 0; i < num_blks; i++)
				{
					fread(buffer, sizeof(char), MAXLINE, fp);
					send(datasock, buffer, MAXLINE, 0);
					//cout<<buffer<<"	"<<i<<endl;
				}
				sprintf(char_num_last_blk, "%d", num_last_blk);
				send(conf, char_num_last_blk, MAXLINE, 0);
				if (num_last_blk > 0)
				{
					fread(buffer, sizeof(char), num_last_blk, fp);
					send(datasock, buffer, MAXLINE, 0);
					//cout<<buffer<<endl;
				}
				fclose(fp);
				cout << "File upload done.\n";
			}
			else
			{
				cout << "soory";
				send(conf, "0", MAXLINE, 0);
			}

			//close(conf);
			//pthread_exit(0);
		}
	}
}

int main(int argc, char **argv)
{

	int i = 0;
	if (argc != 2)
	{ //validating the input
		cerr << "Usage: ./a.out <port number>" << endl;
		exit(1);
	}

	//Create a socket for the soclet
	//If sockfd<0 there was an error in the creation of the socket
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cerr << "Problem in creating the socket" << endl;
		exit(2);
	}

	//preparation of the socket address
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (atoi(argv[1]) <= 1024)
	{
		cerr << "Port number must be greater than 1024" << endl;
		exit(2);
	}
	servaddr.sin_port = htons(atoi(argv[1]));

	//bind the socket
	bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	//listen to the socket by creating a connection queue, then wait for clients
	listen(listenfd, LISTENQ);

	cout << "Server running...waiting for connections." << endl;
	int client_sock;
	int ir = 0;
	pthread_t pid;
	pthread_attr_t attr;
	int data_port_n = 1024;
	pthread_attr_init(&attr);
	while (1)
	{
		cout << "helloo" << endl;

		connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
		if (connfd == -1)
		{
			continue;
		}

		if (pthread_create(&pid, &attr, multipleThread, &connfd) != 0)
		{
			perror("pthread create in main");
			//close(connfd);
		}
	}

	// for (;;)
	// {

	// 	clilen = sizeof(cliaddr);
	// 	//accept a connection
	// 	connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);

	// 	cout << "Received request..." << endl;

	// 	if ((childpid = fork()) == 0)
	// 	{ //if it’s 0, it’s child process

	// 		cout << "Child created for dealing with client requests" << endl;

	// 		//close listening socket
	// 		//close(listenfd);
	// 		//int data_port = 1024; //for data connection
	// 		if (i == 0)
	// 		{
	// 			i++;
	// 			cout << "thread1";
	// 			thread t1(multipleThread, 1024);
	// 			t1.join();
	// 		}
	// 		else
	// 		{
	// 			i++;
	// 			cout << "thread2";
	// 			thread t2(multipleThread, 2024);
	// 			t2.join();
	// 		}

	// 		if (n < 0)
	// 			cout
	// 				<< "Read error" << endl;

	// 		exit(0);
	// 	}
	//close socket of the server
	//close(connfd);
	//}
}

int create_socket(int port)
{
	int listenfd;
	struct sockaddr_in dataservaddr;

	//Create a socket for the soclet
	//If sockfd<0 there was an error in the creation of the socket
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cerr << "Problem in creating the data socket3" << endl;
		exit(2);
	}

	//preparation of the socket address
	dataservaddr.sin_family = AF_INET;
	dataservaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	dataservaddr.sin_port = htons(port);

	if ((bind(listenfd, (struct sockaddr *)&dataservaddr, sizeof(dataservaddr))) < 0)
	{
		cout << listenfd;
		cerr << "Problem in binding the data socket1" << endl;
		exit(2);
	}

	//listen to the socket by creating a connection queue, then wait for clients
	listen(listenfd, 1);

	return (listenfd);
}

int accept_conn(int sock)
{
	int dataconnfd;
	socklen_t dataclilen;
	struct sockaddr_in datacliaddr;

	dataclilen = sizeof(datacliaddr);
	//accept a connection
	if ((dataconnfd = accept(sock, (struct sockaddr *)&datacliaddr, &dataclilen)) < 0)
	{
		cerr << "Problem in accepting the data socket" << endl;
		exit(2);
	}

	return (dataconnfd);
}
