#include "Game/Sys/THPSimple.h"
#include "NL/gl/glState.h"
#include "NL/glx/glxTexture.h"
#include "NL/nlFileGC.h"
#include "dolphin/ai.h"
#include "dolphin/os.h"
#include "dolphin/os/OSCache.h"
#include "dolphin/thp/THPAudio.h"
#include "dolphin/thp/THPInfo.h"
#include "dolphin/thp/THPPlayer.h"
#include "dolphin/thp/THPVideoDecode.h"

static void THPAudioMixCallback();

extern "C" void* memcpy(void*, const void*, unsigned long);
extern "C" void* memset(void*, int, unsigned long);
extern "C" int strcmp(const char*, const char*);

static THPSimpleControl SimpleControl;
static s32 NumReadBuffers;
static int NumAudioBuffers;
static int Initialized;
static s32 SoundBufferIndex;
static void (*OldAIDCallback)();
static s16* LastAudioBuffer;
static s16* CurAudioBuffer;
static s32 AudioSystem;
static long WorkBuffer[16] ATTRIBUTE_ALIGN(32);
static s16 SoundBuffer[2][320] ATTRIBUTE_ALIGN(32);

static unsigned short VolumeTable[128] = {
    0x0000,
    0x0002,
    0x0008,
    0x0012,
    0x0020,
    0x0032,
    0x0049,
    0x0063,
    0x0082,
    0x00A4,
    0x00CB,
    0x00F5,
    0x0124,
    0x0157,
    0x018E,
    0x01C9,
    0x0208,
    0x024B,
    0x0292,
    0x02DD,
    0x032C,
    0x037F,
    0x03D7,
    0x0432,
    0x0492,
    0x04F5,
    0x055D,
    0x05C9,
    0x0638,
    0x06AC,
    0x0724,
    0x07A0,
    0x0820,
    0x08A4,
    0x092C,
    0x09B8,
    0x0A48,
    0x0ADD,
    0x0B75,
    0x0C12,
    0x0CB2,
    0x0D57,
    0x0DFF,
    0x0EAC,
    0x0F5D,
    0x1012,
    0x10CA,
    0x1187,
    0x1248,
    0x130D,
    0x13D7,
    0x14A4,
    0x1575,
    0x164A,
    0x1724,
    0x1801,
    0x18E3,
    0x19C8,
    0x1AB2,
    0x1BA0,
    0x1C91,
    0x1D87,
    0x1E81,
    0x1F7F,
    0x2081,
    0x2187,
    0x2291,
    0x239F,
    0x24B2,
    0x25C8,
    0x26E2,
    0x2801,
    0x2923,
    0x2A4A,
    0x2B75,
    0x2CA3,
    0x2DD6,
    0x2F0D,
    0x3048,
    0x3187,
    0x32CA,
    0x3411,
    0x355C,
    0x36AB,
    0x37FF,
    0x3956,
    0x3AB1,
    0x3C11,
    0x3D74,
    0x3EDC,
    0x4048,
    0x41B7,
    0x432B,
    0x44A3,
    0x461F,
    0x479F,
    0x4923,
    0x4AAB,
    0x4C37,
    0x4DC7,
    0x4F5C,
    0x50F4,
    0x5290,
    0x5431,
    0x55D6,
    0x577E,
    0x592B,
    0x5ADC,
    0x5C90,
    0x5E49,
    0x6006,
    0x61C7,
    0x638C,
    0x6555,
    0x6722,
    0x68F4,
    0x6AC9,
    0x6CA2,
    0x6E80,
    0x7061,
    0x7247,
    0x7430,
    0x761E,
    0x7810,
    0x7A06,
    0x7C00,
    0x7DFE,
    0x8000,
};

/**
 * Offset/Address/Size: 0x17E4 | 0x801CD748 | size: 0x4
 */
static void __THPAsyncCancelCB(nlFile*, void*, unsigned int, unsigned long, void (*)(nlFile*, void*, unsigned int, unsigned long))
{
}

/**
 * Offset/Address/Size: 0x16C8 | 0x801CD62C | size: 0x11C
 */
