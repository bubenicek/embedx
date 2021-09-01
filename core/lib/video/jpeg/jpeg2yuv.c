
#include "system.h"

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#define TRACE_TAG   "jpeg2yuv"


typedef struct
{
    const uint8_t *data;
    int size;
    int pos;

} memory_file_t;


static int memfile_read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    memory_file_t *f = opaque;

    ASSERT(f != NULL);

    int size = buf_size;
    if (f->size - f->pos < buf_size)
        size = f->size - f->pos;

    if (size > 0)
    {
        memcpy(buf, f->data + f->pos, size);
        f->pos += size;
    }

    return size;
}

static int64_t memfile_seek(void* opaque, int64_t offset, int whence)
{
    memory_file_t *f = opaque;

    ASSERT(f != NULL);

    switch (whence)
    {
        case SEEK_SET:
            f->pos = offset;
            break;
        case SEEK_CUR:
            f->pos += offset;
            break;
        case SEEK_END:
            f->pos = f->size - offset;
            break;
        case AVSEEK_SIZE:
            return f->size;
            break;
    }

    return f->pos;
}

/** Convert JPG image buffer to YUV buffer */
int convert_jpeg2yuv420(const uint8_t *jpegdata, int jpegdata_size, uint8_t **yuvdata, int *yuvdata_size, int *width, int *height)
{
    static bool initialized = false;
    AVFormatContext *iFormatContext = NULL;
    int videoStreamIndex = -1;
    AVCodecContext *pCodecCtx = NULL;
    AVCodec *pCodec = NULL;
    AVPacket encodedPacket;
    int frameFinished = 0;
    AVFrame *decodedFrame = NULL;
    AVPicture destPic;
    enum AVPixelFormat destFormat = AV_PIX_FMT_YUV420P;
    struct SwsContext *ctxt = NULL;
    memory_file_t memfile = {.data = jpegdata, .size = jpegdata_size, .pos = 0};
    uint8_t *rawdata;
    int rawdata_size;

    if (!initialized)
    {
        av_register_all();

        av_log_set_level(AV_LOG_QUIET);

        initialized = true;
    }

    AVInputFormat* inf = av_find_input_format("mjpeg");
    if (inf == NULL)
    {
        TRACE_ERROR("probe failed");
        throw_exception(fail_find_format);
    }

    unsigned char* avbuff = av_malloc(4096);
    AVIOContext* ioctx = avio_alloc_context(avbuff, 4096, 0, &memfile, memfile_read_packet, NULL, memfile_seek);
    iFormatContext = avformat_alloc_context();
    iFormatContext->pb = ioctx;
    if (avformat_open_input(&iFormatContext, "nofile.jpg", inf, NULL) != 0)
    {
        TRACE_ERROR("Error opening AVFormatContext");
        throw_exception(fail_open_jpeg);
    }

    // Finding stream information
    if (avformat_find_stream_info(iFormatContext, NULL) < 0)
    {
        TRACE_ERROR("Error in finding stream info");
        throw_exception(fail_init);
    }

    // Finding video stream from number of streams
    for (int a = 0; a < iFormatContext->nb_streams; a++)
    {
        if (iFormatContext->streams[a]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex = a;
            break;
        }
    }
    if (videoStreamIndex == -1)
    {
        TRACE_ERROR("Couldn't find video stream");
        throw_exception(fail_init);
    }

    // Finding decoder for video stream
    pCodecCtx = iFormatContext->streams[videoStreamIndex]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL)
    {
        TRACE_ERROR("Cannot find decoder");
        throw_exception(fail_init);
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        TRACE_ERROR("Cannot open decoder");
        throw_exception(fail_init);
    }

    // Reading video frame
    av_init_packet(&encodedPacket);
    encodedPacket.data = NULL;
    encodedPacket.size = 0;
    if (av_read_frame(iFormatContext, &encodedPacket)<0)
    {
        TRACE_ERROR("Cannot read frame");
        throw_exception(fail_init);
    }

    // Decoding the encoded video frame
    decodedFrame = av_frame_alloc();
    avcodec_decode_video2(pCodecCtx, decodedFrame, &frameFinished, &encodedPacket);
    if (!frameFinished)
    {
        TRACE_ERROR("Frame does not finished");
        throw_exception(fail_decoding);
    }

    //TRACE("W:%d  H:%d  F1:%d  F2:%d", decodedFrame->width, decodedFrame->height, decodedFrame->format, destFormat);

    // Converting the pixel format
    avpicture_alloc(&destPic, destFormat, decodedFrame->width, decodedFrame->height);
    ctxt = sws_getContext(
                decodedFrame->width, decodedFrame->height, decodedFrame->format,
                decodedFrame->width, decodedFrame->height, destFormat,
                SWS_BILINEAR, NULL, NULL, NULL);
    if (ctxt == NULL)
    {
        TRACE_ERROR("Error while calling sws_getContext");
        throw_exception(fail_decoding);
    }
    sws_scale(ctxt, decodedFrame->data, decodedFrame->linesize, 0, decodedFrame->height, destPic.data, destPic.linesize);
    sws_freeContext(ctxt);

    // Copying decoded frame to buffer
    rawdata_size = decodedFrame->height * decodedFrame->width * 2;
    if ((rawdata = malloc(rawdata_size)) == NULL)
    {
        TRACE_ERROR("Alloc yuvdata buffer failed");
        throw_exception(fail_decoding);
    }
    av_image_copy_to_buffer(rawdata, rawdata_size, destPic.data, destPic.linesize, destFormat, decodedFrame->width, decodedFrame->height, 1);

    *yuvdata = rawdata;
    *yuvdata_size = rawdata_size;
    *width = decodedFrame->width;
    *height = decodedFrame->height;

    av_free_packet(&encodedPacket);
    avcodec_close(pCodecCtx);
    avformat_close_input(&iFormatContext);
    av_free(ioctx);
    av_free(avbuff);
    avformat_free_context(iFormatContext);

    return 0;

