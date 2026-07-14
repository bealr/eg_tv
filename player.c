#include "player.h"
#include <raylib.h>

#include <gst/gst.h>
#include <gst/app/gstappsink.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static GstElement *pipeline = NULL;
static GstElement *videoSink = NULL;


static int videoWidth = 0;
static int videoHeight = 0;

static GstBus *playerBus = NULL;

static bool finished = false;
static bool playing = false;


static Texture2D videoTexture = {0};

static unsigned char *textureBuffer = NULL;


static bool textureReady = false;

static void GetVideoInfo(GstSample *sample)
{
    GstCaps *caps =
        gst_sample_get_caps(sample);

    if(!caps)
    {
        printf("NO CAPS\n");
        return;
    }


    GstStructure *s =
        gst_caps_get_structure(caps,0);


    if(!gst_structure_get_int(
        s,
        "width",
        &videoWidth))
        return;


    if(!gst_structure_get_int(
        s,
        "height",
        &videoHeight))
        return;


    printf(
        "VIDEO SIZE %dx%d\n",
        videoWidth,
        videoHeight);


    textureBuffer =
        malloc(
            videoWidth *
            videoHeight *
            4);


    if(!textureBuffer)
    {
        printf("MEMORY ERROR\n");
        return;
    }


    textureReady = false;

    printf("TEXTURE CREATED\n");
}

bool PlayerInit(void)
{
    gst_init(NULL, NULL);

    return true;
}

static void CreateTexture(void)
{
    if(videoTexture.id != 0)
        UnloadTexture(videoTexture);


    Image img = {
        .data = textureBuffer,
        .width = videoWidth,
        .height = videoHeight,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };


    videoTexture = LoadTextureFromImage(img);
}

static void OnPadAdded(
    GstElement *decode,
    GstPad *pad,
    gpointer data)
{
    GstCaps *caps = gst_pad_get_current_caps(pad);

    if (!caps)
        return;


    GstStructure *str =
        gst_caps_get_structure(caps, 0);


    const char *name =
        gst_structure_get_name(str);


    GstElement *target =
        NULL;


    if (g_str_has_prefix(name, "video"))
    {
        printf("VIDEO PAD\n");

        target = gst_bin_get_by_name(
            GST_BIN(pipeline),
            "convert");
    }
    else if (g_str_has_prefix(name, "audio"))
    {
        printf("AUDIO PAD\n");

        target = gst_bin_get_by_name(
            GST_BIN(pipeline),
            "audioConvert");
    }


    if(target)
    {
        GstPad *sinkPad =
            gst_element_get_static_pad(
                target,
                "sink");


        if(!gst_pad_is_linked(sinkPad))
        {
            GstPadLinkReturn ret;

            ret = gst_pad_link(
                pad,
                sinkPad);


            if(ret == GST_PAD_LINK_OK)
                printf("PAD LINK OK\n");
            else
                printf("PAD LINK ERROR\n");
        }


        gst_object_unref(sinkPad);
        gst_object_unref(target);
    }


    gst_caps_unref(caps);
}

static GstFlowReturn OnNewSample(
    GstElement *sink,
    gpointer data)
{

    GstSample *sample = NULL;

    g_signal_emit_by_name(
        sink,
        "pull-sample",
        &sample);

    if(!sample)
    {
        printf("NO SAMPLE\n");
        return GST_FLOW_ERROR;
    }


        if(!textureReady)
    {
        GetVideoInfo(sample);
    }


    GstBuffer *buffer =
        gst_sample_get_buffer(sample);

    if(!buffer)
    {
        printf("NO BUFFER\n");
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }



    GstMapInfo map;


    if(gst_buffer_map(
        buffer,
        &map,
        GST_MAP_READ))
    {
        if(textureBuffer)
        {
            memcpy(
                textureBuffer,
                map.data,
                videoWidth *
                videoHeight *
                4);
        }


        gst_buffer_unmap(
            buffer,
            &map);
    }


    gst_sample_unref(sample);


    return GST_FLOW_OK;
}

