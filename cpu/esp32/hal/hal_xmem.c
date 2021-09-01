
#include "system.h"

TRACE_TAG(hal_xmem);
#if ! ENABLE_TRACE_HAL
#undef TRACE
#define TRACE(...)
#endif

// Locals:
static const esp_partition_t *partition = NULL;

int hal_xmem_init(void)
{
   // Find the partition map in the partition table
   if ((partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "storage")) == NULL)
   {
      TRACE_ERROR("Can not find partition storage");
      return -1;
   }

   return 0;
}

int hal_xmem_read(uint32_t offset, void *buf, int nbytes)
{
   return (esp_partition_read(partition, offset, buf, nbytes) != 0) ? -1 : nbytes;
}

int hal_xmem_write(uint32_t offset, const void *buf, int nbytes)
{
   return (esp_partition_write(partition, offset, buf, nbytes) != 0) ? -1 : nbytes;
}

int hal_xmem_erase(uint32_t offset, uint32_t nbytes)
{
   return esp_partition_erase_range(partition, offset, nbytes);
}