fail_decoding:
    av_free_packet(&encodedPacket);
    avcodec_close(pCodecCtx);
fail_init:
    avformat_close_input(&iFormatContext);
fail_open_jpeg:
    av_free(ioctx);
    av_free(avbuff);
    avformat_free_context(iFormatContext);
fail_find_format:
    return -1;
}

/** Convert JPG image file to YUV buffer */
int convert_jpeg2yuv420_file(const char *jpg_filename, uint8_t **yuvdata, int *yuvdata_size)
{
    static bool initialized = false;
    AVFormatContext *iFormatContext = NULL;
    int videoStreamIndex = -1;
    AVCodecContext *pCodecCtx = NULL;
    AVCodec *pCodec = NULL;
    AVPacket encodedPacket;
    int frameFinished = 0;
    AVFrame *decodedFrame = NULL;
    AVPicture destPic;
    enum AVPixelFormat destFormat = AV_PIX_FMT_YUV420P;
    struct SwsContext *ctxt = NULL;
    uint8_t *rawdata;
    int rawdata_size;

    if (!initialized)
    {
        av_register_all();
        initialized = true;
    }

    // Allocating input format context
    if (avformat_open_input(&iFormatContext, jpg_filename, NULL, NULL) != 0)
    {
        TRACE_ERROR("Error in opening input file %s", jpg_filename);
        throw_exception(fail_open_jpeg);
    }

    // Finding stream information
    if (avformat_find_stream_info(iFormatContext, NULL) < 0)
    {
        TRACE_ERROR("Error in finding stream info");
        throw_exception(fail_init);
    }

    // Finding video stream from number of streams
    for (int a = 0; a < iFormatContext->nb_streams; a++)
    {
        if (iFormatContext->streams[a]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex = a;
            break;
        }
    }
    if (videoStreamIndex == -1)
    {
        TRACE_ERROR("Couldn't find video stream");
        throw_exception(fail_init);
    }

    // Finding decoder for video stream
    pCodecCtx = iFormatContext->streams[videoStreamIndex]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL)
    {
        TRACE_ERROR("Cannot find decoder");
        throw_exception(fail_init);
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        TRACE_ERROR("Cannot open decoder");
        throw_exception(fail_init);
    }

    // Reading video frame
    av_init_packet(&encodedPacket);
    encodedPacket.data = NULL;
    encodedPacket.size = 0;
    if (av_read_frame(iFormatContext, &encodedPacket)<0)
    {
        TRACE_ERROR("Cannot read frame");
        throw_exception(fail_init);
    }

    // Decoding the encoded video frame
    decodedFrame = av_frame_alloc();
    avcodec_decode_video2(pCodecCtx, decodedFrame, &frameFinished, &encodedPacket);
    if (!frameFinished)
    {
        TRACE_ERROR("Frame does not finished");
        throw_exception(fail_decoding);
    }

    // Converting the pixel format
    avpicture_alloc(&destPic, destFormat, decodedFrame->width,decodedFrame->height);
    ctxt = sws_getContext(
                decodedFrame->width, decodedFrame->height,
                decodedFrame->format, decodedFrame->width, decodedFrame->height,
                destFormat, SWS_BILINEAR, NULL, NULL, NULL);
    if (ctxt == NULL)
    {
        TRACE_ERROR("Error while calling sws_getContext");
        throw_exception(fail_decoding);
    }
    sws_scale(ctxt, decodedFrame->data, decodedFrame->linesize, 0, decodedFrame->height, destPic.data, destPic.linesize);
    sws_freeContext(ctxt);

    // Copying decoded frame to buffer
    rawdata_size = decodedFrame->height * decodedFrame->width * 2;
    if ((rawdata = malloc(rawdata_size)) == NULL)
    {
        TRACE_ERROR("Alloc yuvdata buffer failed");
        throw_exception(fail_decoding);
    }
    av_image_copy_to_buffer(rawdata, rawdata_size, destPic.data, destPic.linesize, destFormat, decodedFrame->width, decodedFrame->height, 1);

    *yuvdata = rawdata;
    *yuvdata_size = rawdata_size;

    av_free_packet(&encodedPacket);
    avcodec_close(pCodecCtx);
    avformat_close_input(&iFormatContext);

    return 0;

fail_decoding:
    av_free_packet(&encodedPacket);
    avcodec_close(pCodecCtx);
fail_init:
    avformat_close_input(&iFormatContext);
fail_open_jpeg:
    return -1;
}
