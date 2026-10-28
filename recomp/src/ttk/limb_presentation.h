#pragma once
#include <cstdint>

namespace ttk {
// D36G: render ownership only. These semantic part IDs are not costume joint
// indices. Eligibility and animation remain with the existing guarded hooks.
enum class LimbOwner : uint8_t { Native, Interaction, HiddenArm, DisplayHand, DisplayArm, Kick, Body };
enum class LimbSpace : uint8_t { NativeWorld, AtEye, CameraRelative };
enum class LimbDepth : uint8_t { Native, WorldClip, Viewmodel };
enum class LimbLight : uint8_t { Native, CorrectProximityFade };
enum class LimbCleanup : uint8_t { None, NextJointOrDrawBoundary };
struct LimbPresentation {
    LimbOwner owner=LimbOwner::Native;
    uint32_t parts=0;
    LimbSpace space=LimbSpace::NativeWorld;
    LimbDepth depth=LimbDepth::Native;
    LimbLight light=LimbLight::Native;
    LimbCleanup cleanup=LimbCleanup::None;
    bool bridge_joint_depth=false;
    bool viewmodel() const { return depth==LimbDepth::Viewmodel; }
    bool interaction() const { return owner==LimbOwner::Interaction; }
    bool world_clip() const { return depth==LimbDepth::WorldClip; }
    // D14B: only camera-relative hands/arms and their attached equipment use
    // the accepted H=256 presentation. Native contact limbs, body and kick
    // retain the world projection, even when they share a depth layer.
    bool fixed_projection() const {
        return owner==LimbOwner::DisplayHand || owner==LimbOwner::DisplayArm;
    }
};
constexpr uint32_t limb_part(int part) { return part>=0 && part<32 ? 1u<<part : 0; }
constexpr LimbPresentation limb_presentation(LimbOwner owner,int part,bool bridge=false) {
    switch(owner) {
    case LimbOwner::Interaction:
        return {owner,limb_part(part),LimbSpace::NativeWorld,LimbDepth::WorldClip,
                LimbLight::Native,LimbCleanup::NextJointOrDrawBoundary,bridge};
    case LimbOwner::Body:
        return {owner,limb_part(part),LimbSpace::NativeWorld,LimbDepth::WorldClip,
                LimbLight::CorrectProximityFade,LimbCleanup::NextJointOrDrawBoundary,bridge};
    case LimbOwner::HiddenArm:
        return {owner,limb_part(part),LimbSpace::AtEye,LimbDepth::Native,
                LimbLight::Native,LimbCleanup::NextJointOrDrawBoundary,false};
    case LimbOwner::DisplayArm:
        return {owner,limb_part(part),LimbSpace::CameraRelative,LimbDepth::Viewmodel,
                LimbLight::CorrectProximityFade,LimbCleanup::NextJointOrDrawBoundary,bridge};
    case LimbOwner::DisplayHand:
    case LimbOwner::Kick:
        return {owner,limb_part(part),LimbSpace::CameraRelative,LimbDepth::Viewmodel,
                LimbLight::CorrectProximityFade,LimbCleanup::NextJointOrDrawBoundary,bridge};
    default: return {};
    }
}
// Draw-local, rebuilt by each replay worker; never serialized or allocated in
// guest RAM. A returned Native descriptor grants no host clipping takeover.
LimbPresentation first_person_limb_presentation();
}
