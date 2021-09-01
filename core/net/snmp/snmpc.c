
#include "system.h"
#include "snmpc.h"

TRACE_TAG(snmp_client)
#if !ENABLE_TRACE_SNMP_CLIENT
#include "trace_undef.h"
#endif

/** Open SNMP connection */
int snmpc_open(snmpc_t *client, const char *host, const char *comunity)
{
   
   ASSERT(host != NULL);
   ASSERT(comunity != NULL);
   
   // Initialize session to default values 
   snmp_sess_init(&client->session);
   client->session.version = SNMP_VERSION_2c;
   client->session.peername = (char *)host;
   client->session.community = (uint8_t *)comunity;
   client->session.community_len = strlen(comunity);
   
   netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_QUICK_PRINT, 1);
   
   // Open an SNMP session.
   if ((client->ss = snmp_open(&client->session)) == NULL)
   {
      snmp_sess_perror("snmpget", &client->session);
      return -1;
   }
         
   TRACE("SNMP Client open connection to host: %s", host);
  
   return 0;
}

/** Close SNMP connection */
int snmpc_close(snmpc_t *client)
{
   return 0;
}

/** Get integer value */
int snmpc_get_int_value(snmpc_t *client, const char *name, int *value)
{
   char buf[255];
   
   if (snmpc_get_str_value(client, name, buf, sizeof(buf)) != 0)
      return -1;

   *value = atoi(buf);
      
   return 0;
}

/** Get string value */
int snmpc_get_str_value(snmpc_t *client, const char *name, char *buf, int bufsize)
{
   int status;
   netsnmp_pdu *pdu = NULL;
   netsnmp_pdu *response = NULL;   
   size_t oid_length = MAX_OID_LEN;
   oid oid[MAX_OID_LEN];
   
   // Create PDU for GET request and add object names to request.
   if ((pdu = snmp_pdu_create(SNMP_MSG_GET)) == NULL)
   {
      TRACE_ERROR("Create PDU");
      throw_exception(fail_pdu_create);
   }
      
   if (!snmp_parse_oid(name, oid, &oid_length)) 
   {
      TRACE_ERROR("Parse OID failed");
      snmp_perror(name);
      throw_exception(fail_parse_oid);
   } 
   snmp_add_null_var(pdu, oid, oid_length);
   
   // Exceute request
   status = snmp_synch_response(client->ss, pdu, &response);
   if (status == STAT_SUCCESS)
   {
      if (response->errstat == SNMP_ERR_NOERROR) 
      {
         if (response->variables == NULL)
         {
            TRACE_ERROR("Undefined variables");
            throw_exception(fail_request);
         }

         if (snprint_value(buf, bufsize, oid, oid_length, response->variables) < 0)
         {
            TRACE_ERROR("Format value failed");
            throw_exception(fail_request);
         }
         
         str_remove(buf, '"');
      } 
      else 
      {
         TRACE_ERROR("Error in packet - reason: %s", snmp_errstring(response->errstat));
         throw_exception(fail_request);
      }
   } 
   else if (status == STAT_TIMEOUT) 
   {
      TRACE_ERROR("Timeout: No Response from %s", client->session.peername);
      throw_exception(fail_request);
   } 
   else 
   {                    
        TRACE_ERROR("Get value failed");
        snmp_sess_perror("snmpget", client->ss);
        throw_exception(fail_request);
   }

   if (response)
      snmp_free_pdu(response);

   return 0;

fail_request:
fail_parse_oid:   
   if (response != NULL)
      snmp_free_pdu(response);
      
fail_pdu_create:
   return -1;
}