void PlayerUpdate(void)
{
    if(textureBuffer && !textureReady)
    {
        Image img = {0};

        img.data = textureBuffer;
        img.width = videoWidth;
        img.height = videoHeight;
        img.mipmaps = 1;
        img.format =
            PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;


        videoTexture =
            LoadTextureFromImage(img);


        textureReady = true;

        printf("RAYLIB TEXTURE CREATED\n");
    }


    if(textureReady)
    {
        UpdateTexture(
            videoTexture,
            textureBuffer);
    }
}

bool PlayerFinished(void)
{
    return finished;
}

void PlayerDraw(void)
{
    if(!textureReady)
        return;


    Rectangle src =
    {
        0,
        0,
        videoWidth,
        -videoHeight
    };


    Rectangle dst =
    {
        0,
        0,
        GetScreenWidth(),
        GetScreenHeight()
    };


    DrawTexturePro(
        videoTexture,
        src,
        dst,
        (Vector2){0,0},
        0,
        WHITE);
}

void PlayerStop(void)
{
    playing = false;


    if(pipeline)
    {
        gst_element_set_state(
            pipeline,
            GST_STATE_NULL);
    }


    if(playerBus)
    {
        gst_object_unref(playerBus);
        playerBus = NULL;
    }


    if(pipeline)
    {
        gst_object_unref(pipeline);
        pipeline = NULL;
    }


    if(textureReady)
    {
        UnloadTexture(videoTexture);
        textureReady = false;
    }


    free(textureBuffer);

    textureBuffer = NULL;

    videoWidth = 0;
    videoHeight = 0;
}

bool PlayerPlay(const char *filename)
{
    PlayerStop();

    finished = FALSE;


    pipeline = gst_pipeline_new("raylib-player");


    GstElement *source =
        gst_element_factory_make("filesrc","source");

    GstElement *decode =
        gst_element_factory_make("decodebin","decode");


    GstElement *convert =
    gst_element_factory_make(
        "videoconvert",
        "convert");

        GstElement *capsfilter =
    gst_element_factory_make(
        "capsfilter",
        "capsfilter");


    videoSink =
        gst_element_factory_make(
            "appsink",
            "videosink");


    GstElement *audioConvert =
        gst_element_factory_make(
            "audioconvert",
            "audioConvert");


    GstElement *audioSink =
        gst_element_factory_make(
            "autoaudiosink",
            "audioSink");



            GstCaps *caps =
    gst_caps_new_simple(
        "video/x-raw",
        "format",
        G_TYPE_STRING,
        "RGBA",
        NULL);


g_object_set(
    capsfilter,
    "caps",
    caps,
    NULL);


gst_caps_unref(caps);


    if(!pipeline ||
       !source ||
       !decode ||
       !convert ||
       !videoSink ||
       !audioConvert ||
       !audioSink)
    {
        return false;
    }

    g_object_set(
        source,
        "location",
        filename,
        NULL);



    g_object_set(
        videoSink,
        "emit-signals",
        TRUE,
        "sync",
        TRUE,
        NULL);



    gst_bin_add_many(
    GST_BIN(pipeline),
    source,
    decode,
    convert,
    capsfilter,
    videoSink,
    audioConvert,
    audioSink,
    NULL);



    gst_element_link(
        source,
        decode);


    gst_element_link_many(
    convert,
    capsfilter,
    videoSink,
    NULL);



    gst_element_link_many(
        audioConvert,
        audioSink,
        NULL);

    g_signal_connect(
    decode,
    "pad-added",
    G_CALLBACK(OnPadAdded),
    convert);

    g_signal_connect(
    videoSink,
    "new-sample",
    G_CALLBACK(OnNewSample),
    NULL);



    gst_element_set_state(
        pipeline,
        GST_STATE_PLAYING);

    playerBus = gst_element_get_bus(pipeline);

    playing = true;
    finished = false;


    playing = TRUE;

    printf("player 1\n");


    return true;
}

void PlayerShutdown(void)
{
    PlayerStop();

    gst_deinit();
}