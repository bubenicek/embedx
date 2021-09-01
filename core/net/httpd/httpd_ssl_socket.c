
#include "httpd.h"

#if defined(CFG_HTTPD_USE_SSL_SOCKET) && (CFG_HTTPD_USE_SSL_SOCKET == 1)

#include "lwip/netif.h"

#include "polarssl/entropy.h"
#include "polarssl/ctr_drbg.h"
#include "polarssl/certs.h"
#include "polarssl/x509.h"
#include "polarssl/ssl.h"
#include "polarssl/net.h"
#include "polarssl/error.h"

#if defined(POLARSSL_SSL_CACHE_C)
#include "polarssl/ssl_cache.h"
#endif

#include "httpd.h"

#define TRACE_TAG "httpd-socket"
#if !ENABLE_TRACE_HTTPD_SOCKET
#undef TRACE
#undef TRACE_PRINTFF
#undef TRACE_PRINTF
#define TRACE(...)
#define TRACE_PRINTF(...)
#define TRACE_PRINTFF(...)
#endif

// Prototypes:
static void ssl_debug(void *ctx, int level, const char *str);
static int ssl_random(void* arg, unsigned char *output, size_t output_len);

// Locals:
static int listen_fd;
static int client_fd = -1;
static ssl_context ssl;
static x509_cert srvcert;
static rsa_context rsa;
#if defined(POLARSSL_SSL_CACHE_C)
static ssl_cache_context cache;
#endif


int httpd_ssl_socket_create(int port)
{
   int ret;

#if defined(POLARSSL_SSL_CACHE_C)
   ssl_cache_init(&cache);
#endif

   //
   // Load the certificates and private RSA key
   //
   memset(&srvcert, 0, sizeof(x509_cert));

   //
   // This demonstration program uses embedded test certificates.
   // Instead, you may want to use x509parse_crtfile() to read the
   // server and CA certificates, as well as x509parse_keyfile().
   //
   ret = x509parse_crt(&srvcert, (const unsigned char *) test_srv_crt, strlen( test_srv_crt));
   if (ret != 0)
   {
      TRACE_ERROR("x509parse_crt returned %d", ret);
      goto fail;
   }

   rsa_init(&rsa, RSA_PKCS_V15, 0);
   ret = x509parse_key(&rsa, (const unsigned char *) test_srv_key, strlen( test_srv_key ), NULL, 0);
   if (ret != 0)
   {
      TRACE_ERROR("x509parse_key returned %d", ret);
      goto fail;
   }

   //
   // Setup the listening TCP socket
   //
   if ((ret = net_bind(&listen_fd, NULL, port)) != 0)
   {
      TRACE_ERROR("net_bind returned %d", ret);
      goto fail;
   }

   //
   // Setup stuff
   //
   if ((ret = ssl_init(&ssl)) != 0)
   {
      TRACE_ERROR("ssl_init returned %d", ret);
      goto fail;
   }

   ssl_set_endpoint(&ssl, SSL_IS_SERVER);
   ssl_set_authmode(&ssl, SSL_VERIFY_NONE);

   ssl_set_rng(&ssl, ssl_random , NULL);
#if defined(ENABLE_TRACE_HTTPD_SSL_SOCKET) && (ENABLE_TRACE_HTTPD_SSL_SOCKET == 1)
   ssl_set_dbg(&ssl, ssl_debug, NULL);
#endif   

#if defined(POLARSSL_SSL_CACHE_C)
   ssl_set_session_cache(&ssl, ssl_cache_get, &cache, ssl_cache_set, &cache);
#endif

   ssl_set_ca_chain(&ssl, srvcert.next, NULL, NULL);
   ssl_set_own_cert(&ssl, &srvcert, &rsa);
   ssl_set_bio(&ssl, net_recv, &client_fd, net_send, &client_fd);
   
   return listen_fd;
   
fail:
   return -1;
}