extern "C" int THPSimpleInit(long audioSystem)
{
    memset(&SimpleControl, 0, sizeof(SimpleControl));
    LCEnable();

    if (!THPInit())
    {
        return 0;
    }

    AudioSystem = audioSystem;
    SoundBufferIndex = 0;
    LastAudioBuffer = NULL;
    CurAudioBuffer = NULL;

    if (audioSystem != 1)
    {
        int old = OSDisableInterrupts();
        OldAIDCallback = AIRegisterDMACallback(THPAudioMixCallback);

        if (OldAIDCallback == NULL && AudioSystem != 0)
        {
            AIRegisterDMACallback(NULL);
            OSRestoreInterrupts(old);
            return 0;
        }

        OSRestoreInterrupts(old);

        if (AudioSystem == 0)
        {
            memset(SoundBuffer, 0, sizeof(SoundBuffer));
            DCFlushRange(SoundBuffer, sizeof(SoundBuffer));
            AIInitDMA((u32)SoundBuffer[SoundBufferIndex], 0x280);
            AIStartDMA();
        }
    }

    Initialized = 1;
    return 1;
}

/**
 * Offset/Address/Size: 0x1664 | 0x801CD5C8 | size: 0x64
 */
extern "C" void THPSimpleQuit()
{
    LCDisable();
    if (AudioSystem != 1 && OldAIDCallback != NULL)
    {
        int old = OSDisableInterrupts();
        AIRegisterDMACallback(OldAIDCallback);
        OSRestoreInterrupts(old);
    }
    Initialized = 0;
}

/**
 * Offset/Address/Size: 0x1364 | 0x801CD2C8 | size: 0x300
 */
extern "C" int THPSimpleOpen(const char* fileName)
{
    long offset;
    long i;

    if (!Initialized)
    {
        return 0;
    }

    if (SimpleControl.open)
    {
        return 0;
    }

    memset(&SimpleControl.videoInfo, 0, sizeof(THPVideoInfo));
    memset(&SimpleControl.audioInfo, 0, sizeof(THPAudioInfo));

    SimpleControl.fileInfo = nlOpen(fileName);
    if (!SimpleControl.fileInfo)
    {
        return 0;
    }

    nlRead(SimpleControl.fileInfo, WorkBuffer, sizeof(WorkBuffer));
    memcpy(SimpleControl.magic, WorkBuffer, sizeof(THPHeader));

    if (strcmp(SimpleControl.magic, "THP") != 0)
    {
        nlClose(SimpleControl.fileInfo);
        SimpleControl.fileInfo = NULL;
        return 0;
    }

    if (SimpleControl.version != 0x00011000)
    {
        nlClose(SimpleControl.fileInfo);
        SimpleControl.fileInfo = NULL;
        return 0;
    }

    offset = SimpleControl.compInfoDataOffsets;
    nlSeek(SimpleControl.fileInfo, offset, 0);
    nlRead(SimpleControl.fileInfo, WorkBuffer, 0x20);
    memcpy(&SimpleControl.compInfo, WorkBuffer, sizeof(THPFrameCompInfo));

    offset += sizeof(THPFrameCompInfo);
    SimpleControl.audioExist = 0;

    for (i = 0; i < SimpleControl.compInfo.mNumComponents; i++)
    {
        switch (SimpleControl.compInfo.mFrameComp[i])
        {
        case 0:
            nlSeek(SimpleControl.fileInfo, offset, 0);
            nlRead(SimpleControl.fileInfo, WorkBuffer, 0x20);
            memcpy(&SimpleControl.videoInfo, WorkBuffer, sizeof(THPVideoInfo));
            offset += sizeof(THPVideoInfo);
            break;
        case 1:
            nlSeek(SimpleControl.fileInfo, offset, 0);
            nlRead(SimpleControl.fileInfo, WorkBuffer, 0x20);
            memcpy(&SimpleControl.audioInfo, WorkBuffer, sizeof(THPAudioInfo));
            offset += sizeof(THPAudioInfo);
            SimpleControl.audioExist = 1;
            break;
        default:
            return 0;
        }
    }

    SimpleControl.curOffset = SimpleControl.movieDataOffsets;
    SimpleControl.readSize = SimpleControl.firstFrameSize;
    SimpleControl.readIndex = 0;
    SimpleControl.totalReadFrame = 0;
    SimpleControl.dvdError = 0;
    SimpleControl.textureSet.mFrameNumber = -1;
    SimpleControl.nextDecodeIndex = 0;
    SimpleControl.audioDecodeIndex = 0;
    SimpleControl.audioOutputIndex = 0;
    SimpleControl.preFetchState = 0;
    SimpleControl.audioState = 0;
    SimpleControl.loop = 0;
    SimpleControl.open = 1;
    SimpleControl.curVolume = 127.0f;
    SimpleControl.targetVolume = 127.0f;
    SimpleControl.rampCount = 0;

    return 1;
}

