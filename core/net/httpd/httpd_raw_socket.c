
#include "httpd.h"

TRACE_TAG(httpd_socket);
#if !ENABLE_TRACE_HTTPD_SOCKET
#undef TRACE
#define TRACE(...)
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL    0x4000      /* Do not generate SIGPIPE.  */
#endif // MSG_NOSIGNAL

int httpd_raw_socket_create(int port)
{
   int sd, opt;
   struct sockaddr_in serveraddr;

   // Create socket
   if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      TRACE_ERROR("Create socket");
      goto fail_socket;
   }
   
   opt = 1;
   if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0)
   {
      TRACE_ERROR("setsockopt(SO_REUSEADDR) failed");  
      goto fail_bind;
   }
      
   // Bind socket
   memset(&serveraddr, 0, sizeof(serveraddr));
   serveraddr.sin_family = AF_INET;
   serveraddr.sin_addr.s_addr = INADDR_ANY;
   serveraddr.sin_port = htons(port);
   if (bind(sd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
   {
      TRACE_ERROR("Bind socket to port: %d", port);
      goto fail_bind;
   }

   if (listen(sd, CFG_HTTPD_LISTEN_BACKLOG) < 0)
   {
      TRACE_ERROR("Socket listen");
      goto fail_listen;
   }

   return sd;

fail_listen:
fail_bind:
   closesocket(sd);
fail_socket:
   return -1;
}

int httpd_raw_socket_connect(const char *host, int port)
{
   int sd;
   struct sockaddr_in server_addr;
   struct hostent *server_host;

   if ((server_host = gethostbyname(host)) == NULL)
   {
    	TRACE_ERROR("gethostbyname '%s'", host);
      return -1;
   }

   memcpy( (void *) &server_addr.sin_addr, (void *) server_host->h_addr, server_host->h_length);

   if ((sd = socket( AF_INET, SOCK_STREAM, IPPROTO_IP )) < 0)
   {
      TRACE_ERROR("socket failed");
      return -1;
   }

   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(port);

   if (connect(sd, (struct sockaddr *) &server_addr, sizeof( server_addr ) ) < 0)
   {
      closesocket(sd);
      return -1;
   }
   
   TRACE("Socket connected to %s:%d", host, port);

   return sd;
}

int httpd_raw_socket_close(int sd)
{
   return closesocket(sd);
}

int httpd_raw_socket_accept(int sd, struct sockaddr_in *remote_addr)
{
   int res;
   int newsd;
   fd_set read_fds;   
   socklen_t addrlen = sizeof(struct sockaddr_in);
   
   FD_ZERO(&read_fds);
   FD_SET(sd, &read_fds);

   // wait infinite for incomming connection
   if ((res = select(sd+1, &read_fds, NULL, NULL, NULL)) < 0)
   {
      if (errno == EINTR)
      {
         // select interrupt by signal
         TRACE("Wait for connection interrupted by signal");
         return -1;
      }
      else
      {
         TRACE_ERROR("Wait for connection select error = %d", errno);
         return -1;
      }
   }
   else if (!res)
   {
      TRACE("Wait for connection timeout");
      return -1;
   }

   if ((newsd = accept(sd, (struct sockaddr *)remote_addr, &addrlen)) < 0)
   {
      TRACE_ERROR("accept error");
      return -1;
   }
   
   return newsd;
}

int httpd_raw_socket_send(int sd, const void *buf, int count)
{
   return send(sd, buf, count, MSG_NOSIGNAL);
}

int httpd_raw_socket_recv(int sd, void *buf, int bufsz)
{
   int res;
#if defined(CFG_HTTPD_READ_SOCKET_TIMEOUT) && (CFG_HTTPD_READ_SOCKET_TIMEOUT > 0)
   fd_set read_fds;
   struct timeval tv;

   // wait for data
   FD_ZERO(&read_fds);
   FD_SET(sd, &read_fds);
   tv.tv_sec = 0;
   tv.tv_usec = CFG_HTTPD_READ_SOCKET_TIMEOUT * 1000;

   if ((res = select(sd+1, &read_fds, NULL, NULL,  &tv)) == 0)
   {
      // timeout
      return -2;
   }
   else if (res < 0)
   {
      TRACE_ERROR("select()");
      return -1;
   }
#endif

   // receive data
   res = recv(sd, buf, bufsz, 0);
   if (!res)
   {
      // connection closed
      TRACE("Connection disconnected");
   }

   return res;
}

int httpd_raw_socket_readto(int sd, char *buf, int bufsize, char c)
{
   int res;
   int total = 0;

   while (1)
   {
      if ((res = httpd_raw_socket_recv(sd, buf, 1)) <= 0)
         return res;

      total++;

      if (*buf == c)
      {
         break;
      }

      buf++;
   }

   buf[total] = '\0';

   return total;
}

