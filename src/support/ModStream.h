#pragma once

#include "cmixer.h"

namespace ibxm {
extern "C" {
#include "ibxm.h"
}
}

namespace cmixer {
class ModStream : public Source {
    ibxm::module* module;
    ibxm::replay* replay;
    std::vector<char> moduleFile;
    std::vector<int> replayBuffer;
    int rbOffset;
    int rbLength;
    double playbackSpeedMult;

    void ClearImplementation() override {};
    void RewindImplementation() override {};
    void FillBuffer(int16_t* buffer, int length) override;

public:
    ModStream(std::vector<char>&& rawModule);
    void SetPlaybackSpeed(double f);
};
}