/**
 * Offset/Address/Size: 0x1298 | 0x801CD1FC | size: 0xCC
 */
extern "C" int THPSimpleClose()
{
    THPSimpleControl* ctrl = &SimpleControl;

    if (ctrl->open && ctrl->preFetchState == 0)
    {
        if (ctrl->audioExist)
        {
            if (ctrl->audioState == 1)
            {
                return 0;
            }
        }
        else
        {
            ctrl->audioState = 0;
        }

        THPSimpleControl* sc = &SimpleControl;

        if (sc->readProgress == 0)
        {
            ctrl->open = 0;

            while (nlAsyncReadsPending(sc->fileInfo))
            {
                nlServiceFileSystem();
            }

            nlClose(SimpleControl.fileInfo);

            SimpleControl.fileInfo = NULL;

            return 1;
        }
    }

    return 0;
}

/**
 * Offset/Address/Size: 0x1238 | 0x801CD19C | size: 0x60
 */
extern "C" unsigned long THPSimpleCalcNeedMemory(int numReadBuffers, int numAudioBuffers)
{
    unsigned long size;

    NumReadBuffers = numReadBuffers;

    THPSimpleControl* ctrl = &SimpleControl;

    NumAudioBuffers = numAudioBuffers;

    if (ctrl->open)
    {
        size = ((ctrl->bufSize + 31) & ~31) * numReadBuffers;

        if (ctrl->audioExist)
        {
            size += numAudioBuffers * ((ctrl->audioMaxSamples * 4 + 31) & ~31);
        }

        return size + 0x1000;
    }

    return 0;
}

/**
 * Offset/Address/Size: 0xE6C | 0x801CCDD0 | size: 0x3CC
 */
extern "C" int THPSimpleSetBuffer(unsigned char* buffer)
{
    unsigned long i;
    unsigned char* ptr;
    unsigned long numRead;
    unsigned long numAudio;
    PlatTexture* tex;

    if (SimpleControl.open && SimpleControl.preFetchState == 0)
    {
        if (SimpleControl.audioState == 1)
        {
            return 0;
        }

        ptr = buffer;

        SimpleControl.textureSet.mYTexture = (u8*)glx_GetTex(glGetTexture("movie"), 1, 1)->m_SwizzledData;
        SimpleControl.textureSet.mUTexture = (u8*)glx_GetTex(glGetTexture("movie_u"), 1, 1)->m_SwizzledData;
        tex = glx_GetTex(glGetTexture("movie_v"), 1, 1);
        numRead = NumReadBuffers;
        SimpleControl.textureSet.mVTexture = (u8*)tex->m_SwizzledData;

        for (i = 0; i < (unsigned long)NumReadBuffers; i++)
        {
            SimpleControl.readBuffer[i].mPtr = ptr;
            ptr += (SimpleControl.bufSize + 31) & ~31;
            SimpleControl.readBuffer[i].mIsValid = 0;
        }

        if (SimpleControl.audioExist)
        {
            numAudio = NumAudioBuffers;
            for (i = 0; i < (unsigned long)NumAudioBuffers; i++)
            {
                SimpleControl.audioBuffer[i].mBuffer = (s16*)ptr;
                SimpleControl.audioBuffer[i].mCurPtr = (s16*)ptr;
                SimpleControl.audioBuffer[i].mValidSample = 0;
                ptr += (SimpleControl.audioMaxSamples * 4 + 31) & ~31;
            }
        }

        SimpleControl.thpWork = ptr;
    }

    return 1;
}

