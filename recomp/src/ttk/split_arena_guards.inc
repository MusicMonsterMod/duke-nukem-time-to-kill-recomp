// D34C: derived read-only by level_overlay_guards.py --arenas.
// Separate from campaign guards. Does not enable a gameplay lease.
struct SplitArenaGuard { unsigned level, tag, address, size; const char* digest; };
static constexpr SplitArenaGuard split_arena_guards[] = {
    // /D1/DB15/BASEOVR.OVR: tag 0x12, 1420 bytes, 0 scratch bytes.
    {15, 0x12, 0x800ca968, 1420, "1b99b40eca8f780480d211af70727268932df25479c87232def948083aa5bd65"},
    // /D1/DB16/BASEOVR.OVR: tag 0x12, 1420 bytes, 0 scratch bytes.
    {16, 0x12, 0x800ca968, 1420, "1b99b40eca8f780480d211af70727268932df25479c87232def948083aa5bd65"},
    // /D1/DB17/BASEOVR.OVR: tag 0x12, 1420 bytes, 0 scratch bytes.
    {17, 0x12, 0x800ca968, 1420, "1b99b40eca8f780480d211af70727268932df25479c87232def948083aa5bd65"},
    // /D1/DB18/BASEOVR.OVR: tag 0x12, 1420 bytes, 0 scratch bytes.
    {18, 0x12, 0x800ca968, 1420, "1b99b40eca8f780480d211af70727268932df25479c87232def948083aa5bd65"},
    // /D1/DB19/BASEOVR.OVR: tag 0x12, 1420 bytes, 0 scratch bytes.
    {19, 0x12, 0x800ca968, 1420, "1b99b40eca8f780480d211af70727268932df25479c87232def948083aa5bd65"},
    // /D2/DB20/BASEOVR.OVR: tag 0x12, 1420 bytes, 0 scratch bytes.
    {20, 0x12, 0x800ca968, 1420, "1b99b40eca8f780480d211af70727268932df25479c87232def948083aa5bd65"},
};
