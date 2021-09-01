
#include "httpd.h"

TRACE_TAG(httpd_rest);
#if !ENABLE_TRACE_HTTPD_REST
#undef TRACE
#define TRACE(...)
#endif

// external vars
extern const httpd_rest_call_t httpd_restcalls[];
extern const rest_api_object_t *rest_api_objects[];


/** Get REST call definition by name */
const httpd_rest_call_t *httpd_get_rest_call(struct httpd_connection *s, char *name, const char *argv[], int *argc)
{
  int ix;
  int pos;
  char *p1, *p2, *pp;
  const httpd_rest_call_t *restcall = httpd_restcalls;

  while(restcall->name != NULL)
  {
    pos = 0;
    p1 = name;
    p2 = (char *)restcall->name;

    while (*p2 != '\0' && *p1 != '\0')
    {
      if (*p2 == '{')
      {
        p2 = strchr(p2, '}');
        if (p2 != NULL)
        {
          argv[pos++] = p1;
          pp = strchr(p1, '/');
          if (pp != NULL)
          {
            p1 = pp;
            p1++;  // Skip '/'
          }
          else
          {
            // seek to end of p1
            for (; *p1 != '\0'; p1++);
          }
           
          p2++;             // Skip '}'
          if (*p2 == '/')
            p2++;           // Skip '/'
        }
        else
        {
          TRACE_ERROR("Bad REST call: '%s' param", restcall->name);
          break;
        }
      }
      else
      {
        if (*p2 != *p1)
          break;

        p2++;
        p1++;
      }
    }

    if (*p2 == '\0' && *p1 == '\0')
    {
      TRACE("Found REST call name: '%s'  argc: %d", restcall->name, pos);

      // Terminate REST URL values in string
      for (ix = 0; ix < pos; ix++)
      {
        pp = strchr(argv[ix], '/');
        if (pp != NULL)
          *pp = '\0';
      }

      for (ix = 0; ix < pos; ix++)
      {
        TRACE("   argv[%d] = '%s'", ix, argv[ix]);
      }

      *argc = pos;

      return restcall;
    }

    restcall++;
  }

  return NULL;
}

/** Get REST input parameter */
rest_input_param_t *rest_get_input_param(rest_input_data_t *data, const char *objname, const char *parname)
{
   int ix, io;
   
   for (io = 0; io < data->objects_count; io++)
   {
      for (ix = 0; ix < data->objects[io].params_count; ix++)
      {
         ASSERT(data->objects[io].params[ix].def != NULL);
         
         if (objname != NULL)
         {
            if (!strcmp(objname, data->objects[io].objname) && 
                !strcmp(data->objects[io].params[ix].def->name, parname))
            {
               return &data->objects[io].params[ix];
            }
         }
         else
         {
            if (!strcmp(data->objects[io].params[ix].def->name, parname))
               return &data->objects[io].params[ix];
         }
      }
   }

   return NULL;
}

/** Get REST input parameter definition */
const rest_input_param_def_t *rest_get_input_param_def(const rest_input_param_def_t *defs, const char *name)
{  
   while(defs != NULL && defs->name != NULL)
   {
      if (!strcmp(defs->name, name))
         return defs;
      
      defs++;
   }
   
   return NULL;
}