static inline void update_read_idx()
{
    s32 readIndex = SimpleControl.readIndex;
    if (readIndex + 1 >= NumReadBuffers)
        readIndex = 0;
    else
        readIndex = readIndex + 1;
    SimpleControl.readIndex = readIndex;
}

/**
 * Offset/Address/Size: 0xD0C | 0x801CCC70 | size: 0x160
 */
static void __THPSimpleDVDCallback(nlFile* file, void* buffer, unsigned int bytesRead, unsigned long offset)
{
    SimpleControl.readProgress = 0;

    SimpleControl.readBuffer[SimpleControl.readIndex].mFrameNumber = SimpleControl.totalReadFrame;
    SimpleControl.totalReadFrame++;
    SimpleControl.readBuffer[SimpleControl.readIndex].mIsValid = TRUE;

    SimpleControl.curOffset += SimpleControl.readSize;
    SimpleControl.readSize = *(u32*)SimpleControl.readBuffer[SimpleControl.readIndex].mPtr;

    update_read_idx();

    if (SimpleControl.readBuffer[SimpleControl.readIndex].mIsValid != 0)
    {
        return;
    }

    if (SimpleControl.dvdError != 0)
    {
        return;
    }

    if (SimpleControl.preFetchState != 1)
    {
        return;
    }

    if (SimpleControl.totalReadFrame > SimpleControl.numFrames - 1)
    {
        if (SimpleControl.loop != 1)
        {
            return;
        }
        SimpleControl.totalReadFrame = 0;
        SimpleControl.curOffset = SimpleControl.movieDataOffsets;
        SimpleControl.readSize = SimpleControl.firstFrameSize;
    }

    SimpleControl.readProgress = 1;
    nlSeek(SimpleControl.fileInfo, SimpleControl.curOffset, 0);
    nlReadAsync(SimpleControl.fileInfo, SimpleControl.readBuffer[SimpleControl.readIndex].mPtr, SimpleControl.readSize, __THPSimpleDVDCallback, 0);
}

/**
 * Offset/Address/Size: 0xB9C | 0x801CCB00 | size: 0x170
 */
extern "C" int THPSimplePreLoad(long loop)
{
    unsigned long i;
    unsigned long readNum;

    if (SimpleControl.open && SimpleControl.preFetchState == 0)
    {
        readNum = NumReadBuffers;
        if (loop == 0 && SimpleControl.numFrames < (unsigned long)NumReadBuffers)
        {
            readNum = SimpleControl.numFrames;
        }

        for (i = 0; i < readNum; i++)
        {
            nlSeek(SimpleControl.fileInfo, SimpleControl.curOffset, 0);
            nlRead(SimpleControl.fileInfo, SimpleControl.readBuffer[SimpleControl.readIndex].mPtr, SimpleControl.readSize);

            long idx = SimpleControl.readIndex;
            SimpleControl.curOffset += SimpleControl.readSize;
            SimpleControl.readSize = *(long*)SimpleControl.readBuffer[idx].mPtr;
            SimpleControl.readBuffer[idx].mIsValid = 1;
            SimpleControl.readBuffer[SimpleControl.readIndex].mFrameNumber = SimpleControl.totalReadFrame;

            SimpleControl.readIndex = (SimpleControl.readIndex + 1 >= NumReadBuffers) ? 0 : SimpleControl.readIndex + 1;
            SimpleControl.totalReadFrame++;

            if ((unsigned long)SimpleControl.totalReadFrame > SimpleControl.numFrames - 1)
            {
                if (SimpleControl.loop == 1)
                {
                    SimpleControl.totalReadFrame = 0;
                    SimpleControl.curOffset = SimpleControl.movieDataOffsets;
                    SimpleControl.readSize = SimpleControl.firstFrameSize;
                }
            }
        }

        SimpleControl.loop = loop;
        SimpleControl.preFetchState = 1;
        return 1;
    }

    return 0;
}

