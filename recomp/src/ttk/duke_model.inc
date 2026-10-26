// Duke model joints, shared by the swim aim (D08O2B), first person (D11D)
// and the kick (D12B).
static constexpr unsigned duke_joints=19;
// D11D/D12B: Duke's model record (player+0x40) has byte 0 = joint count, a
// part -> joint table at +0x24 (the game reads the right hand from it, +0x33 =
// part 0xf), and 0x28-byte joint records from +0x44 (byte 1 the part, byte 2
// the hierarchy level). Costumes order their joints differently: the first
// map, medieval and Roman models put the neck at joint 9, the cowboy model
// (LEVEL01 and the other Old West levels) at joint 15, with a hip-height part
// at joint 9. Joints are therefore always found by part, never by index.
static constexpr unsigned part_right_hip=0x05, part_spine=0x09, part_arm_first=0x0a,
    part_arm_last=0x0e, part_neck=0x10, part_left_shoulder=0x0a, part_right_shoulder=0x0d, part_backpack=0x14;
static uint32_t duke_model() {
    const uint32_t desc=psx_mod_read_word(player+0x40);
    return desc>=0x80010000 && desc<=0x801fff00 && psx_mod_read_byte(desc)==duke_joints ? desc : 0;
}
// Joint index of a part, or -1 when the table and the joint's record disagree.
static int duke_part_joint(uint32_t desc,unsigned part) {
    const unsigned joint=psx_mod_read_byte(desc+0x24+part);
    return joint<duke_joints && psx_mod_read_byte(desc+0x44+0x28*joint+1)==part ? (int)joint : -1;
}
// Part of a joint (record byte 1), checked against the table.
static int duke_joint_part(uint32_t desc,unsigned joint) {
    if(joint>=duke_joints) return -1;
    const unsigned part=psx_mod_read_byte(desc+0x44+0x28*joint+1);
    return part<0x20 && psx_mod_read_byte(desc+0x24+part)==joint ? (int)part : -1;
}
