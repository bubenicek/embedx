

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "voip_codec/voip_codec.h"


#define SAMPLES_PER_FRAME       160
#define BYTES_PER_FRAME         6



int main(int argc, char *argv[])
{
    int ix, frame_index = 0;
    voip_codec_t codec;
    voip_codec_params_t codec_params;
    FILE *fr;
    FILE *fw;
    int16_t samples_buf[SAMPLES_PER_FRAME];
    uint8_t bits_buf[BYTES_PER_FRAME];
    const char *filename;
    char path[255];

    if (argc < 2)
    {
        voip_error("Bad number of params");
        return 1;
    }

    filename = argv[1];

    // Open codec
    if (voip_codec_open(&codec, VOIP_CODEC_TYPE_CODEC2, &codec_params) != 0)
    {
        voip_error("open codec");
        return 1;
    }

    voip_debug("codec open ok, samples: %d  bytes: %d  per frame", codec_params.samples_per_frame, codec_params.bytes_per_frame);


    //
    // Test encoder
    //

    fr = fopen(filename, "r");
    if (fr == NULL)
    {
        voip_error("Can't open raw audio file: %s", filename);
        return 1;
    }

    sprintf(path, "%s.c2", filename);
    fw = fopen(path, "w");
    if (fw == NULL)
    {
        voip_error("Can't create C2 file: %s", path);
        return 1;
    }

    while(fread(samples_buf, sizeof(short), SAMPLES_PER_FRAME, fr) > 0)
    {
        voip_codec_encode(&codec, samples_buf, SAMPLES_PER_FRAME, bits_buf, BYTES_PER_FRAME);
        fwrite(bits_buf, sizeof(char), BYTES_PER_FRAME, fw);
    }

    voip_debug("File %s encoded to %s", filename, path);

    fclose(fw);
    fclose(fr);


    //
    // Test decoder
    //

    fr = fopen(path, "r");
    if (fr == NULL)
    {
        voip_error("Can't open C2 compressed file: %s", path);
        return 1;
    }

    strcat(path, ".raw");
    fw = fopen(path, "w");
    if (fw == NULL)
    {
        voip_error("Can't create decoded raw C2 file: %s", path);
        return 1;
    }

    while(fread(bits_buf, sizeof(char), BYTES_PER_FRAME, fr) > 0)
    {
        voip_codec_decode(&codec, bits_buf, BYTES_PER_FRAME, samples_buf, SAMPLES_PER_FRAME);
        fwrite(samples_buf, sizeof(short), SAMPLES_PER_FRAME, fw);
    }

    voip_debug("File %s decoded to %s", filename, path);

    fclose(fw);
    fclose(fr);


    // Close codec
    voip_codec_close(&codec);

    return 0;
}
