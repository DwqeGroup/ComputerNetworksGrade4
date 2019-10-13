/*
 * proxy.c - CS:APP Web proxy
 */

#include "csapp.h"

/* pass args to doit func */
typedef struct clientinfo{
    int socketfd;
    struct sockaddr_in clientaddr;
    unsigned long tid;
}clientinfo;

/* bool */
typedef unsigned char bool;
#define True 1
#define False 0

/* error type Rio_writen_w */
#define NO_ERROR        0 
#define CLIENT_CLOSED   1
#define UNKOWN_ERROR    2

/*
 * Function prototypes
 */
int parse_uri(char *uri, char *target_addr, char *path, int *port);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);
void doit(clientinfo *client);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
void *thread_handler(void *arg);
int open_clientfd_ts(char *hostname, int port);
int Open_clientfd_ts(char *hostname, int port);
int Rio_writen_w(int fd, void *usrbuf, size_t n);
ssize_t Rio_readnb_w(rio_t *rp, void *usrbuf, size_t n,bool *serverstat);
ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen,bool *serverstat);

/* File pointer to log file */
FILE *logfile;

/* Mutex semaphores */
sem_t *mutex_host;
sem_t *mutex_file;

/* thread id */
unsigned long tid = 0;

/*
 * main - Main routine for the proxy program
 */
int main(int argc, char **argv){
    int lisenfd, port;
    unsigned int clientlen;
    clientinfo* client;
    pthread_t thread;
    
    if (argc != 2){
        fprintf(stderr, "usage:%s <port>\n", argv[0]);
        exit(1);
    }
    port = atoi(argv[1]);
    
    /* Ignore SIGPIPE */
    Signal(SIGPIPE, SIG_IGN);

    sem_unlink("mutex_host");
    sem_unlink("mutex_file");
    if((mutex_host = sem_open("mutex_host",O_CREAT,
         S_IRUSR | S_IWUSR, 1))==NULL){
        fprintf(stderr,"cannot create mutex");
    }
    if((mutex_file = sem_open("mutex_file",O_CREAT,
        S_IRUSR | S_IWUSR, 1))==NULL){
        fprintf(stderr,"cannot create mutex");
    }
    
    lisenfd = Open_listenfd(port);
    clientlen = sizeof(struct sockaddr_in);
    
    while (1){
        /* Create a new memory area to pass arguments to doit */
        /* It will be free by doit */
        client = (clientinfo*)Malloc(sizeof(clientinfo));
        client->socketfd = Accept(lisenfd, (SA *)&client->clientaddr, &clientlen);
//        printf("Client %s connected tid = %zd\n",inet_ntoa(client->clientaddr.sin_addr),tid);
        client->tid = tid ++;
        Pthread_create(&thread, NULL, thread_handler, client);
    }
    return 0;
}
void *thread_handler(void *arg){
    clientinfo *client = (clientinfo*)arg;
    /* Detach this thread */
    Pthread_detach(pthread_self());
    doit(client);
    printf("Thread %zd terminated\n",client->tid);
    pthread_exit(0);
    return NULL;
}

/*
 *  doit
 */