/**
 * Offset/Address/Size: 0xB88 | 0x801CCAEC | size: 0x14
 */
extern "C" void THPSimpleAudioStart()
{
    SimpleControl.audioState = 1;
}

/**
 * Offset/Address/Size: 0xB74 | 0x801CCAD8 | size: 0x14
 */
extern "C" void THPSimpleAudioStop()
{
    SimpleControl.audioState = 0;
}

/**
 * Offset/Address/Size: 0xA14 | 0x801CC978 | size: 0x160
 */
extern "C" int THPSimpleLoadStop()
{
    long i;

    if (SimpleControl.open && SimpleControl.audioState == 0)
    {
        SimpleControl.preFetchState = 0;

        if (SimpleControl.readProgress != 0)
        {
            nlCancelPendingAsyncReads(SimpleControl.fileInfo, __THPAsyncCancelCB);

            while (nlAsyncReadsPending(SimpleControl.fileInfo))
            {
                nlServiceFileSystem();
                OSYieldThread();
            }

            SimpleControl.readProgress = 0;
        }

        for (i = 0; i < 16; i++)
        {
            SimpleControl.readBuffer[i].mIsValid = 0;
        }

        SimpleControl.audioBuffer[0].mValidSample = 0;
        SimpleControl.audioBuffer[1].mValidSample = 0;
        SimpleControl.audioBuffer[2].mValidSample = 0;
        SimpleControl.audioBuffer[3].mValidSample = 0;
        SimpleControl.audioBuffer[4].mValidSample = 0;
        SimpleControl.audioBuffer[5].mValidSample = 0;
        SimpleControl.textureSet.mFrameNumber = -1;
        SimpleControl.curOffset = SimpleControl.movieDataOffsets;
        SimpleControl.readSize = SimpleControl.firstFrameSize;
        SimpleControl.readIndex = 0;
        SimpleControl.totalReadFrame = 0;
        SimpleControl.dvdError = 0;
        SimpleControl.nextDecodeIndex = 0;
        SimpleControl.audioDecodeIndex = 0;
        SimpleControl.audioOutputIndex = 0;
        SimpleControl.curVolume = SimpleControl.targetVolume;
        SimpleControl.rampCount = 0;

        return 1;
    }

    return 0;
}

static inline int VideoDecode(unsigned char* videoFrame)
{
    long ret = THPVideoDecode(videoFrame,
        SimpleControl.textureSet.mYTexture,
        SimpleControl.textureSet.mUTexture,
        SimpleControl.textureSet.mVTexture,
        SimpleControl.thpWork);
    if (ret == 0)
    {
        SimpleControl.textureSet.mFrameNumber = SimpleControl.readBuffer[SimpleControl.nextDecodeIndex].mFrameNumber;
        return 1;
    }
    return 0;
}

/**
 * Offset/Address/Size: 0x684 | 0x801CC5E8 | size: 0x390
 */
