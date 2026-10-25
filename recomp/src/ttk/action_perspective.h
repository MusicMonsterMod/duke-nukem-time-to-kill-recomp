#pragma once
#include <cstring>

namespace ttk {
// Presentation only. These values never authorize movement, aim or fire.
// Future action controls use flat <action>_view keys, first/legacy-third,
// default first. Register a key/menu row only when its eye path ships.
enum class ActionView {First, LegacyThird};
inline bool parse_action_view(const char* value, ActionView& out) {
    if(value && !std::strcmp(value,"first")) {out=ActionView::First;return true;}
    if(value && !std::strcmp(value,"legacy-third")) {out=ActionView::LegacyThird;return true;}
    return false;
}
enum class PerspectiveAction {None, Ground, Swimming, Jetpack, Push, Traversal, WaterExit, Committed, Fall, Ladder, Pole, Mantle, Overhead};
inline const char* perspective_action(PerspectiveAction action) {
    constexpr const char* names[]={"none","ground","swimming","jetpack","push","traversal","water-exit","committed","fall","ladder","pole","mantle","overhead"};
    return names[static_cast<unsigned>(action)];
}
enum class PerspectiveReason {Off, Active, Suspended, Push, Traversal, LegacyChoice,
    Legacy, Identity, Frontend, Scripted, Unsupported, Orbit, Original, WorldChange};
inline const char* perspective_reason(PerspectiveReason reason) {
    constexpr const char* names[]={"off","active","input-suspended","push-object","traversal",
        "legacy-choice","legacy","identity","frontend","scripted","unsupported-camera",
        "orbit","original-camera","world-change"};
    return names[static_cast<unsigned>(reason)];
}
struct PerspectiveFacts {
    bool selected=false, active=false;
    bool overhead=false, mantle=false, pole=false, ladder=false, water_exit=false, push=false, traversal=false, committed=false, fall=false, swim=false, jet=false;
    PerspectiveReason denial=PerspectiveReason::Active;
};
struct ActionPerspective {
    PerspectiveAction action=PerspectiveAction::None;
    bool selected=false, capable=false, effective=false;
    PerspectiveReason reason=PerspectiveReason::Off;
};
inline ActionPerspective resolve_perspective(const PerspectiveFacts& f, ActionView choice=ActionView::First) {
    ActionPerspective r{};r.selected=f.selected;
    if(f.denial!=PerspectiveReason::Active) {r.reason=f.denial;return r;}
    r.action=f.water_exit?PerspectiveAction::WaterExit:f.push?PerspectiveAction::Push:
        f.ladder?PerspectiveAction::Ladder:f.pole?PerspectiveAction::Pole:f.overhead?PerspectiveAction::Overhead:f.mantle?PerspectiveAction::Mantle:f.committed?PerspectiveAction::Committed:f.fall?PerspectiveAction::Fall:
        f.traversal?PerspectiveAction::Traversal:f.swim?PerspectiveAction::Swimming:
        f.jet?PerspectiveAction::Jetpack:PerspectiveAction::Ground;
    r.capable=f.overhead || f.water_exit || f.mantle || f.push || f.ladder || f.pole || !f.traversal || f.committed || f.fall;
    r.effective=f.selected && r.capable && choice==ActionView::First;
    r.reason=!f.selected?PerspectiveReason::Off:
        !r.capable?PerspectiveReason::Traversal:choice==ActionView::LegacyThird?PerspectiveReason::LegacyChoice:
        f.active?PerspectiveReason::Active:PerspectiveReason::Suspended;
    return r;
}
}
