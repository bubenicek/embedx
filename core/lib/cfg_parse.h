/* config file parser */
/* Greg Kennedy 2012 */

#ifndef CFG_STRUCT_H_
#define CFG_STRUCT_H_

#define CFG_MAX_LINE 1024

typedef enum
{
    CFG_ITEM_BOOL,
    CFG_ITEM_INTEGER,
    CFG_ITEM_DOUBLE,
    CFG_ITEM_STRING

} cfg_item_type_t;

typedef struct
{
    const char *name;
    cfg_item_type_t type;
    void *value;
    int size;

} cfg_item_t;

struct cfg_struct;

/* Create a cfg_struct */
struct cfg_struct * cfg_init();

/* Free a cfg_struct */
void cfg_free(struct cfg_struct *);

/* Load into cfg from a file */
int cfg_load(struct cfg_struct *, const char *);

/* Save complete cfg to file */
int cfg_save(struct cfg_struct *, const char *);

/* Get value from cfg_struct by key */
const char * cfg_get(struct cfg_struct *, const char *);

/* Set key,value in cfg_struct */
int cfg_set(struct cfg_struct *, const char *, const char *);

/* Delete key (+value) from cfg_struct */
void cfg_delete(struct cfg_struct *, const char *);

const char *cfg_get_str(struct cfg_struct *cfg, const char *key, const char *default_value);
int cfg_set_str(struct cfg_struct *cfg, const char *key, const char *value);

int cfg_get_int(struct cfg_struct *cfg, const char *key, int default_value);
int cfg_set_int(struct cfg_struct *cfg, const char *key, int value);

bool cfg_get_bool(struct cfg_struct *cfg, const char *key, bool default_value);
int cfg_set_bool(struct cfg_struct *cfg, const char *key,  bool value);

double cfg_get_double(struct cfg_struct *cfg, const char *key, double default_value);
int cfg_set_double(struct cfg_struct *cfg, const char *key, double value);


int cfg_load_items(struct cfg_struct *cfg, const char *filename, const cfg_item_t items[], int items_count);
int cfg_save_items(struct cfg_struct *cfg, const char *filename, const cfg_item_t items[], int items_count);


#endif
