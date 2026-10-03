// LEVEL00 convenience, Modernized only. Keep the original secret body, dialogue
// bit, moving bed, sound and one-shot counter. Change ONLY its prerequisite:
// (flags & 0x408) == 8 -> (flags & 0x400) == 0.
static constexpr uint32_t apartment_compare=0x800cc57c, apartment_mask=0x800cc580;
static bool apartment_patch_active() {
    return input_modernized() && psx_mod_read_word(apartment_compare)==0x24020000 &&
        psx_mod_read_word(apartment_mask)==0x30a30400;
}
static uint32_t apartment_identity_word(uint32_t address) {
    // Only the two apartment words need a pair check. Doing this on every
    // guarded word (identity runs many times per frame) triples guest RAM
    // traffic and stalls gameplay/audio.
    if(address!=apartment_compare && address!=apartment_mask)
        return psx_mod_read_word(address);
    const uint32_t cmp=psx_mod_read_word(apartment_compare);
    const uint32_t msk=psx_mod_read_word(apartment_mask);
    const bool original=cmp==0x24020008 && msk==0x30a30408;
    const bool patched=cmp==0x24020000 && msk==0x30a30400;
    if(original || patched) {
        if(address==apartment_compare)return 0x24020008;
        return 0x30a30408;
    }
    return psx_mod_read_word(address);
}
static void apartment_patch_install() {
    // Caller authenticated the full resident/overlay identity and live player
    // camera ownership. These writes invalidate executable RAM through the
    // runtime service; never patch generated C or the disc.
    if(psx_mod_read_word(0x800be570)!=0 || g_precise_mode || g_ls_mode || g_psx_call_bail ||
       psx_mod_read_word(apartment_compare)!=0x24020008 ||
       psx_mod_read_word(apartment_mask)!=0x30a30408)return;
    psx_mod_write_code_word(apartment_compare,0x24020000);
    psx_mod_write_code_word(apartment_mask,0x30a30400);
    std::fprintf(stderr,"[TTK apartment] Modernized lights-off secret prerequisite enabled (LEVEL00)\n");
}
