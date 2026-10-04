#pragma once
#include "code_identity.h"

namespace ttk {
// Original SLUS-00583 sky composition. These three calls build a fresh matrix
// around the view eye, outside the world-object draw loop. See note 92.
inline bool sky_transform_call(uint32_t camera,uint32_t ra) {
    return camera==0x800d6eb0u &&
        (ra==0x80038c68u || ra==0x80038d7cu || ra==0x80038e18u);
}

class SkyRenderIdentity {
    struct Guard {uint32_t address,size;const char* digest;};
    std::array<std::vector<uint32_t>,1> expected;
    IdentityMemo memo;
public:
    bool valid(uint64_t frame,uint32_t generation,const uint8_t* ram) {
        if(memo.valid(frame,generation)) return memo.ok;
        // Resident EXE code, not a level-overlay address. Authenticate the
        // entire routine, including timer, matrix construction and draw calls.
        const Guard guards[]={{0x800388e4u,0x578u,
            "c78073edff91c270605ab870d7804078115604c2c49073eb79d359fd5322df13"}};
        const bool ok=code_identity(guards,expected,psx_mod_read_word,ram);
        memo.set(frame,generation,ok);
        return ok;
    }
};
}