void doit(clientinfo *client){
    
    int serverfd;
    char buf[MAXLINE],method[MAXLINE],uri[MAXLINE],version[MAXLINE];
    char hostname[MAXLINE],pathname[MAXLINE];
    int port;
    char logstring[MAXLINE];
    rio_t rio;
    ssize_t len = 0;
    unsigned int recv_len = 0; /* Received total length */
    bool chunked = True;
    bool clienton = True;
    bool serveron = True;
    unsigned int content_length = 0;
    unsigned int read_size;

    
    /* init args */
    Rio_readinitb(&rio,client->socketfd);
    if (Rio_readlineb_w(&rio, buf, MAXLINE,&clienton) == 0) {
        Close(client->socketfd);
        return;
    }
    
    sscanf(buf,"%s %s %s",method,uri,version);
    if(parse_uri(uri,hostname,pathname,&port)!=0){
        fprintf(stderr, "parse error\n");
        clienterror(client->socketfd, method, "400","uri error","URI error");
        Close(client->socketfd);
        return;
    }
    
    printf("Thread %zd %s\n",client->tid,uri);
    /* connect to server */
    
    if((serverfd=Open_clientfd_ts(hostname,port))<0){
        printf("Cannot connect to server %s %d\n",hostname,port);
        clienterror(client->socketfd, method, "302","Server not found", "Server not found");
        Close(client->socketfd);
        return;
    }
    
    /* generate and push the request to server */

//    if(pathname[0]=='\0') strcpy(pathname,"/");
//    // if(strcmp("HTTP/1.0",version)!=0) printf("Only support HTTP/1.0");
//    sprintf(buf,"%s %s HTTP/1.0\r\n",method, pathname);
//    // printf("%s",buf);
//    Rio_writen_w(serverfd,buf,strlen(buf));
//    sprintf(buf,"Host: %s\r\n",hostname);
//    // printf("%s",buf);
//    Rio_writen_w(serverfd,buf,strlen(buf));
//    sprintf(buf,"\r\n");
//    // printf("%s",buf);
//    Rio_writen_w(serverfd,buf,strlen(buf));
    
    /* Or just copy the HTTP request from client */
    
    if(pathname[0]=='\0') strcpy(pathname,"/");
    sprintf(buf,"%s %s %s\r\n",method, pathname,version);
    Rio_writen_w(serverfd, buf, strlen(buf));
#ifdef MYDEBUG
    printf("%s",buf);
#endif
    while ((len = Rio_readlineb_w(&rio, buf, MAXLINE,&clienton)) != 0) {
        Rio_writen_w(serverfd, buf,len);
#ifdef MYDEBUG
        printf("%s",buf);
#endif
        if (!strcmp(buf, "\r\n")) /* End of request */
            break;
        memset(buf,0,MAXLINE);
    }
    
    /* 
     * receive the response from server 
     */

    len = 0;
    /* Receive response from target server and forward to client */
    Rio_readinitb(&rio, serverfd);
    /* Read head */
    while ((len = Rio_readlineb_w(&rio, buf, MAXLINE,&serveron)) != 0 && clienton && serveron) {
        /* Fix bug of return value when response line exceeds MAXLINE */
        if (len == MAXLINE && buf[MAXLINE - 2] != '\n') --len;
        /* when found "\r\n" means head ends */
        if (!strcmp(buf, "\r\n")){
            if(Rio_writen_w(client->socketfd, buf, len)==CLIENT_CLOSED){clienton = False; break;}
            break;
        }
        if (!strncasecmp(buf, "Content-Length:", 15)) {
            sscanf(buf + 15, "%u", &content_length);
            chunked = False;
        }
        if (!strncasecmp(buf, "Transfer-Encoding:", sizeof("Transfer-Encoding:"))) {
            if(strstr(buf,"chunked")!=NULL || strstr(buf,"Chunked")!=NULL)
                chunked = True;
        }

        /* Send the response line to client and count the total len */
        if(Rio_writen_w(client->socketfd, buf, len)==CLIENT_CLOSED){clienton = False; break;}
        recv_len += len;
    }

    /* Read body */
    if(chunked && clienton && serveron){
        /* Transfer-Encoding:chuncked */
        while ((len = Rio_readlineb_w(&rio, buf, MAXLINE,&serveron)) != 0) {
            /* Fix bug of return value when response line exceeds MAXLINE */
            if (len == MAXLINE && buf[MAXLINE - 2] != '\n') --len;
            /* Send the response line to client and count the total len */
            if(Rio_writen_w(client->socketfd, buf, len)==CLIENT_CLOSED){clienton = False; break;}
            recv_len += len;
            /* End of response */
            if (!strcmp(buf, "0\r\n")) {
                if(Rio_writen_w(client->socketfd, buf, len)==CLIENT_CLOSED){clienton = False; break;}
                recv_len += 2;
                break;
            }
        }
    }
    else if(clienton && serveron){
        read_size = MAXLINE > content_length?content_length:MAXLINE;
        while((len = Rio_readnb_w(&rio,buf,read_size,&serveron))!=0){
            content_length -= len;
            recv_len += len;
            if(Rio_writen_w(client->socketfd, buf, len)==CLIENT_CLOSED){clienton = False; break;}
            if(content_length == 0) break;
            read_size = MAXLINE > content_length?content_length:MAXLINE;
        }
    }


    // printf("Thread %zd finished\n",client->tid);
    format_log_entry(logstring, &client->clientaddr, uri, recv_len);
    P(mutex_file);
    logfile = fopen("proxy.log","a");
    fprintf(logfile, "%s\n", logstring);
    fclose(logfile);
    V(mutex_file);
    
    /* close all socket */
    close(client->socketfd);
    close(serverfd);
    /* free the clientinfo space */
    free(client);
}

/*
 * parse_uri - URI parser
 *
 * Given a URI from an HTTP proxy GET request (i.e., a URL), extract
 * the host name, path name, and port.  The memory for hostname and
 * pathname must already be allocated and should be at least MAXLINE
 * bytes. Return -1 if there are any problems.
 */
