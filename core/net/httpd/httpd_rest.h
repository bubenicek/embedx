
#ifndef __HTTPD_REST_H
#define __HTTPD_REST_H

#define CFG_REST_INPUT_MAXNUM_PARAMS      16
#define CFG_REST_INPUT_MAXNUM_OBJECTS     4

#define CFG_REST_INPUT_MAXNUM_TOKENS      (CFG_REST_INPUT_MAXNUM_PARAMS * 2)


typedef enum
{
   REST_API_OK = 0,
   REST_API_ERR = -1,
   REST_API_ERR_NOTFOUND = -2,
   REST_API_ERR_FORMAT = -3,

} rest_api_ret_t;

//
// Input REST data interface
//

typedef enum
{
	REST_PRIMITIVE = 0,
	REST_OBJECT    = 1,
	REST_ARRAY     = 2,
	REST_STRING    = 3

} rest_input_param_type_t;


/** REST input parameter definition */
typedef struct
{
   rest_input_param_type_t type;
   const char *name;            // REST API name
   const char *colname;         // Column name in the table
   uint8_t base_object_param;   // Base object parameter

} rest_input_param_def_t;


/** REST input parameter */
typedef struct
{
   const rest_input_param_def_t *def;
   const char *name;
   const char *value;

} rest_input_param_t;


/** REST input object */
typedef struct
{
   const char *objname;
   rest_input_param_t params[CFG_REST_INPUT_MAXNUM_PARAMS];
   int params_count;

} rest_input_object_t;


/** REST input data */
typedef struct
{
   rest_input_object_t objects[CFG_REST_INPUT_MAXNUM_OBJECTS];
   int objects_count;

} rest_input_data_t;


/** Rest API object definition */
typedef struct
{
   char *objname;
   char *tablename;
   rest_input_param_def_t params[CFG_REST_INPUT_MAXNUM_PARAMS];

} rest_api_object_t;

/** Get REST object by name */
const rest_api_object_t *rest_api_get_object(const char *objname);


/** Get REST input parameter */
rest_input_param_t *rest_get_input_param(rest_input_data_t *data, const char *objname, const char *parname);

/** Get REST inpujt parameter definition */
const rest_input_param_def_t *rest_get_input_param_def(const rest_input_param_def_t *defs, const char *name);

/** Parse REST input data */
extern int rest_input_init(struct httpd_connection *con, const rest_api_object_t *objdef, rest_input_data_t *data);
/** Parse REST input array data */
extern int rest_input_array_init(struct httpd_connection *con, const rest_api_object_t *objdef, rest_input_data_t *data);


//
// Output REST data interface
//
extern int rest_output_begin(struct httpd_connection *httpcon, int result, const char *msgtext);
extern int rest_output_end(struct httpd_connection *httpcon);

#define rest_output_result(_con, _result, _text) \
({ \
  int _res = rest_output_begin(_con, _result, _text); \
  _res += rest_output_end(_con); \
  _res; \
})

extern int rest_output_array_begin(struct httpd_connection *httpcon, const char *name);
extern int rest_output_array_end(struct httpd_connection *httpcon);

extern int rest_output_object_begin(struct httpd_connection *httpcon, const char *name);
extern int rest_output_object_end(struct httpd_connection *httpcon);

extern int rest_output_value_str(struct httpd_connection *httpcon, const char *name, const char *fmt, ...);
extern int rest_output_value_int(struct httpd_connection *httpcon, const char *name, int value);
extern int rest_output_value_double(struct httpd_connection *httpcon, const char *name, double value);
extern int rest_output_value_bool(struct httpd_connection *httpcon, const char *name, int value);

struct httpd_rest_call;

/** REST function type */
typedef int (*httpd_rest_function_t)(struct httpd_connection *con, const struct httpd_rest_call *restcall, const char *argv[], int argc);


/** REST function call */
typedef struct httpd_rest_call
{
  char *name;
  httpd_rest_function_t get_func;
  httpd_rest_function_t update_func;
  httpd_rest_function_t insert_func;
  httpd_rest_function_t delete_func;
  const rest_api_object_t *objdef;

} httpd_rest_call_t;

const httpd_rest_call_t *httpd_get_rest_call(struct httpd_connection *s, char *name, const char *argv[], int *argc);


//
// REST authorization
//
extern int httpd_rest_authorize(struct httpd_connection *con, const httpd_rest_call_t *restcall);


#endif // __HTTPD_REST_H
