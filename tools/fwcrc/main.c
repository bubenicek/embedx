
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../../core/lib/crc32.h"
#include "../../core/lib/bootloader/app_header.h"

int main(int argc, char *argv[])
{
   int res;
   FILE *fw;
   char *filename;
   uint32_t hdr_offset;
   app_header_t hdr;
   struct stat st;
   uint32_t crc = 0;
   uint8_t buf[1024];

   if (argc < 3)
   {
      printf("Bad number of params, usage: ./fwcrc {filename} {app_header_offset}\n");
      return 1;
   }
   filename = argv[1];
   sscanf(argv[2], "%x", &hdr_offset);   // must be given as HEX number!

   // Get FW size
   if (stat(filename, &st) < 0)
   {
      printf("Can't open firmware file %s\n", filename);
      return 1;
   }

   // Open firmware file
   if ((fw = fopen(filename, "r+")) == NULL)
   {
      printf("Can't open firmware file %s\n", filename);
      return 1;
   }

   // Read fw header
   if (fseek(fw, hdr_offset, SEEK_SET) < 0)
   {
      printf("Seek to header failed\n");
      return 1;
   }
   if (fread(&hdr, sizeof(char), sizeof(hdr), fw) != sizeof(hdr))
   {
      printf("Read fw header failed\n");
      return 1;
   }

   // Modify header
   hdr.magic = APP_HEADER_MAGIC;
   hdr.fw_size = st.st_size;
   hdr.fw_crc = 0;

   // Write header
   if (fseek(fw, hdr_offset, SEEK_SET) < 0)
   {
      printf("Seek to header failed\n");
      return 1;
   }
   if (fwrite(&hdr, sizeof(char), sizeof(hdr), fw) != sizeof(hdr))
   {
      printf("Write fw header failed\n");
      return 1;
   }

   fflush(fw);
   rewind(fw);

   // Count CRC32
   while((res = fread(buf, 1, sizeof(buf), fw)) > 0)
   {
      // count crc32
      crc = crc32(crc, buf, res);
   }
   if (res < 0)
   {
      printf("Read from fw file failed\n");
      return 1;
   }

   // Modify header
   hdr.fw_crc = crc;

   // Write header
   if (fseek(fw, hdr_offset, SEEK_SET) < 0)
   {
      printf("Seek to header failed\n");
      return 1;
   }
   if (fwrite(&hdr, sizeof(char), sizeof(hdr), fw) != sizeof(hdr))
   {
      printf("Write fw header failed\n");
      return 1;
   }

   fclose(fw);

   printf("Write firmware CRC: 0x%X at offset: 0x%X\n", crc, hdr_offset);

   return 0;
}