int parse_uri(char *uri, char *hostname, char *pathname, int *port){
    char *hostbegin;
    char *hostend;
    char *pathbegin;
    int len;
    
    if (strncasecmp(uri, "http://", 7) != 0){
        hostname[0] = '\0';
        return -1;
    }
    
    /* Extract the host name */
    hostbegin = uri + 7;
    hostend = strpbrk(hostbegin, " :/\r\n\0");
    /* when no ':' show up in the end,hostend may be NULL */
    if(hostend == NULL) hostend = hostbegin + strlen(hostbegin);
    len = (int)(hostend - hostbegin);
    strncpy(hostname, hostbegin, len);
    hostname[len] = '\0';

    /* Extract the port number */
    *port = 80; /* default */
    if (*hostend == ':')
        *port = atoi(hostend + 1);
    
    /* Extract the path */
    pathbegin = strchr(hostbegin, '/');
    if (pathbegin == NULL){
        pathname[0] = '\0';
    }
    else{
        strcpy(pathname, pathbegin);
    }
    
    return 0;
}

/*
 * format_log_entry - Create a formatted log entry in logstring.
 *
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size){
    time_t now;
    char time_str[MAXLINE];
    unsigned long host;
    unsigned char a, b, c, d;
    
    /* Get a formatted time string */
    now = time(NULL);
    strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));
    
    /*
     * Convert the IP address in network byte order to dotted decimal
     * form. Note that we could have used inet_ntoa, but chose not to
     * because inet_ntoa is a Class 3 thread unsafe function that
     * returns a pointer to a static variable (Ch 13, CS:APP).
     */
    host = ntohl(sockaddr->sin_addr.s_addr);
    a = host >> 24;
    b = (host >> 16) & 0xff;
    c = (host >> 8) & 0xff;
    d = host & 0xff;
    
    /* Return the formatted log entry string */
    sprintf(logstring, "%s: %d.%d.%d.%d %s %d", time_str, a, b, c, d, uri,size);
}


void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg){
    char buf[MAXLINE], body[MAXBUF];
    /* Build the HTTP response body */
    sprintf(body, "%s: %s\r\n", errnum, shortmsg);
    sprintf(body, "%s%s: %s", body, longmsg, cause);
    
    /* Print the HTTP response */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen_w(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    Rio_writen_w(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    Rio_writen_w(fd, buf, strlen(buf));
    Rio_writen_w(fd, body, strlen(body));
}

/*
 * A thread-safe version of open_clientfd
 */
int open_clientfd_ts(char *hostname, int port){
    int clientfd;
    struct hostent *hp;
    struct sockaddr_in serveraddr;

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1; /* check errno for cause of error */

    /* Fill in the server's IP address and port */
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    P(mutex_host);
    if ((hp = gethostbyname(hostname)) == NULL)
        return -2; /* check h_errno for cause of error */
    bcopy((char *)hp->h_addr_list[0], (char *)&serveraddr.sin_addr.s_addr, hp->h_length);
    V(mutex_host);
    serveraddr.sin_port = htons(port);

    /* Establish a connection with the server */
    if (connect(clientfd, (SA *)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;
    return clientfd;
}

/*
 * A wrapper function for open_clientfd_ts().
 * If operation fails, it prints an error message and terminates the program.
 */
int Open_clientfd_ts(char *hostname, int port){
    int rc;
    if ((rc = open_clientfd_ts(hostname, port)) < 0) {
        if (rc == -1)
            unix_error("Open_clientfd_ts Unix error");
        else
            dns_error("Open_clientfd_ts DNS error");
    }
    return rc;
}

int Rio_writen_w(int fd, void *usrbuf, size_t n){
    if (rio_writen(fd, usrbuf, n) != n){
        printf("Rio_writen_w error\n");
        if(errno == EPIPE)
            /* client have closed this connection */
            return CLIENT_CLOSED;
        return UNKOWN_ERROR;
    }
    return NO_ERROR;
}

ssize_t Rio_readnb_w(rio_t *rp, void *usrbuf, size_t n,bool *serverstat){
    ssize_t rc;
    if ((rc = rio_readnb(rp, usrbuf, n)) < 0) {
        printf("Rio_readnb_w error\n");
        rc = 0;
        if(errno == ECONNRESET) *serverstat = False;
    }
    return rc;
}

ssize_t Rio_readlineb_w(rio_t *rp, void *usrbuf, size_t maxlen,bool *serverstat){
    ssize_t rc;
    if ((rc = rio_readlineb(rp, usrbuf, maxlen)) < 0) {
        printf("Rio_readlineb_w failed\n");
        rc = 0;
        if(errno == ECONNRESET) *serverstat = False;
    }
    return rc;
}