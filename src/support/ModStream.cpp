#include <climits>
#include <cstdio>
#include "ModStream.h"

using namespace cmixer;

ModStream::ModStream(std::vector<char> &&rawModuleData)
        : Source(44100, INT_MAX)
        , moduleFile(rawModuleData)
        , replayBuffer(2048*8)
        , rbOffset(0)
        , rbLength(0)
        , playbackSpeedMult(1.0)
{
    ibxm::data d;
    d.buffer = moduleFile.data();
    d.length = moduleFile.size();
    char errors[256];
    errors[0] = '\0';
    this->module = ibxm::module_load(&d, errors);
    this->replay = ibxm::new_replay(this->module, 44100, 0);
    //printf("%p IBXM Error: %s\n", this->module, errors);
}

void ModStream::Rewind2()
{
    printf("Rewind not supported\n");
}

void ModStream::SetPlaybackSpeed(double f)
{
    playbackSpeedMult = f;
}

void ModStream::FillBuffer(int16_t *output, int length)
{
    length /= 2;

    while (length > 0) {
        // refill replay buffer if exhausted
        if (rbLength == 0) {
            rbOffset = 0;
            rbLength = ibxm::replay_get_audio(replay, replayBuffer.data(), 0, (int)(playbackSpeedMult * 100.0));
        }

        // number of stereo samples to copy from replay buffer to output buffer
        int nToCopy = std::min(rbLength, length);

        int *input = &replayBuffer[rbOffset * 2];

        // Copy samples
        for (int i = 0; i < nToCopy * 2; i++) {
            int sample = *(input++);

            if (sample < -32768) sample = -32768;
            if (sample >  32767) sample =  32767;

            *(output++) = sample;
        }

        rbOffset += nToCopy;
        rbLength -= nToCopy;
        length -= nToCopy;
    }
}
