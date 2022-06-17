//
// request.c: Does the bulk of the work for the web server.
//

#include "request.h"

/**
 * @param buf A buffer to prit to
 * @param var Contains the data of time
 * @param stats Contains the stats about the thread
 * @return 0 on sucess. -1 if one of the pointers is NULL. 
 */
int printStatsToBuf(char buf[], ConnVar var, Stats stats)
{
   if(!buf || !var || !stats)
   {
      return -1;
   }
   struct timeval dispatch_interval;
   timersub(&(var->leave_time), &(var->enter_time), &dispatch_interval);
   sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf, var->enter_time.tv_sec, var->enter_time.tv_usec);
   sprintf(buf, "%sStat-Req-Dispatch:: %lu.%06lu\r\n", buf, dispatch_interval.tv_sec, dispatch_interval.tv_usec);
   sprintf(buf, "%sStat-Thread-Id:: %d\r\n", buf, stats->thread_id);
   sprintf(buf, "%sStat-Thread-Count:: %d\r\n", buf, stats->thread_req_count);
   sprintf(buf, "%sStat-Thread-Static:: %d\r\n", buf, stats->thread_static_req_count);
   sprintf(buf, "%sStat-Thread-Dynamic:: %d\r\n\r\n", buf, stats->thread_dynamic_req_count);
   return 0;
}

// requestError(      fd,    stats,   filename,        "404",    "Not found", "OS-HW3 Server could not find this file");
void requestError(ConnVar var, Stats stats, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
   int fd = var->connfd;
   char buf[MAXLINE], body[MAXBUF];

   // Create the body of the error message
   sprintf(body, "<html><title>OS-HW3 Error</title>");
   sprintf(body, "%s<body bgcolor="
                 "fffff"
                 ">\r\n",
           body);
   sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
   sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
   sprintf(body, "%s<hr>OS-HW3 Web Server\r\n", body);

   // Write out the header information for this response
   sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Content-Type: text/html\r\n");
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Content-Length: %lu\r\n", strlen(body));
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   struct timeval dispatch_interval;
   timersub(&(var->leave_time), &(var->enter_time), &dispatch_interval);

   sprintf(buf, "Stat-Req-Arrival:: %lu.%06lu\r\n", var->enter_time.tv_sec, var->enter_time.tv_usec);
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Stat-Req-Dispatch:: %lu.%06lu\r\n", dispatch_interval.tv_sec, dispatch_interval.tv_usec);
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Stat-Thread-Id:: %d\r\n", stats->thread_id);
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Stat-Thread-Count:: %d\r\n", stats->thread_req_count);
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Stat-Thread-Static:: %d\r\n", stats->thread_static_req_count);
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   sprintf(buf, "Stat-Thread-Dynamic:: %d\r\n\r\n", stats->thread_dynamic_req_count);
   Rio_writen(fd, buf, strlen(buf));
   printf("%s", buf);

   // Write out the content
   Rio_writen(fd, body, strlen(body));
   printf("%s", body);
}

//
// Reads and discards everything up to an empty text line
//
void requestReadhdrs(rio_t *rp)
{
   char buf[MAXLINE];

   Rio_readlineb(rp, buf, MAXLINE);
   while (strcmp(buf, "\r\n"))
   {
      Rio_readlineb(rp, buf, MAXLINE);
   }
   return;
}

//
// Return 1 if static, 0 if dynamic content
// Calculates filename (and cgiargs, for dynamic) from uri
//
int requestParseURI(char *uri, char *filename, char *cgiargs)
{
   char *ptr;

   if (strstr(uri, ".."))
   {
      sprintf(filename, "./public/home.html");
      return 1;
   }

   if (!strstr(uri, "cgi"))
   {
      // static
      strcpy(cgiargs, "");
      sprintf(filename, "./public/%s", uri);
      if (uri[strlen(uri) - 1] == '/')
      {
         strcat(filename, "home.html");
      }
      return 1;
   }
   else
   {
      // dynamic
      ptr = index(uri, '?');
      if (ptr)
      {
         strcpy(cgiargs, ptr + 1);
         *ptr = '\0';
      }
      else
      {
         strcpy(cgiargs, "");
      }
      sprintf(filename, "./public/%s", uri);
      return 0;
   }
}

//
// Fills in the filetype given the filename
//
void requestGetFiletype(char *filename, char *filetype)
{
   if (strstr(filename, ".html"))
      strcpy(filetype, "text/html");
   else if (strstr(filename, ".gif"))
      strcpy(filetype, "image/gif");
   else if (strstr(filename, ".jpg"))
      strcpy(filetype, "image/jpeg");
   else
      strcpy(filetype, "text/plain");
}