int httpd_ssl_socket_close(int sd)
{
   if (sd != -1)
   {
      if (sd == client_fd)
      {
         net_close(client_fd);         
         ssl_session_reset(&ssl);      
         client_fd = -1;
      }
      else if (sd == listen_fd)
      {
         x509_free(&srvcert);
         rsa_free(&rsa);
         ssl_free(&ssl);
#if defined(POLARSSL_SSL_CACHE_C)
         ssl_cache_free(&cache);
#endif       
         listen_fd = -1;
      }
      
      return 0;      
   }
   else
   {
      return -1;
   }
}

int httpd_ssl_socket_accept(int sd, struct sockaddr_in *remote_addr)
{
   int ret;
   uint32_t start_tm;
   
   TRACE("Waiting for a remote connection ...");
 
   client_fd = -1;
   if ((ret = net_accept(listen_fd, &client_fd, NULL)) != 0)
   {
      TRACE_ERROR("net_accept returned %d\n\n", ret);
      goto fail;
   }
   
   TRACE_PRINTFF("Handshaking ... ");
   start_tm = hal_time_ms();
 
   // Handshake
   while ((ret = ssl_handshake(&ssl)) != 0)
   {
      if (ret != POLARSSL_ERR_NET_WANT_READ && ret != POLARSSL_ERR_NET_WANT_WRITE)
      {
         TRACE_ERROR("ssl_handshake returned -0x%x\n\n", -ret);
         goto reset;
      }
   }

   TRACE_PRINTF("done, tmlen: %d (ms)\n", hal_time_ms() - start_tm);
  
   return client_fd;

reset:   
   if (client_fd != -1)
   {
      net_close(client_fd);
      client_fd = -1;
   }
   ssl_session_reset(&ssl);
fail:
   return -1;
}

int httpd_ssl_socket_send(int sd, const void *buf, int count)
{
   return ssl_write(&ssl, buf, count);
}

int httpd_ssl_socket_recv(int sd, void *buf, int bufsize)
{
   int ret;

   do 
   {
      ret = ssl_read(&ssl, buf, bufsize);   
   
   } while (ret == POLARSSL_ERR_NET_WANT_READ || ret == POLARSSL_ERR_NET_WANT_WRITE);
   
   if (ret < 0)
   {
      switch(ret)
      {
         case POLARSSL_ERR_SSL_PEER_CLOSE_NOTIFY:
            TRACE("connection was closed gracefully");
            ret = 0;
            break;
       
         case POLARSSL_ERR_NET_CONN_RESET:
            TRACE("connection was reset by peer");
            ret = 0;
            break;
       
         default:
            TRACE_ERROR("ssl_read returned -0x%x", -ret);
            ret = -1;
      }     
   } 
   
   return ret;  
}

int httpd_ssl_socket_readto(int sd, char *buf, int bufsize, char c)
{
   int res;
   int total = 0;

   while (bufsize > 0)
   {
      if ((res = httpd_ssl_socket_recv(sd, buf, 1)) <= 0)
         return res;

      total++;
      bufsize--;
      
      if (*buf++ == c)
      {
         *buf = '\0';
         break;
      }      
   }

   return total;
}

/** SSL debug output */
static void ssl_debug(void *ctx, int level, const char *str)
{
   TRACE_PRINTF("%s", str);  
}

/**
  * @brief  Returns a random number.
  * @param  arg not used
  * @param  output random number
  * @param  output_len random number length
  * @retval 0
  */
static int ssl_random(void* arg, unsigned char *output, size_t output_len)
{
   uint32_t nbrng = 0;
   uint8_t offset = 0;

   nbrng = output_len;

   while (nbrng > 0)
   {
      // Get the random number 
      *(output + offset) = hal_rand_number() && 0xFFFFFF00;

      offset++;
      nbrng --;
   }

   // Return the random number 
   return(0);
}

#endif   // CFG_HTTPD_USE_SSL_SOCKET
