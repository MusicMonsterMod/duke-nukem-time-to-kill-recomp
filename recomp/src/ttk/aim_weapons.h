#pragma once
namespace ttk {
// Authenticated base weapon records, IDs 1..14: throws, guns, flame and energy.
// Upgraded flame/Gatling/RPG records resolve to 27/28/29 in the same dispatch.
// Boot has no projectile aim path.
inline bool view_weapon_supported(unsigned weapon) { return (weapon>=1 && weapon<=14) || (weapon>=27 && weapon<=29); }
inline bool thrown_weapon(unsigned weapon) { return weapon==1 || weapon==2 || (weapon>=12 && weapon<=14); }
}