extern "C" long THPSimpleDecode(long audioTrack)
{
    int* validBuffer;
    THPReadBuffer* readBuffer;
    int old;
    unsigned long i;
    unsigned char* ptr;
    unsigned long* compSizePtr;
    unsigned long sample;

    do
    {
        validBuffer = &SimpleControl.readBuffer[0].mIsValid;

        if (validBuffer[SimpleControl.nextDecodeIndex * 3] == 0)
        {
            break;
        }

        readBuffer = SimpleControl.readBuffer;
        compSizePtr = (unsigned long*)(readBuffer[SimpleControl.nextDecodeIndex].mPtr + 8);
        ptr = readBuffer[SimpleControl.nextDecodeIndex].mPtr + SimpleControl.compInfo.mNumComponents * 4 + 8;

        if (SimpleControl.audioExist != 0 && AudioSystem != 1)
        {
            if (audioTrack < 0 || (unsigned long)audioTrack >= SimpleControl.audioInfo.mSndNumTracks)
            {
                return 4;
            }

            if (SimpleControl.audioBuffer[SimpleControl.audioDecodeIndex].mValidSample == 0)
            {
                for (i = 0; i < SimpleControl.compInfo.mNumComponents; i++)
                {
                    switch (SimpleControl.compInfo.mFrameComp[i])
                    {
                    case 0:
                        if (!VideoDecode(ptr))
                        {
                            return 1;
                        }
                        break;
                    case 1:
                        sample = THPAudioDecode(
                            SimpleControl.audioBuffer[SimpleControl.audioDecodeIndex].mBuffer,
                            ptr + *compSizePtr * audioTrack,
                            0);
                        old = OSDisableInterrupts();
                        SimpleControl.audioBuffer[SimpleControl.audioDecodeIndex].mValidSample = sample;
                        SimpleControl.audioBuffer[SimpleControl.audioDecodeIndex].mCurPtr = SimpleControl.audioBuffer[SimpleControl.audioDecodeIndex].mBuffer;
                        OSRestoreInterrupts(old);
                        if (++SimpleControl.audioDecodeIndex >= NumAudioBuffers)
                        {
                            SimpleControl.audioDecodeIndex = 0;
                        }
                        break;
                    }
                    ptr += *compSizePtr;
                    compSizePtr++;
                }
            }
            else
            {
                return 3;
            }
        }
        else
        {
            for (i = 0; i < SimpleControl.compInfo.mNumComponents; i++)
            {
                switch (SimpleControl.compInfo.mFrameComp[i])
                {
                case 0:
                    if (!VideoDecode(ptr))
                    {
                        return 1;
                    }
                    break;
                }
                ptr += *compSizePtr;
                compSizePtr++;
            }
        }

        validBuffer[SimpleControl.nextDecodeIndex * 3] = 0;
        SimpleControl.nextDecodeIndex = (SimpleControl.nextDecodeIndex + 1 >= NumReadBuffers) ? 0 : SimpleControl.nextDecodeIndex + 1;

        old = OSDisableInterrupts();

        do
        {
            if (validBuffer[SimpleControl.readIndex * 3] == 0 && SimpleControl.readProgress == 0 && SimpleControl.dvdError == 0 && SimpleControl.preFetchState == 1)
            {
                if ((unsigned long)SimpleControl.totalReadFrame > SimpleControl.numFrames - 1)
                {
                    if (SimpleControl.loop != 1)
                    {
                        break;
                    }
                    SimpleControl.totalReadFrame = 0;
                    SimpleControl.curOffset = SimpleControl.movieDataOffsets;
                    SimpleControl.readSize = SimpleControl.firstFrameSize;
                }

                SimpleControl.readProgress = 1;
                nlSeek(SimpleControl.fileInfo, SimpleControl.curOffset, 0);
                nlReadAsync(SimpleControl.fileInfo,
                    readBuffer[SimpleControl.readIndex].mPtr,
                    SimpleControl.readSize,
                    __THPSimpleDVDCallback,
                    0);
            }
        } while (false);

        OSRestoreInterrupts(old);
        return 0;
    } while (false);

    return 2;
}

/**
 * Offset/Address/Size: 0x31C | 0x801CC280 | size: 0x368
 */
