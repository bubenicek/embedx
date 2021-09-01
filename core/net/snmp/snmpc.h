
#ifndef __SNMPC_H
#define __SNMPC_H

#include "net-snmp/net-snmp-config.h"
#include "net-snmp/utilities.h"
#include "net-snmp/net-snmp-includes.h"


typedef struct
{
   netsnmp_session session, *ss;  
   
} snmpc_t;


/** Open SNMP connection */
int snmpc_open(snmpc_t *c, const char *host, const char *comunity);

/** Close SNMP connection */
int snmpc_close(snmpc_t *c);

/** Get integer value */
int snmpc_get_int_value(snmpc_t *c, const char *oid, int *value);

/** Get string value */
int snmpc_get_str_value(snmpc_t *c, const char *oid, char *buf, int busize);


#endif // __SNMPC_H
