#include "Game/Sys/PlatStream.h"
#include "Game/Sys/GCStream.h"
#include "NL/nlMemory.h"
#include "NL/nlSortedSlot.h"

extern "C"
{
    void sndStreamMixParameterEx(unsigned long stid, unsigned char vol, unsigned char pan, unsigned char span, unsigned char auxa, unsigned char auxb);
    void sndStreamDeactivate(unsigned long stid);
}

GCAudioStreaming::AudioBufferMgr g_BufferMgr;

nlStaticSortedSlot<GCAudioStreaming::AudioStream*, 7> g_Streams;
static bool g_StreamingInitd;

/**
 * Offset/Address/Size: 0x574 | 0x801C7638 | size: 0x44
 */
void PlatAudio::InitStreaming()
{
    if (!g_StreamingInitd)
    {
        g_BufferMgr.Init(0x5DD80);
        g_StreamingInitd = true;
    }
}

static inline void StopAllStreamObjs()
{
    using namespace GCAudioStreaming;
    unsigned long stream = 0;

    while (stream < g_Streams.m_EntryCount)
    {
        (*g_Streams.m_pEntryLookup[stream].pEntry)->Stop();
        stream++;
    }
}

static inline void PurgeStoppedStreams(unsigned char Block)
{
    using namespace GCAudioStreaming;
    unsigned long stream = 0;

    while (stream < g_Streams.m_EntryCount)
    {
        AudioStream** pStream = g_Streams.m_pEntryLookup[stream].pEntry;
        AudioStream* obj = *pStream;
        if (Block)
        {
            obj->SafeToPurge();
        }
        pStream = g_Streams.m_pEntryLookup[stream].pEntry;
        delete *pStream;
        if (pStream != NULL)
        {
            g_Streams.DeleteEntry(pStream);
        }
        stream++;
    }
}

/**
 * Offset/Address/Size: 0x208 | 0x801C72CC | size: 0x36C
 */
void PlatAudio::ShutdownStreaming()
{
    StopAllStreamObjs();
    PurgeStoppedStreams(true);

    g_Streams.Clear();

    nlFree(g_BufferMgr.m_MRAMBuffer);
    g_BufferMgr.m_MRAMBuffer = NULL;
    g_BufferMgr.m_PoolSize = 0;
    g_StreamingInitd = false;
}

/**
 * Offset/Address/Size: 0x1C4 | 0x801C7288 | size: 0x44
 */
void PlatAudio::ConfigureStreamBuffers(unsigned long count)
{
    g_BufferMgr.DeleteBuffers();
    g_BufferMgr.CreateBuffers(count);
}

/**
 * Offset/Address/Size: 0x1BC | 0x801C7280 | size: 0x8
 */
bool PlatAudio::IsStreamingInited()
{
    return g_StreamingInitd;
}

/**
 * Offset/Address/Size: 0x0 | 0x801C70C4 | size: 0x1BC
 */
void PlatAudio::StopAllStreams()
{
    for (unsigned long stream = 0; stream < g_Streams.m_EntryCount; stream++)
    {
        (*g_Streams.m_pEntryLookup[stream].pEntry)->Stop();
    }
}