static void MixAudio(short* destination, short* source, unsigned long sample)
{
    unsigned long requestSample;
    unsigned long i;
    unsigned short vol;
    long mix;
    short* dst;
    short* libsrc;
    short* thpsrc;

    if (AudioSystem == 1)
    {
        return;
    }

    if (source != NULL)
    {
        if ((SimpleControl.open != 0) && (SimpleControl.audioState == 1) && (SimpleControl.audioExist != 0))
        {
            while (1)
            {
                if (SimpleControl.audioBuffer[SimpleControl.audioOutputIndex].mValidSample == 0)
                {
                    break;
                }

                if (SimpleControl.audioBuffer[SimpleControl.audioOutputIndex].mValidSample >= sample)
                {
                    requestSample = sample;
                }
                else
                {
                    requestSample = SimpleControl.audioBuffer[SimpleControl.audioOutputIndex].mValidSample;
                }

                thpsrc = SimpleControl.audioBuffer[SimpleControl.audioOutputIndex].mCurPtr;
                dst = destination;
                libsrc = source;

                for (i = 0; i < requestSample; i++)
                {
                    if (SimpleControl.rampCount != 0)
                    {
                        SimpleControl.rampCount--;
                        SimpleControl.curVolume += SimpleControl.deltaVolume;
                    }
                    else
                    {
                        SimpleControl.curVolume = SimpleControl.targetVolume;
                    }

                    vol = VolumeTable[(long)SimpleControl.curVolume];

                    mix = libsrc[0] + ((vol * thpsrc[0]) >> 15);
                    if (mix < -0x8000)
                    {
                        mix = -0x8000;
                    }
                    if (mix > 0x7FFF)
                    {
                        mix = 0x7FFF;
                    }
                    dst[0] = mix;

                    mix = libsrc[1] + ((vol * thpsrc[1]) >> 15);
                    if (mix < -0x8000)
                    {
                        mix = -0x8000;
                    }
                    if (mix > 0x7FFF)
                    {
                        mix = 0x7FFF;
                    }
                    dst[1] = mix;

                    dst += 2;
                    libsrc += 2;
                    thpsrc += 2;
                }

                sample -= requestSample;

                SimpleControl.audioBuffer[SimpleControl.audioOutputIndex].mValidSample -= requestSample;
                SimpleControl.audioBuffer[SimpleControl.audioOutputIndex].mCurPtr = thpsrc;

                if ((SimpleControl.audioBuffer[SimpleControl.audioOutputIndex].mValidSample == 0) && (++SimpleControl.audioOutputIndex >= NumAudioBuffers))
                {
                    SimpleControl.audioOutputIndex = 0;
                }

                if (sample == 0)
                {
                    return;
                }

                destination = dst;
                source = libsrc;
            }

            memcpy(destination, source, sample << 2);
        }
        else
        {
            memcpy(destination, source, sample << 2);
        }
    }
    else
    {
        if ((SimpleControl.open != 0) && (SimpleControl.audioState == 1) && (SimpleControl.audioExist != 0))
        {
            while (1)
            {
                requestSample = SimpleControl.audioBuffer[SimpleControl.audioOutputIndex].mValidSample;
                if (requestSample == 0)
                {
                    break;
                }

                if (requestSample >= sample)
                {
                    requestSample = sample;
                }

                thpsrc = SimpleControl.audioBuffer[SimpleControl.audioOutputIndex].mCurPtr;
                dst = destination;

                for (i = 0; i < requestSample; i++)
                {
                    if (SimpleControl.rampCount != 0)
                    {
                        SimpleControl.rampCount--;
                        SimpleControl.curVolume += SimpleControl.deltaVolume;
                    }
                    else
                    {
                        SimpleControl.curVolume = SimpleControl.targetVolume;
                    }

                    vol = VolumeTable[(long)SimpleControl.curVolume];

                    mix = (vol * thpsrc[0]) >> 15;
                    if (mix < -0x8000)
                    {
                        mix = -0x8000;
                    }
                    if (mix > 0x7FFF)
                    {
                        mix = 0x7FFF;
                    }
                    dst[0] = mix;

                    mix = (vol * thpsrc[1]) >> 15;
                    if (mix < -0x8000)
                    {
                        mix = -0x8000;
                    }
                    if (mix > 0x7FFF)
                    {
                        mix = 0x7FFF;
                    }
                    dst[1] = mix;

                    dst += 2;
                    thpsrc += 2;
                }

                sample -= requestSample;

                SimpleControl.audioBuffer[SimpleControl.audioOutputIndex].mValidSample -= requestSample;
                SimpleControl.audioBuffer[SimpleControl.audioOutputIndex].mCurPtr = thpsrc;

                if ((SimpleControl.audioBuffer[SimpleControl.audioOutputIndex].mValidSample == 0) && (++SimpleControl.audioOutputIndex >= NumAudioBuffers))
                {
                    SimpleControl.audioOutputIndex = 0;
                }

                if (sample == 0)
                {
                    return;
                }

                destination = dst;
            }

            memset(destination, 0, sample << 2);
        }
        else
        {
            memset(destination, 0, sample << 2);
        }
    }
}