void requestServeDynamic(ConnVar var, char *filename, char *cgiargs, Stats stats)
{
   int fd = var->connfd;
   char buf[MAXLINE], *emptylist[] = {NULL};

   // The server does only a little bit of the header.
   // The CGI script has to finish writing out the header.
   sprintf(buf, "HTTP/1.0 200 OK\r\n");
   sprintf(buf, "%sServer: OS-HW3 Web Server\r\n", buf);

   //===== here we print to buf the stats:
   struct timeval dispatch_interval;
   timersub(&(var->leave_time), &(var->enter_time), &dispatch_interval);
   sprintf(buf, "%sStat-Req-Arrival:: %lu.%06lu\r\n", buf, var->enter_time.tv_sec, var->enter_time.tv_usec);
   sprintf(buf, "%sStat-Req-Dispatch:: %lu.%06lu\r\n", buf, dispatch_interval.tv_sec, dispatch_interval.tv_usec);
   sprintf(buf, "%sStat-Thread-Id:: %d\r\n", buf, stats->thread_id);
   sprintf(buf, "%sStat-Thread-Count:: %d\r\n", buf, stats->thread_req_count);
   sprintf(buf, "%sStat-Thread-Static:: %d\r\n", buf, stats->thread_static_req_count);
   sprintf(buf, "%sStat-Thread-Dynamic:: %d\r\n", buf, stats->thread_dynamic_req_count);
   Rio_writen(fd, buf, strlen(buf));

   pid_t new_pid = Fork();
   if (new_pid == 0)
   {
      /* Child process */
      Setenv("QUERY_STRING", cgiargs, 1);
      /* When the CGI process writes to stdout, it will instead go to the socket */
      Dup2(fd, STDOUT_FILENO);
      Execve(filename, emptylist, environ);
   }
   WaitPid(new_pid, NULL, 0); // a small change of the staff code. instead of `Wait(NULL);`.
}

void requestServeStatic(ConnVar var, char *filename, int filesize, Stats stats)
{
   int fd = var->connfd;
   int srcfd;
   char *srcp, filetype[MAXLINE], buf[MAXBUF];

   requestGetFiletype(filename, filetype);

   srcfd = Open(filename, O_RDONLY, 0);

   // Rather than call read() to read the file into memory,
   // which would require that we allocate a buffer, we memory-map the file
   srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
   Close(srcfd);

   // put together response
   sprintf(buf, "HTTP/1.0 200 OK\r\n");
   sprintf(buf, "%sServer: OS-HW3 Web Server\r\n", buf);
   sprintf(buf, "%sContent-Length: %d\r\n", buf, filesize);
   sprintf(buf, "%sContent-Type: %s\r\n", buf, filetype);
   //===== here we print to buf the stats:
   printStatsToBuf(buf, var, stats);

   Rio_writen(fd, buf, strlen(buf));

   //  Writes out to the client socket the memory-mapped file
   Rio_writen(fd, srcp, filesize);
   Munmap(srcp, filesize);
}

/**
 * requestHandle: Handles a request. Also update the thread stats.
 * @param var
 * @param stats
 */ 
void requestHandle(ConnVar var, Stats thread_stats)
{
   int fd = var->connfd;
   int is_static;
   struct stat sbuf;
   char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
   char filename[MAXLINE], cgiargs[MAXLINE];
   rio_t rio;

   Rio_readinitb(&rio, fd);
   Rio_readlineb(&rio, buf, MAXLINE);
   sscanf(buf, "%s %s %s", method, uri, version);

   printf("%s %s %s\n", method, uri, version);

   thread_stats->thread_req_count++; // increment this field anyway.

   if (strcasecmp(method, "GET"))
   {
      requestError(var, thread_stats, method, "501", "Not Implemented", "OS-HW3 Server does not implement this method");
      return;
   }
   requestReadhdrs(&rio);

   is_static = requestParseURI(uri, filename, cgiargs);
   if (stat(filename, &sbuf) < 0)
   {
      requestError(var, thread_stats, filename, "404", "Not found", "OS-HW3 Server could not find this file");
      return;
   }

   if (is_static)
   {
      //===== request is static:
      if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
      {
         requestError(var, thread_stats, filename, "403", "Forbidden", "OS-HW3 Server could not read this file");
         return;
      }
      thread_stats->thread_static_req_count++;
      requestServeStatic(var, filename, sbuf.st_size, thread_stats);
   }
   else
   {
      //===== request is dynamic:
      if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
      {
         requestError(var, thread_stats, filename, "403", "Forbidden", "OS-HW3 Server could not run this CGI program");
         return;
      }
      thread_stats->thread_dynamic_req_count++;
      requestServeDynamic(var, filename, cgiargs, thread_stats);
   }
}
