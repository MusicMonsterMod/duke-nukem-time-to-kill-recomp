# D39 - Screen flash framework (Duke3D `palfrom`-style colour flashes)

Job: [D39](../MODERNIZATION_JOBS.md) and D39A-D39G. Status: research intake
2026-11-03; no code yet. Follows D08G7 ([note 369](369-d08g7-kroz-feel.md)),
accepted as preliminary.

## Why

The user wants to own coloured screen flashes the way EDuke32 does: hit,
damage, pickup, explosion (white on a big blast, as in their own CON mods)
and the god-mode flash. D08G7 showed the limit of drawing them as PS1 GPU
commands: the GPU has four fixed blend modes (B/2+F/2, B+F, B-F, B+F/4), so a
fade toward a colour by a variable amount can only be approximated, and its
level stepped at the game's 30 Hz (20 Hz on overrun) tick, which staggers on
a 180 Hz display (user, 2026-11-03: "the fade isnt smooth enough, it kind of
staggers"). The renderer is ours (Polymorph, `recomp/psxrecomp`, tracked in
`recomp/patches/time-to-kill-accepted-source.patch`); the limit was the
chosen path, not the stack.

## Duke3D / EDuke32 behaviour (the spec)

`palfrom <intensity> [red] [green] [blue]` (EDuke32 wiki "Palfrom",
"Pals_time"): intensity 0-63 (63 fully opaque; above 63 prolongs full
opacity), colour components 0-63 (6-bit palette units), omitted components
0. It sets the player's `pals` (colour) and `pals_time` (intensity); a new
palfrom replaces the current flash.

Measured from the user's EDuke32 recording (D08G7, three god-off flashes;
mean red dominance per 60 fps frame):

- weight = intensity / 64 toward the colour (x4 to 8-bit): the god-off
  flash is intensity 32 toward red 32 (G, B x0.47; R x0.47 + 63 in video
  levels, i.e. half way to red ~128);
- onset instant; about 2 tics at the peak; then intensity falls by 1 a
  30 Hz tic, a straight line, to 0 (32 tics, about 1.1 s);
- the whole frame is tinted: world, weapon, status bar and quote text.

To confirm in D39A in EDuke32 itself (user's install and `kroztest` save):
the decrement per tic, the >63 hold, override versus "keep the stronger",
whether the 2-tic hold is real or the order of tic and draw, and the blend
space (palette entries, so per-channel linear in 6-bit units).

### Where Duke3D flashes (GAME.CON in the user's DUKE3D.GRP, 35 uses)

| Event | palfrom | Look |
| --- | --- | --- |
| Item pickup (health, armour, ammo, inventory; quick get) | 16 0 32 | light green |
| Weapon pickup | 32 0 32 | green |
| Shield (pigcop armour) | 24 0 32 | green |
| Hit by fire laser, shark bite | 32 32 | red |
| Hit by RPG / radius explosion | 48 52 | strong red |
| Knee (kick) hit | 16 32 | light red |
| Shrinker / expander (COOLEXPLOSION1) hit | 48 48 0 48 | magenta |
| Grow spark (dying) | 48 48 | red |
| Frozen (each tic while frozen) | 16 0 0 24 | blue |
| Squished | 32 63 63 63 | white |
| Gibbed by growing / squished small | 48 64 | full red |
| Fire, burning barrel, steam, burning state (near, damaging) | 16 16 / 24 16 / 32 32 | red/orange |
| Enemy melee (octabrain 8 32, newbeast 16 16, turret 32 16, commander 32, boss4 32 32 0 0) | various | red |
| Boss stomp | 63 63 | full red |
| god mode off (engine: a zero-damage hit) | (32 32) | red |

Night vision and steroids are palette swaps in Duke3D, not palfrom.

## TTK side (known)

- TTK draws no hit tint. Its damage handler `0x800a40a8` (hooked since D08A13)
  rumbles and plays pain sounds per damage type (note 369 table: type 11 fall,
  12 toxic gas, mode 5 under water, others `0x2009`/`0x200a`). It is the
  natural source for damage flashes, with the type, amount, attacker and
  source object in its arguments.
- Pickups: `0x80081a48` (hooked, D08A9). Explosions: effect spawns such as
  `0x8006d594(type, pos, ...)` and the explosion/death calls (`0x80070f7c`)
  need a survey (D39F).
- The flash state already lives in guest mod memory (the word after the D20C
  mirror, tag `0x5d087000`), so savestates and redraw workers carry it.

## Design direction (to settle in D39B/C)

1. **Polymorph present tint (framework).** A final full-frame pass: out =
   mix(frame, colour, weight), applied to the composed image after the game
   chains (choose: before or after host overlays such as the inventory box
   and OSD; Duke3D tints its text). Exact, any colour and weight.
2. **Display-rate fade.** The renderer evaluates the weight per present from
   the flash's start and the interpolated game time (redraw fraction), so a
   fade is smooth at 120/180/240 Hz while the game logic stays 30 Hz;
   optionally Duke3D-faithful 30 Hz steps as a setting.
3. **Game API.** `screen_flash(intensity, r, g, b)` with palfrom units and
   rules, state in mod memory; a console command (`flash 48 63 63 63`) and
   debug JSON for tests; Vanilla never flashes.
4. **Classic renderer** (frozen): keep the D08G7 add/subtract quads as its
   fallback, or no flash.
5. **Accessibility:** a Screen Flashes option (On / Reduced / Off), D21.

## Risks

- Present-path work must cover the main present, render replay images,
  `present_shot`, widescreen bezels and the savestate panel; the tint must not
  apply to menus, loading screens or FMV.
- The redraw fraction must be the same one the replay picks, or the fade can
  step against motion.
- Framework edits need `export_runtime_patch.py` and a clean-pin check.