/**
 * Offset/Address/Size: 0x2D4 | 0x801CC238 | size: 0x48
 */
extern "C" int THPSimpleGetVideoInfo(THPVideoInfo* videoInfo)
{
    if (SimpleControl.open)
    {
        memcpy(videoInfo, &SimpleControl.videoInfo, sizeof(THPVideoInfo));
        return 1;
    }
    return 0;
}

/**
 * Offset/Address/Size: 0x2B0 | 0x801CC214 | size: 0x24
 */
extern "C" s32 THPSimpleGetTotalFrame()
{
    if (SimpleControl.open)
        return SimpleControl.numFrames;
    return 0;
}

/**
 * Offset/Address/Size: 0x138 | 0x801CC09C | size: 0x178
 */
static void THPAudioMixCallback()
{
    if (AudioSystem == 0)
    {
        SoundBufferIndex ^= 1;
        AIInitDMA((u32)SoundBuffer[SoundBufferIndex], 0x280);
        BOOL old = OSEnableInterrupts();
        MixAudio(SoundBuffer[SoundBufferIndex], NULL, 0xA0);
        DCFlushRange(SoundBuffer[SoundBufferIndex], 0x280);
        OSRestoreInterrupts(old);
    }
    else
    {
        if (AudioSystem == 2)
        {
            if (LastAudioBuffer != NULL)
            {
                CurAudioBuffer = LastAudioBuffer;
            }
            OldAIDCallback();
            LastAudioBuffer = (s16*)((u32)AIGetDMAStartAddr() + 0x80000000);
        }
        else
        {
            OldAIDCallback();
            CurAudioBuffer = (s16*)((u32)AIGetDMAStartAddr() + 0x80000000);
        }

        SoundBufferIndex ^= 1;
        AIInitDMA((u32)SoundBuffer[SoundBufferIndex], 0x280);
        BOOL old = OSEnableInterrupts();

        if (CurAudioBuffer != NULL)
        {
            DCInvalidateRange(CurAudioBuffer, 0x280);
        }

        MixAudio(SoundBuffer[SoundBufferIndex], CurAudioBuffer, 0xA0);
        DCFlushRange(SoundBuffer[SoundBufferIndex], 0x280);
        OSRestoreInterrupts(old);
    }
}

/**
 * Offset/Address/Size: 0x10 | 0x801CBF74 | size: 0x128
 */
extern "C" int THPSimpleSetVolume(long vol, long time)
{
    THPSimpleControl* ctrl = &SimpleControl;

    if (ctrl->open && ctrl->audioExist)
    {
        u32 rate = AIGetDSPSampleRate();
        long samplePerMs = 0x30;
        if (!rate)
            samplePerMs = 0x20;

        if (vol > 127)
            vol = 127;
        if (vol < 0)
            vol = 0;
        if (time > 60000)
            time = 60000;
        if (time < 0)
            time = 0;

        int old = OSDisableInterrupts();
        ctrl = &SimpleControl;

        ctrl->targetVolume = (float)vol;

        if (time != 0)
        {
            ctrl->rampCount = samplePerMs * time;
            ctrl->deltaVolume = (ctrl->targetVolume - ctrl->curVolume) / (float)ctrl->rampCount;
        }
        else
        {
            ctrl->curVolume = ctrl->targetVolume;
            ctrl->rampCount = 0;
        }

        OSRestoreInterrupts(old);
        return 1;
    }
    return 0;
}

/**
 * Offset/Address/Size: 0x0 | 0x801CBF64 | size: 0x10
 */
extern "C" s32 THPSimpleGetCurrentFrame()
{
    return SimpleControl.textureSet.mFrameNumber;
}
