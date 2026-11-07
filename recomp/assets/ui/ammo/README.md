# Ammo box icons (D08A32)

Original 14x14 icons in Time to Kill's style, inside a 16x16 transparent
canvas, one per weapon for the bottom-right ammo box in Modernized. Each frame
has a full-colour PNG and its 15-colour HUD cell (`-15col.png`: 15 colours in
PSX 15-bit colour plus transparent). `ammo.json` lists the frames per weapon
slot; `tools/local/build_ammo_icons.py` writes them into
`src/ttk/ammo_icons.h` as 4bpp cells and palettes (`--check` verifies the
header). The Gatling gun has no frames: it keeps the game's own cartridge belt.

| Weapon | Frames |
| --- | --- |
| Throwing knife, axe, shotgun, Buffalo rifle, RPG, flamethrower, pipe bomb, holy hand grenade, dynamite | one still icon |
| Crossbow | `crossbow-00` to `-13`: three bolts at rest, a shot sends the top bolt off and the stack moves up; at three or fewer the stack shows what is left, blank when empty |
| Desert Eagle | `deagle-00` to `-23`: the top of one magazine following the clip: rest 0-4+, shots, the reload |
| Energy weapon | `energy-0` to `-7`: the atom at rest, then a tumble and flare per shot |
| Freezer | `freezer-0` to `-3`: rest, then the shot glow (peak, mid, low) |
