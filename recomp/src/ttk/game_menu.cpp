// D19: the Modernized title OPTIONS menu, replicated and extended. The original
// OPTIONS screen (frontend state 0x800d24e4 = 12, sub-screen 0x800d24d2 = 9,
// handler 0x800841c4) lists SOUND, CONTROLLER and GAME SETTINGS. Modernized
// shows the same list plus PC CONTROLS, DISPLAY and MODERNIZED, and those
// three screens, all drawn by the game's own code: text 0x8008b454, text width
// 0x8008b3c8, sprites 0x8008b678 (the prompt buttons), sliders 0x80083c40 /
// 0x80083edc and the menu sounds 0x8006bbd8. Nothing is drawn by the host, so
// the screens match the originals glyph for glyph and the D17 frontend
// interpolation redraws them like any menu.
//
// While one of these screens is up the sub-screen is 8, a value the frontend's
// handler table (0x80013200) leaves without a handler: the guest draws no menu
// and reads no menu input, and the attract demo keeps playing behind. SOUND,
// CONTROLLER and GAME SETTINGS still run the original handlers (sub-screens 7,
// 4, 11); they return 9, which brings our OPTIONS back with that item
// highlighted. Back from OPTIONS is the original's: sub-screen 10, cursor 0.
//
// Settings: a change is kept as a draft until CONFIRM (CANCEL drops it), as on
// GAME SETTINGS. Confirmed values are written to DNTTK_MENU_SETTINGS_FILE,
// which run.py folds into the Modernized profile when the game closes (and on
// the next launch). Options marked live also apply at once (ttk_live_env).
// Contract: documentation/145-d19-game-menu.md.
//
// D19H: in gameplay the pause menu (0x800be56a = 12) and the death screen (15)
// become lists in the same style (RESUME, MISSION BRIEFING, OPTIONS, LOAD
// GAME, RESTART LEVEL, MAIN MENU; CONTINUE, LOAD GAME, RESTART LEVEL, QUIT)
// on the free screen values 8 and 9, with YES / NO prompts. Each choice does
// what the original's handler and main loop do with it (LOAD GAME: D19K). Contract: documentation/147-d19h-pause-menu.md.
//
// D19I: in gameplay our pages are drawn at the pause dispatch, where the
// original draws its pause text, so they are as bright as the original's
// (documentation/148-d19i-menu-brightness.md).
//
// D19M: QUIT GAME at the end of both lists asks QUIT GAME? (NO first); YES
// sends the window's own close event, so the program shuts down exactly as
// when the window is closed (documentation/149-d19m-quit-game.md).
// D19P: the death list's QUIT reads QUIT TO MENU (asking QUIT TO MENU?);
// it still gives up through the game over screen to the title.
//
// D19K: LOAD GAME on the title, pause and death lists opens our LOAD GAME
// page instead of the memory card's: the savestate slots (D24B: level name,
// era, save time) as a list in the menu font, the chosen slot's thumbnail
// beside it (a host image placed on the game picture), then LOAD THIS GAME?.
// CONFIRM loads the slot as F7 does. The original's startup memory card
// check (frontend states 1-3) still runs, unseen and without its waits
// (documentation/169-d19k-load-game.md).
//
// D19J: the title main menu (frontend state 7), CHOOSE DIFFICULTY LEVEL (8),
// TWO PLAYER OPTIONS (8) and CHOOSE CHARACTER (9) are ours too, plus QUIT GAME.
// They run on state 12 with the free sub-screen 12 (no handler: the attract
// demo and its loads go on), drawn with the originals' own calls and
// positions, and keep their state in the originals' variables. Starting a
// game hands the last page to the original handler with one Cross press: it
// draws the same image and exits the frontend itself
// (documentation/150-d19j-title-menu.md).
//
// D20B: the one-player statistics screen (game mode 2) is ours too: the same
// lines, positions and prompts, drawn at the flip of its run handler, and SAVE
// opens our SAVE GAME page instead of the memory card's. The original still
// runs underneath (its timer is held, so it never shows its prompts or reads
// the pad, and its text is hidden); CONFIRM hands it the result its own Cross
// gives (documentation/179-d20b-statistics-screen.md).
#include "game_menu.h"
#include "live_env.h"
#include "cpu_state.h"
#include "mod_plugins.h"
#include "code_identity.h"
#include "pc_input.h"
#include "modern_controls.h"
#include "psx_sdl.h"
#include "savestate.h"
#include "savestate_slots.h"
#include "ttk_font.h"
#include "inventory_hud.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <mutex>
#include <string>
#include <vector>

extern "C" int g_psx_call_bail, g_precise_mode, g_ls_mode;

namespace ttk {bool frame_rate_live();bool frame_rate_set(const char* value);}

namespace ttk {
namespace {
struct Guard {uint32_t address,size;const char* digest;};
// Read-only digests of the owned executable's bytes (tests/local/test_game_menu.py).
const Guard guards[]={
    {0x80013200u,0x30u,"c8df06f70c493d55ff7bd25b49dd202cd611692bb6a445213eed75d5dcf75dd6"},        // frontend sub-screen handler table
    {0x80023228u,0xd8u,"f977b9b9f9075bc46ea87de071e8a37912fb9a2205bb521978ee07e7e9d1dec7"},  // state 12 entry and sub-screen dispatch
    {0x80083c40u,0x170u,"7ff2ef0497a53afcf63c77b1ccb414f0f56a439844ec59d150d605b6ff480841"},    // slider init
    {0x80083edcu,0x204u,"b26f6edb991ac07642f0fd54db110440042b5884fdd45a94f6a86492bd26eef3"},    // slider draw
    {0x8008411cu,0x418u,"bf064314cbaaf45fc8e26ccd6d489a541ce00344518bb82b1b727f2b2aac5d62"},  // prompt helper and the OPTIONS handler
    {0x80013558u,0x40u,"e38a09bc1b05e4f6850535e623455c202dadb6846e794b920a5094cbc8550df2"},   // pause menu handler table
    {0x80025fb0u,0x1b4u,"954687d1cca308195b7207a86abbd7343d565e70529d788655aeef18c545dd3d"},  // pause menu dispatch
    {0x80086174u,0x308u,"a51470c88430cd8b1d31be722968c6f26c74b2b8876ccf75f4a970218049f41a"},  // pause OPTIONS handler
    {0x8008b210u,0x3e8u,"4a2ed47060bc5ca37cc486f180c0a3f949b941d9ad1e0979a6ab7f480ed45cc9"},     // glyph, width and text
    {0x8008b678u,0x40u,"226e8fcb4f143545c487c091f96ca9a853daa3a332e5ab5ad9e66cd4038c188a"},    // sprite draw entry
    // Font glyph tables from 0x800c3f38 (0x800c3f32 and 0x800c3f34 are menu
    // variables). D19P: 0x800c3f94 is the selected item (D08A6), not guarded:
    // a state with an item selected failed the check and kept the original menus.
    {0x800c3f38u,0x5cu,"a73396cc61ae5213b2ebcbc29e8300c06854a58ada6ef55179341636c2f926ee"},
    // D19Q: GAME SETTINGS' word 0x800c3f9c (GORE, STATUS BARS) and GAME SPEED
    // 0x800c3fa0 are the player's settings (the memory card restores them):
    // not guarded. A check first made after a card with other settings had
    // loaded (a savestate restored early) kept the original menus.
    {0x800c3f98u,0x4u,"df3f619804a92fdb4057192dc43dd748ea778adc52bc498ce80524c014b81119"},
    {0x800c3fa4u,0x38cu,"1833d1cc459e582980e7fed23d3171a68f99677561b9ad13626832bb9413b7ec"},
    {0x800c43b4u,0xf0u,"9ea07ba25cff15a7da9de43768b28d70b836cf13269c705e70e6730b55395b3b"},   // sprite records and font info
    {0x800c60f4u,0x400u,"1298dbf2ac90b8efff35a60c7ec93bc3dd6897d1434ec84ba5b1871093b09a7a"},  // text table
    // D19H: the pause and death screens our lists stand in for.
    {0x800857e4u,0x990u,"80acc5a03247a7a97930a0d8278cc68839fd0694914e5e1dee3cf04e5d5d4cc8"},  // pause top handler (12)
    {0x8008647cu,0x7f0u,"af33ec8f74e797df8f960b844588faa1011fdf22e8a6469ca34177a363b1da38"},  // death screen handler (15)
    {0x8002690cu,0x90u,"03968170f1610cb68b72423a8fb70e3664bd36899f7b5d15b2fdaba62fe67c19"},   // mode 1: dispatch, resume on 0
    {0x800c3d2cu,0x40u,"c9ef39aacbbd3027c182f6bdb255613e0794c223519cfcda97ce4a45a604c513"},   // level name string ids
    {0x800c3e10u,0x104u,"76189e6a61e1552476bb95a777ad819198c94812700157401ed2092b1f146fb8"},  // briefing string ids (13 x 10)
    {0x80094a38u,0x40u,"8e661ce6839ad2f8cefacf50eea5e0a97e9f547f5f060ce100590cc04d12f03b"},   // game event (restart, quit, replay)
    {0x8006ac24u,0x30u,"d45ab3dd28ba07a76ba1e90b5f64cc4ab0f3342cd41c224cbe0b5e022b8265d9"},   // resume: sound
    {0x8003c3dcu,0x40u,"b02ecd72ad0fe5b8072a35109c6e6c88ae7cd108fbfe80c604ab8a3999633b64"},   // resume: music
    {0x800c64f4u,0x26cu,"3716c4900759d23bbd1d3f71ca5024a5aef410814ab33aca47a435d1d6fd5945"},  // text table 0x100-0x19a (briefings)
    // D19J: the title pages we stand in for and the tables they read.
    {0x80022324u,0xfd8u,"cbcdd79ea3bfa28c53362e64f06b2bf3cb192bec173cdd906f1aa349e801b908"},  // frontend tick and states 0-12
    {0x800131b0u,0x34u,"14222073a6cd63ed99f8768a6ac93f00160a251c4ead8600cc096210b79636b8"},   // frontend state table
    {0x800131e8u,0x14u,"fe4a165c813fddb5da320da4e98bd354f3c4d3beee23f4728837ced6d7abb939"},   // main menu choice table
    {0x80084534u,0x318u,"f117c07c8be0dad501401838d2bc854c3b1a999a08808baa0187e89eb573a14d"},  // CHOOSE DIFFICULTY LEVEL
    {0x8008484cu,0x814u,"18b50b14cc0a6cdf340c521f56855fef0a0839921c294dab61750ec4d393d810"},  // TWO PLAYER OPTIONS
    {0x80085060u,0x6b0u,"5bbfb37b63bccf7306ddd0d46e2c9b7e163509795ece67dd50fddf601bd36ec5"},  // CHOOSE CHARACTER
    {0x800c3de4u,0x10u,"5cd023106a3b69feb9e1a71b77471c35d4a43b210a1751a4a3aab314a1dfc30f"},   // difficulty string ids
    {0x80016e8cu,0x30u,"9d528df15f856133001784171950a0e3c1a7c9d05b1ae7ecb1393b71a5f0b164"},   // two player levels, formats, characters
    // D19N: GAME SETTINGS' values, the pad layouts and the memory card's settings.
    {0x80089920u,0xacu,"755b97bdc2a6a2074cdca0f00b7c6c3d645c01f623163a83bfdb30874cb36031"},   // GAME SPEED apply
    {0x800899ccu,0x8e8u,"31f94a7078b4714634cd78d7a9bd4d56d49f97da7e6d014ffbc26ef908d260b7"},  // GAME SETTINGS handler
    {0x80082f64u,0x220u,"e860559817616eac4b4d75765356b5ea2ccf6a10b6f9e678d9ebcd9dbf572308"},  // card settings pack and unpack
    {0x8001e394u,0xb0u,"93339da683cb20d974a0bbdbd408155d2f24ceb84afdef270976b091aea716ce"},   // pad layout copies
    {0x800bda7cu,0x10u,"fb1a32f64869bc2512840a45c4e2191f18294129c8cfa8bc2f3d6d7b49b08bb3"},   // default pad layout
    {0x800270e8u,0xe8u,"eb6e3fbd7db19271a49d47b0767dafd6f9ff9d4035320933519db031d8d0ffe2"},   // D20B: game mode 1 exit (to the statistics screen)
    // D20B: game mode 2 (init, enter, the challenge, two-player and one-player
    // screens, run, next level, exit).
    {0x800271d0u,0x1018u,"49525d942bc3a82ceb034bde4dfc33f9b69b654a76fa60f2625907c8a3fbf572"},
};
std::array<std::vector<uint32_t>,sizeof guards/sizeof guards[0]> expected;

constexpr uint32_t game_mode=0x800bcbb0, controller_pad=0x800be56c, geometry=0x800d2210;
constexpr uint32_t image_counter=0x800bdbd0;
constexpr uint32_t fn_text=0x8008b454, fn_width=0x8008b3c8, fn_sprite=0x8008b678;
constexpr uint32_t fn_slider_init=0x80083c40, fn_slider_draw=0x80083edc, fn_sound=0x8006bbd8;
constexpr uint32_t fn_prompt=0x8008411c, sprites=0x800c43b4, sentinel=0x800000fc;
// D19G: the glyph call (0x8008b210) adds one 40-byte packet at the pointer
// 0x800d67a8, its CLUT at +14. Section headings use the font column's amber
// ramp (row 239, 14 rows below gold 225; entry 0 transparent like gold's).
constexpr uint32_t glyph_next=0x800d67a8, glyph_size=40;
constexpr int clut_amber=14*64;
// D19T: the font has no double quote (the small font's '"' entry is its
// apostrophe, the large one's stray pixels): a quote is two of its own
// apostrophes, closing as they are, opening turned half round (both texture
// axes flipped over the apostrophe's ink rows). Glyph call (char, x, y, flags);
// glyph table 0x800c3f30 + font * 0x100 + char * 4: u, v, width (guarded).
constexpr uint32_t fn_glyph=0x8008b210, glyph_table=0x800c3f30;
struct QuoteMetrics {int pair_step,ink_rows;};
const QuoteMetrics quote_metrics[]={{5,8},{6,10}}; // small font, menu font
// D20D: the small font has no '!' or '%' either. They are the Modernized text
// pack's (build_ttk_fonts.py: D24C '!' from I and period, D24E '%' as './.'),
// drawn here as 1-row runs copied from the font's own I, '.' and '/' texels
// (analysis d20d-stats/runs.py): {x, row, length, u, v} from the pen. The game's
// glyph quads take u..u+w, v..v+11 at x..x+w, y..y+11 (exclusive).
struct GlyphRun {uint8_t x,y,n,u,v;};
constexpr GlyphRun small_bang[]={{3,0,4,18,84},{3,1,5,18,85},{2,2,5,17,86},{2,3,3,17,87},{5,3,1,21,87},{2,4,2,17,88},
    {4,4,1,21,88},{1,5,3,16,89},{4,5,1,21,89},{1,6,4,16,94},{0,8,6,17,80},{0,9,6,17,81},{1,10,4,16,94}};
constexpr GlyphRun small_percent[]={{0,0,6,17,80},{8,0,2,20,84},{0,1,6,17,81},{7,1,4,19,85},{1,2,4,16,94},{7,2,4,19,86},
    {6,3,4,18,87},{6,4,4,27,76},{5,5,4,19,80},{5,6,4,18,90},{4,7,4,17,90},{4,8,4,18,90},{9,8,6,17,80},{3,9,4,16,92},
    {9,9,6,17,81},{4,10,3,16,94},{10,10,4,16,94}};
// D19N: a row with no effect is drawn at this texture modulation (0x80 is
// full; D08A6 dims an off gadget's digits the same way). The font's grey CLUT
// row 224 has an opaque entry 0: it drew a box behind each glyph.
constexpr uint8_t dim_level=0x48;
constexpr uint16_t ours_sub=8, death_sub=9;
// D19N: the original GAME SETTINGS' values (0x800899cc): the settings word
// (bit 0 AUTO AIM, bits 1-2 GORE 0-2, bit 3 STATUS BARS, bit 4 vibration,
// bit 5 a vibrating pad) and GAME SPEED (0-100), applied by 0x80089920. The pad
// layouts (16 bytes per pad, default 0x800bda7c) are the original CONTROLLER's.
// The memory card's settings are packed by 0x80082f64 and unpacked by
// 0x80083070, which ends with 0x80089920 (return 0x8008315c).
constexpr uint32_t settings_word=0x800c3f9c, game_speed=0x800c3fa0, fn_game_speed=0x80089920;
constexpr uint32_t pad_layouts=0x800d1b60, default_layout=0x800bda7c, layout_size=16;
constexpr uint32_t fn_card_pack=0x80082f64, card_unpacked=0x8008315cu;
constexpr uint16_t auto_aim_bit=1, gore_bits=6, status_bars_bit=8;
// D19H: the pause menu (0x800be56a = 12, its prompt 0x800dd1c4, reset flag
// 0x800c3f28) and the death screen (15, prompt 0x800dd1c6, reset 0x800c3f34).
// D19K: their LOAD (pause screen 5, death prompt 3) is no longer used.
constexpr uint32_t pause_screen=0x800be56a;
constexpr uint32_t pause_reset=0x800c3f28, death_reset=0x800c3f34;
constexpr uint32_t level_index=0x800be570, players=0x800c27bc, continues=0x800d7411;
constexpr uint32_t level_names=0x800c3d2c, briefings=0x800c3e10, strings=0x800c60f4;
constexpr uint16_t pause_top=12, death_top=15;
constexpr uint32_t paused=0x800be568, player=0x800d7198, player_size=0x8a4, player_flags=0x224;
constexpr uint32_t fn_resume_sound=0x8006ac24, fn_resume_music=0x8003c3dc, fn_event=0x80094a38;
constexpr uint32_t continue_request=0x800d71cd; // byte 2: the death screen's CONTINUE (not the challenge stages)
constexpr uint16_t restart_code=13, main_menu_code=1; // the pause handler's results the main loop acts on
constexpr uint32_t event_replay=0xfd, event_restart=0xfe, event_quit=0xff;
// Text ids: YES, NO, REALLY QUIT?, RESTART LEVEL?, CONFIRM CONTINUE, CONFIRM
// REPLAY, CONTINUE / CONTINUES REMAINING.
constexpr int text_yes=0xfb, text_no=0xfc, text_quit=0x102, text_restart=0x103;
constexpr int text_continue=0x104, text_replay=0x113, text_remaining=0xf7, text_remaining_many=0xf8;
constexpr int text_last=0x19a; // the highest id a level name or briefing uses (guarded)
// D19J: the frontend (state 0x800d24e4 and its handlers' variables).
constexpr uint32_t fe_state=0x800d24e4, fe_cursor=0x800d24e6, fe_items=0x800d2528, fe_timer=0x800d24d0, fe_fields=0x800d21fc;
constexpr uint32_t players_choice=0x800bdea4, level_choice=0x800bdeaa;
constexpr uint32_t diff_cursor=0x800c3d7c, diff_saved=0x800c3df6, diff_fresh=0x800c3df8, diff_ids=0x800c3de4;
constexpr uint32_t two_cursor=0x800dd1b0, two_split=0x800dd1b2, two_level=0x800c3dfc, two_time=0x800c3dfe, two_kills=0x800c3e00;
constexpr uint32_t two_fresh=0x800c3e04, split_mode=0x800be564, two_levels=0x80016e8c, two_format=0x80016e98;
constexpr uint32_t char_choice=0x800dd1b8, char_locks=0x800dd1c0, char_saved=0x800c3e08, char_fresh=0x800c3e0c, char_ids=0x80016ea4;
// Pad history words (0x8001d5ac): pad 1 from +0x10 (Start), pad 2 0x54 on;
// bit 0 = pressed at this tick, bit 1 = at the tick before.
constexpr uint32_t pad_history=0x800d1aa0, history_cross=0x3c;
constexpr uint32_t history_checked[]={0x10,0x14,0x18,0x1c,0x20,0x34,0x3c};
constexpr uint16_t title_sub=12, state_start=4, state_difficulty=8, state_character=9;
constexpr uint16_t idle_fields=0x4650; // state 6: the main menu's idle time before PRESS START
// Where the frontend calls 0x800220f4: state 12 before its sub-screen
// dispatch, states 8 and 9 before their handlers.
constexpr uint32_t from_options=0x80023248, from_difficulty=0x80023130, from_character=0x800231b4;
constexpr uint32_t demo_load=0x800d24e2; // D19O: 0 = a demo scene's blocking load (no pad read)
// D19K: the startup memory card check. State 1 shows CHECKING MEMORY CARD and
// probes the card (0x80082e6c) once its status 0x800bdeb0 (starts at -3) has
// counted up to -1; result 0 / 1 (a card) leads to state 2, else state 3.
// State 2 shows CHECKING SAVE GAME FILE, waits on 0x800d24d0 (-3 up to 0),
// reads the file (0x80083490) and unpacks the saved settings (0x80083070)
// before PRESS START (state 4). State 3 is the no card / not formatted / no
// free block message, left with CONTINUE (Cross) for state 4.
constexpr uint32_t card_status=0x800bdeb0, card_wait=0x800d24d0;
constexpr uint16_t state_card=1, state_save_file=2, state_card_message=3;
constexpr int text_checking_card=0x39, text_checking_save=0xf2;
constexpr uint32_t card_states_begin=0x80022440, card_states_end=0x80022664; // states 1 and 2
constexpr uint32_t card_unpaced_budget=300; // D19R: fields per tick, well above the probe's 90
// D20B: game mode 2, the statistics screen. Its run handler (0x80027df8)
// draws a screen between 0x8001fe60(2) and the flip 0x8001fcbc(1) (return
// 0x80027f4c), stores its result (0x800d2694: 1 SAVE, -1 CONTINUE) and leaves
// the mode when that is negative. Page 0x800d2672: 0 statistics, 1 the memory
// card's SAVE GAME (0x8008747c), 2 statistics after a save (CONTINUE only).
// The screens show their prompts and read the pad only once the timer
// 0x800d2670 (300 on entry, less the fields 0x800d21fc of each tick) is out:
// one player 0x800277dc, challenge stages 0x80027214 (two players 0x80027474).
constexpr uint32_t stats_timer=0x800d2670, stats_page=0x800d2672, stats_result=0x800d2694, tick_fields=0x800d21fc;
constexpr uint32_t flip=0x8001fcbc, stats_flip_ra=0x80027f4c;
// Game mode 1's exit (0x800270e8): end code 0x800be55e 4 (level complete), or
// 0xfe in a challenge stage, leads to mode 2. The image on screen is still the
// level's last; by mode 2's first flip it has been cleared.
constexpr uint32_t gameplay_exit=0x800270e8, end_code=0x800be55e;
constexpr uint32_t stats_one_begin=0x800277dc, stats_one_end=0x80027df8, stats_challenge_begin=0x80027214, stats_challenge_end=0x80027474;
constexpr uint16_t stats_wait_fields=300;
// The one-player screen's values: the player's kills (+0x3b6), time in ms
// (+0x398), continues (+0x279) and secrets (+0x3b0), the level's secret count,
// the challenge item found (0x800d79f6) and the challenge stages won (bits of
// 0x800d79f7 from stage 21).
constexpr uint32_t secrets_total=0x800beb18, challenge_found=0x800d79f6, challenges_done=0x800d79f7;
constexpr int text_kills_label=0x86, text_time_label=0x87, text_continues_label=0xf9, text_secrets_label=0x88;
constexpr int text_challenge_label=0xfa, text_completed=0xd5, text_failed=0xd6, text_save=0x17, text_continue_prompt=0x19;
// Text ids of the title pages (the originals pass the same).
constexpr int text_title1=1, text_title2=2, text_one=0x57, text_two=0x58, text_load=0x31, text_options=0x59;
constexpr int text_select=0x11, text_confirm=0x12, text_return=0x13, text_change=0x14, text_cancel=0x15, text_start=0x8a;
constexpr int text_difficulty=0x21, text_two_heading=0x2c, text_level=0x25, text_time=0x2d, text_kills=0x2e, text_split=0x24;
constexpr int text_horizontal=0x22, text_vertical=0x23, text_unlimited=0x2f, text_minutes=0x30;
constexpr int text_character=0xc9, text_player1=0xca, text_player2=0xcb;
constexpr uint16_t screen_no_titles=14; // 0x800be56a: the title pages leave their headings out
// Where OPTIONS lives: the title frontend (state 0x800d24e4 = 12, sub-screen
// 0x800d24d2, handler 0x800841c4) and the pause menu (mode 1, 0x800be56a,
// handler 0x80086174). Both use 8 as an unused screen with no handler.
struct MenuContext {
    uint16_t mode; uint32_t state; uint16_t state_value; uint32_t sub; uint16_t options,back;
    uint32_t cursor; const char* back_label;
};
const MenuContext contexts[]={
    {0,0x800d24e4u,12,0x800d24d2u,9,10,0x800c3de0u,"CANCEL"},
    {1,0,0,0x800be56au,2,12,0x800c3f32u,"RETURN"},
};
// The context showing OPTIONS (or one of our screens) now, else null.
const MenuContext* current_context() {
    const uint16_t mode=psx_mod_read_half(game_mode);
    for(const MenuContext& c:contexts)
        if(c.mode==mode && (!c.state || psx_mod_read_half(c.state)==c.state_value)) return &c;
    return nullptr;
}
// Screen values with no handler that our screens use: 8 (OPTIONS, settings,
// the pause list and its pages), 9 in gameplay (the death list), 12 on the
// title (D19J: the main menu and its pages).
bool our_screen(const MenuContext* c,uint16_t sub) {
    return sub==ours_sub || (c->mode==1 && sub==death_sub) || (c->mode==0 && sub==title_sub);
}
// Text flags: font << 8, 4 centred, 8 right-aligned, 1 highlighted (blue CLUT).
constexpr uint32_t font_small=0, font_menu=0x100, centre=4, right=8, blue=1;
enum Icon {icon_triangle=4, icon_cross=6, icon_square=7, icon_change=11, icon_select=12};

struct Choice {const char* label;const char* env;const char* json;};
// D19N: Game rows are the original GAME SETTINGS' own values in guest memory
// (saved on the memory card as before), never in the profile.
enum class Section {Controls, Presentation, Game};
struct Row {
    const char* label; Section section; const char* key; const char* env; bool live; const char* desc;
    std::vector<Choice> choices; int def; // a pick
    bool slider=false; int lo=0,hi=0,step=1; double scale=1; // value * scale is the setting
    bool link=false; // D19G: CONFIRM opens a page (KEY BINDINGS), no value of its own
    bool heading=false; // D19N: an amber section heading (ADVANCED), skipped by the cursor
};
// One value per row: a choice index, or the slider value.
using Values=std::vector<int>;
struct Screen {const char* title; std::vector<Row> rows;};

const Choice on_off[]={{"ON","1","true"},{"OFF","0","false"}};
Row heading(const char* label) {Row r{label,Section::Controls,nullptr,nullptr,false,"",{},0};r.heading=true;return r;}
// Enum rows use the existing draft/apply_live/atomic write_settings path.
Row view_row(const char* label,const char* key,const char* env,const char* help,bool action=false) {
    return {label,Section::Controls,key,env,true,help,
        action?std::vector<Choice>{{"FIRST PERSON","first","\"first\""},
                                   {"THIRD PERSON","legacy-third","\"legacy-third\""}}:
               std::vector<Choice>{{"THIRD PERSON","third","\"third\""},
                                   {"FIRST PERSON","first","\"first\""}},0};
}
// D19N (user-approved layout A, 2026-10-20): OPTIONS is SOUND (the original
// screen), CONTROLS, DISPLAY and GAMEPLAY with the everyday rows, and ADVANCED
// with every original / modern switch under section headings. The original
// CONTROLLER (pad layout) is not offered: Modernized play needs the default
// layout (enforce_modern_settings). GAME SETTINGS' rows moved: GORE and STATUS
// BARS to GAMEPLAY; AUTO AIM (now the profile's aim assist, written into the
// game) and GAME SPEED to ADVANCED > ORIGINAL GAME, with the RED DOT that
// AUTO AIM drives (user review, 2026-10-20). Contract: documentation/166-d19n-options-layout.md.
std::vector<Screen> make_screens() {
    std::vector<Screen> s(4);
    s[0]={"CONTROLS",{
        {"CONTROL SCHEME",Section::Controls,"scheme","DNTTK_CONTROL_SCHEME",true,
         "MODERN: MOUSE LOOK AND STRAFE. LEGACY: TURN WITH A/D",
         {{"MODERN","modern","\"modern\""},{"LEGACY","legacy","\"legacy\""}},0},
        {"MOUSE SENSITIVITY",Section::Controls,"mouse_sensitivity","DNTTK_MOUSE_SENSITIVITY",true,
         "HOW FAR THE CAMERA TURNS FOR EACH MOUSE MOVE",{},12,true,1,200,1,0.01},
        {"INVERT MOUSE",Section::Controls,"invert_y","DNTTK_MOUSE_INVERT_Y",true,
         "MOVING THE MOUSE UP LOOKS DOWN",{{"OFF","0","false"},{"ON","1","true"}},0},
        {"KEY BINDINGS",Section::Controls,nullptr,nullptr,true,"THE KEY OR MOUSE BUTTON FOR EACH ACTION. CONFIRM OPENS THE LIST",
         {{">",">",">"}},0,false,0,0,1,1,true},
    }};
    s[1]={"DISPLAY",{
        {"WINDOW",Section::Presentation,"display",nullptr,true,"HOW THE GAME OPENS. F11 SWITCHES TO FULLSCREEN",
         {{"WINDOWED","windowed","\"windowed\""},{"BORDERLESS","borderless","\"borderless\""},{"EXCLUSIVE","exclusive","\"exclusive\""}},0},
        {"RESOLUTION",Section::Presentation,"internal_scale",nullptr,false,"INTERNAL 3D RESOLUTION. 1X IS THE ORIGINAL",
         {{"1X","1","1"},{"2X","2","2"},{"3X","3","3"},{"4X","4","4"}},3},
        {"WIDESCREEN",Section::Controls,"widescreen","DNTTK_WIDESCREEN",true,"A WIDER VIEW, HUD IN THE CORNERS. MOVIES STAY 4:3",
         {{"16:9","16:9","\"16:9\""},{"16:10","16:10","\"16:10\""},{"21:9","21:9","\"21:9\""},{"AUTO","auto","\"auto\""},{"OFF","off","\"off\""}},0},
        {"FRAME RATE",Section::Controls,"frame_rate","DNTTK_FRAME_RATE",true,"AUTO CHOOSES UP TO 120 FPS. GAME SPEED NEVER CHANGES",
         {{"AUTO","auto","\"auto\""},{"DISPLAY","display","\"display\""},{"30","30","\"30\""},{"60","60","\"60\""},{"120","120","\"120\""},{"144","144","\"144\""},
          {"165","165","\"165\""},{"180","180","\"180\""},{"240","240","\"240\""},{"UNLIMITED","unlimited","\"unlimited\""}},0},
    }};
    s[2]={"GAMEPLAY",{
        view_row("VIEW","view","DNTTK_CAMERA_VIEW","THE FIRST / THIRD PERSON KEY SWITCHES IN GAME"),
        {"SHOULDER",Section::Controls,"shoulder","DNTTK_CAMERA_SHOULDER",true,"THE SHOULDER KEY CYCLES IN GAME",
         {{"CENTER","center","\"center\""},{"RIGHT","right","\"right\""},{"LEFT","left","\"left\""}},0},
        {"CAMERA DISTANCE",Section::Controls,"camera_distance","DNTTK_CAMERA_DISTANCE",true,"ALT AND THE MOUSE WHEEL ALSO SET IT IN GAME",
         {{"ORIGINAL","0","0"},{"1024","1024","1024"},{"1536","1536","1536"},{"2048","2048","2048"},
          {"3072","3072","3072"},{"4096","4096","4096"},{"6144","6144","6144"}},0},
        {"CROSSHAIR",Section::Controls,"crosshair","DNTTK_CROSSHAIR",true,"SHOWS OR HIDES THE CROSSHAIR",
         {on_off[0],on_off[1]},0},
        {"GORE",Section::Game,"gore",nullptr,true,"HOW MUCH BLOOD",{{"NONE","0","0"},{"SOME","1","1"},{"LOTS","2","2"}},2},
        {"STATUS BARS",Section::Game,"status_bars",nullptr,true,"HEALTH, ARMOR, AMMO AND ITEM BOXES",{{"ON","1","1"},{"OFF","0","0"}},0},
        {"VIEW BOB",Section::Controls,"view_bob","DNTTK_VIEW_BOB",true,"FIRST PERSON HEAD BOB WHILE WALKING",
         {{"OFF","off","\"off\""},{"SUBTLE","subtle","\"subtle\""},{"ON","on","\"on\""},{"STRONG","strong","\"strong\""}},2},
    }};
    s[3]={"ADVANCED",{
        heading("CAMERA AND AIM"),
        {"CAMERA",Section::Controls,"camera","DNTTK_CAMERA_MODE",true,"INDEPENDENT: THE MOUSE TURNS THE CAMERA",
         {{"INDEPENDENT","independent","\"independent\""},{"ORIGINAL","original","\"original\""}},0},
        {"WEAPON AIM",Section::Controls,"weapon_aim","DNTTK_WEAPON_AIM",true,"VIEW: SHOTS GO WHERE THE CROSSHAIR POINTS",
         {{"VIEW","view","\"view\""},{"ORIGINAL","original","\"original\""}},0},
        heading("ACTION PERSPECTIVE"),
        view_row("SWIMMING VIEW","swim_view","DNTTK_SWIM_VIEW",
                 "FIRST PERSON ONLY. MODERN CONTROLS / INDEPENDENT CAMERA",true),
        view_row("JETPACK VIEW","jetpack_view","DNTTK_JETPACK_VIEW",
                 "FIRST PERSON ONLY. INDEPENDENT OF JETPACK FLIGHT SCHEME",true),
        view_row("PUSH/PULL VIEW","push_view","DNTTK_PUSH_VIEW",
                 "FIRST PERSON ONLY. GRAB, PUSH AND PULL OBJECTS",true),
        view_row("LADDER VIEW","ladder_view","DNTTK_LADDER_VIEW",
                 "FIRST PERSON ONLY. LADDER MOUNTS, CLIMBING AND EXITS",true),
        view_row("CHAIN/POLE VIEW","pole_view","DNTTK_POLE_VIEW",
                 "FIRST PERSON ONLY. CHAIN AND POLE CLIMBING",true),
        view_row("MANTLE VIEW","mantle_view","DNTTK_MANTLE_VIEW",
                 "FIRST PERSON ONLY. LEDGES, HANGS AND WATER CLIMB-OUTS",true),
        view_row("OVERHEAD VIEW","overhead_view","DNTTK_OVERHEAD_VIEW",
                 "FIRST PERSON ONLY. CEILING HANGS AND MONKEY BARS",true),
        view_row("WALL CLIMB VIEW","wall_view","DNTTK_WALL_VIEW",
                 "FIRST PERSON ONLY. CLIMBING WALLS AND THEIR EXITS",true),
        view_row("USE INTERACTION VIEW","use_view","DNTTK_USE_VIEW",
                 "FIRST PERSON ONLY. DOORS, LEVERS, ITEMS AND NPC ACTIONS",true),
        view_row("MIRROR VIEW","mirror_view","DNTTK_MIRROR_VIEW",
                 "FIRST PERSON ONLY. MIRROR INTERACTION",true),
        heading("MOVEMENT"),
        {"JUMP",Section::Controls,"jump","DNTTK_JUMP",true,"MANUAL: LEAVE ON THE PRESS AND STEER IN THE AIR",
         {{"ASSISTED","assisted","\"assisted\""},{"MANUAL","manual","\"manual\""}},0},
        {"JUMP WALLS",Section::Controls,"jump_walls","DNTTK_JUMP_WALLS",true,"SLIDE: A WALL ONLY STOPS THE MOTION INTO IT",
         {{"SLIDE","slide","\"slide\""},{"ORIGINAL","original","\"original\""}},0},
        {"WALL BACKFLIP",Section::Controls,"wall_backflip","DNTTK_WALL_BACKFLIP",true,"MODERN: NO BACKWARDS LAUNCH OFF STEEP CARS AND SLOPES",
         {{"MODERN","modern","\"modern\""},{"ORIGINAL","original","\"original\""}},0},
        {"CROUCH",Section::Controls,"crouch","DNTTK_CROUCH",true,"MODERN: CROUCH WALK AND CROUCH ON LANDING",
         {{"MODERN","modern","\"modern\""},{"ORIGINAL","original","\"original\""}},0},
        {"SWIMMING",Section::Controls,"swimming","DNTTK_SWIMMING",true,"MODERN: SHIFT WALKS OR RUNS IN WATER AS ON LAND",
         {{"MODERN","modern","\"modern\""},{"ORIGINAL","original","\"original\""}},0},
        {"JETPACK",Section::Controls,"jetpack","DNTTK_JETPACK",false,"EDUKE32: INSTANT J FLIGHT, 61 SECONDS OF FUEL",
         {{"MODERN","modern","\"modern\""},{"CLASSIC","classic","\"classic\""},{"EDUKE32","eduke32","\"eduke32\""}},0},
        heading("ITEMS"),
        {"AUTO BIO MASK",Section::Controls,"auto_bio_mask","DNTTK_AUTO_BIO_MASK",true,"ON: AUTO GAS PROTECTION AND INSTANT USE WHILE FIRING",
         {{"ON","on","\"on\""},{"OFF","off","\"off\""}},0},
        {"NIGHT VISION",Section::Controls,"night_vision","DNTTK_NIGHT_VISION",true,"MODERN: INSTANT ON/OFF WITHOUT INTERRUPTING FIRE",
         {{"MODERN","modern","\"modern\""},{"ORIGINAL","original","\"original\""}},0},
        {"DYNAMITE",Section::Controls,"dynamite","DNTTK_DYNAMITE",true,"MODERN: A DRAWN STICK STAYS UNLIT UNTIL YOU FIRE",
         {{"MODERN","modern","\"modern\""},{"ORIGINAL","original","\"original\""}},0},
        {"STEROIDS",Section::Controls,"steroids","DNTTK_STEROIDS",false,"PORTABLE: A PICKUP IS STORED, R TAKES IT",
         {{"PORTABLE","portable","\"portable\""},{"ORIGINAL","original","\"original\""}},0},
        heading("GRAPHICS"),
        {"RENDERER",Section::Presentation,"renderer",nullptr,false,"SOFTWARE IS THE ORIGINAL LOOK",
         {{"OPENGL","opengl","\"opengl\""},{"SOFTWARE","software","\"software\""}},0},
        {"FILTER",Section::Presentation,"output_filter",nullptr,false,"HOW THE FINISHED IMAGE IS SCALED TO THE WINDOW",
         {{"LINEAR","linear","\"linear\""},{"NEAREST","nearest","\"nearest\""}},0},
        {"DRAW DISTANCE",Section::Controls,"draw_distance","DNTTK_DRAW_DISTANCE",false,"EXTENDED: TWICE THE VIEW, NO DISTANT BLACK SEAMS",
         {{"EXTENDED","extended","\"extended\""},{"ORIGINAL","original","\"original\""}},0},
        {"GEOMETRY",Section::Controls,"geometry_precision","DNTTK_GEOMETRY_PRECISION",false,"CORRECTED: STEADIER POLYGON EDGES",
         {{"ORIGINAL","original","\"original\""},{"CORRECTED","corrected","\"corrected\""}},0},
        {"TEXTURES",Section::Controls,"texture_precision","DNTTK_TEXTURE_PRECISION",false,"CORRECTED: NO TEXTURE WARP",
         {{"ORIGINAL","original","\"original\""},{"CORRECTED","corrected","\"corrected\""}},0},
        heading("PERFORMANCE"),
        {"CPU SPEED",Section::Controls,"cpu_overclock","PSX_CPU_OVERCLOCK",false,"150 KEEPS BUSY SCENES SMOOTH. 100 IS THE ORIGINAL",
         {{"100","100","100"},{"125","125","125"},{"150","150","150"},{"175","175","175"},{"200","200","200"}},2},
        {"CPU TIMING",Section::Controls,"cpu_timing","DNTTK_CPU_TIMING",false,"FAST USES FAR LESS HOST CPU AT THE SAME GAME SPEED",
         {{"FAST","fast","\"fast\""},{"ACCURATE","accurate","\"accurate\""}},0},
        heading("ORIGINAL GAME"),
        // AUTO AIM is the profile's aim assist: ON also turns the original's
        // auto aim on in the game, which the assist and the red dot need.
        {"AUTO AIM",Section::Controls,"aim_assist","DNTTK_AIM_ASSIST",true,"ON: THE ORIGINAL TARGET LOCK HELPS YOUR AIM",
         {{"OFF","off","\"off\""},{"ON","original-lock","\"original-lock\""}},0},
        {"RED DOT",Section::Controls,"red_dot","DNTTK_RED_DOT",true,"THE ORIGINAL TARGET MARKER",
         {{"OFF","0","false"},{"ON","1","true"}},0},
        // 0x80089920 sets Duke's (0x800c3c80) and the enemies' (0x800c3d14)
        // animation rate from it; 50 is the original's default.
        {"GAME SPEED",Section::Game,"game_speed",nullptr,true,"HOW FAST DUKE AND THE ENEMIES MOVE. NOT THE FRAME RATE",
         {},50,true,0,100,5,1},
    }};
    return s;
}

struct Item {const char* label;int guest_sub;int screen;};
// OPTIONS: the original SOUND (its handler's own return value), then ours.
const Item options_items[]={{"SOUND",7,-1},{"CONTROLS",-1,0},{"DISPLAY",-1,1},{"GAMEPLAY",-1,2},{"ADVANCED",-1,3}};
constexpr int option_count=5;

// Recursive: a pad read inside one of our guest calls comes back into
// game_menu_input on this thread; it then leaves the keys for the next read.
std::recursive_mutex mutex;
bool busy=false;
std::vector<Screen> screens;
std::vector<Values> committed, draft;
std::once_flag init_once;
std::deque<std::string> owned_strings; // env values not among the choices (stable addresses)

int screen=-1;            // -1 OPTIONS, else screens[]
int cursor=0, top=0;      // selection and first visible row
bool owned=false;         // sub-screen 8 (or 9) is ours
// Actions for the next live frame (guest writes and sounds happen there).
int pending_sub=-1, pending_cursor=-1, pending_sound=-1;
int pending_speed=-1;     // D19N: GAME SPEED for 0x80089920 at the next tick

// D19H: the pause menu and death screen as lists. Page::Options is D19's
// OPTIONS (screen -1) and settings screens; the rest are gameplay pages.
// D19J: Main, Difficulty, TwoPlayer and Character are the title pages.
// D19G: Keys is KEY BINDINGS (from PC CONTROLS).
// D19K: Load is LOAD GAME's slot list, LoadAsk its LOAD THIS GAME?.
// D19Q: Save is SAVE GAME's slot list, SaveAsk its OVERWRITE THIS SAVE?.
// D19S: DeleteAsk is DELETE SAVED GAME: on either list.
// D20B: Stats is the statistics screen (game mode 2).
enum class Page {Options, Pause, Death, Brief, Ask, Main, Difficulty, TwoPlayer, Character, Keys, Load, LoadAsk, Save, SaveAsk, DeleteAsk, Stats};
enum class Act {Resume, Brief, Options, Save, Load, Restart, MainMenu, Continue, Quit, QuitGame, OnePlayer, TwoPlayer, Level, RestoreKeys};
struct ListItem {const char* label;Act act;};
bool title_page(Page p) {return p==Page::Main || p==Page::Difficulty || p==Page::TwoPlayer || p==Page::Character;}
// D19J: the title pages act in the frontend tick, on the originals' variables,
// in the order the keys came (a page change and the next key in one tick).
enum class Cmd {Up, Down, Left, Right, Confirm, Back};
std::vector<Cmd> title_cmds;
bool title_activity=false;    // a key this tick: the main menu's idle time starts again
bool title_enter=false;       // the main menu opens: state 6's idle time and level reset
// A frontend state for the next tick's entry (PRESS START 4, or a handoff): the image of this tick still shows our page, as the original
// handler draws its screen in the tick that chooses.
int pending_state=-1;
// Starting a game: the original handler (difficulty 8, character 9) runs
// with one Cross press and exits the frontend itself; it draws the same image.
enum class Handoff {None, Pending, Armed};
Handoff handoff=Handoff::None;
Page page=Page::Options;
std::vector<ListItem> list;   // the pause or death list on screen
Page ask_from=Page::Pause;    // a YES / NO prompt: the list it came from,
Act ask_act=Act::Resume;      // what YES does
Act return_item=Act::Resume;  // the item highlighted when the list comes back
// Every final action runs, in the tick, exactly what the original's handler
// does on that confirmation (its sound, flags and calls) and what the main
// loop then does with its result. The handler itself never runs: it would draw
// the old screen into the next image, which then shows for one frame.
enum class Final {None, Resume, PauseRestart, PauseMainMenu, DeathContinue, DeathRestart, DeathQuit};
Final pending_final=Final::None;
// After a resume the screen value stays ours through that dispatch and reads 0
// again at the next composition.
bool resumed=false;
bool quitting=false;          // D19M: the close event is sent; keys are ignored until the exit
const char* back_label="CANCEL";
bool identity_ok=false, identity_checked=false;
uint32_t last_image=~0u;
// D19O: the call site of the last draw. Where a demo scene's load hands over
// (the blocking load to the tick's logo, the logo to the demo), the guest
// builds the same image twice and shows the second pass, which covered the
// menu drawn in the first: about 100 ms without the menu each time. A new
// call site in the same image draws again.
uint32_t last_draw_ra=0;
uint64_t draws=0, entries=0, writes=0, finals=0, card_skips=0, card_hidden=0;
bool card_unpaced=false; // D19R: an unpaced window requested
// D19N: slider objects in guest RAM, the original SOUND screen's three
// (0x800dd290, +0x40, +0x80; the handler 0x8008aa08 initialises them each time
// it opens, and it never runs while our screens are up). Objects in the
// framework's GPU-DMA memory were not in the D17 frontend replay's copy of the
// machine: at a high frame rate a title screen with a slider in view showed
// nothing at all.
constexpr uint32_t sliders=0x800dd290u;
constexpr int slider_slots=3, slider_size=0x40;

// D19J: a savestate load drops what the menu knew of the guest before it; the
// screens reopen from the loaded state (our screens: OPTIONS, the lists, the
// title main menu).
bool key_wait=false; // D19G: KEY BINDINGS waits for the new key (input_capture_key)
uint32_t seen_loads=0;
bool loading=false;  // D19K: a slot load is requested; keys wait for it
// D19Q: a SAVE GAME save is requested (keys wait for it); its slot, and
// whether the framework reported it written (game_menu_saved).
bool saving=false, save_written=false;
int save_slot=-1;
// D19Q: the game as it was when the pause list opened (the displayed image
// then is the last gameplay image): a save from the pause menu gets it as its
// thumbnail instead of the menu.
uint32_t pause_thumb[SAVESTATE_THUMB_W*SAVESTATE_THUMB_H];
bool pause_thumb_ok=false;
// D19Q3: every successfully loaded paused gameplay save resumes once, regardless
// of load route or old sidecar metadata. Later manual pauses are independent.
bool resume_load=false, quiet_resume=false;
bool thumb_from_load=false; // pause_thumb is the loaded slot's picture
// D20: EDuke32's quick save and quick load. The slot last saved to or loaded
// from this session (any way: menus, F7, the quick keys); none at launch, as in
// EDuke32, so the first quick save opens SAVE GAME and quick load LOAD GAME.
int quick_slot=-1;
// A quick save or load under way: its notice comes once the framework is done.
bool quick_saving=false, quick_saved=false, quick_loading=false;
int quick_save_slot=-1;
uint32_t quick_loading_from=0;
// F2 / F3 (or a quick key with no slot yet): the pause menu about to open goes
// straight to SAVE GAME or LOAD GAME (direct, until direct_until), and Escape
// there returns to play (direct_open), as EDuke32's menus opened from play do.
Page direct=Page::Options;
uint64_t direct_until=0;
bool direct_open=false;
// D20B: our statistics screen is up (its page, or SAVE GAME opened from it);
// fields until its prompts show; CONFIRM waits for the next flip; a savestate
// was loaded since it last opened (it then opens with its prompts).
bool stats_open=false, stats_continue=false, stats_after_load=false;
// D20D: the best time for this level and difficulty (ms, 0 = none) and
// whether this level just set it.
uint32_t stats_best_ms=0;bool stats_new_best=false;
int stats_wait=0;
void thumb_hide();
void savestate_check() {
    const uint32_t n=psx_mod_savestate_loads();
    if(n==seen_loads) return;
    seen_loads=n;
    {   // D20: the slot just loaded (LOAD GAME, F7 or quick load).
        char status[192];savestate_status_json(status,sizeof status);
        const char* at=std::strstr(status,"\"last_slot\":");
        if(std::strstr(status,"\"last_op\":\"load\"") && at) {
            const int slot=std::atoi(at+12);
            if(slot>=0 && slot<SAVESTATE_SLOTS) quick_slot=slot;
        }
    }
    direct=Page::Options;direct_open=false;
    owned=false;page=Page::Options;screen=-1;pending_sub=pending_cursor=-1;
    pending_final=Final::None;resumed=false;
    resume_load=psx_mod_read_half(game_mode)==1 && psx_mod_read_half(paused)!=0;
    quiet_resume=false;
    pending_state=-1;handoff=Handoff::None;title_cmds.clear();title_enter=false;
    if(key_wait) {key_wait=false;input_capture_key(false);}
    loading=false;saving=false;thumb_hide();
    // D20B: a state loaded on the statistics screen opens it with its prompts.
    stats_open=false;stats_continue=false;stats_after_load=psx_mod_read_half(game_mode)==2;
}

const char* intern(const char* text) {owned_strings.emplace_back(text);return owned_strings.back().c_str();}

int choice_from_env(Row& row,const char* value) {
    if(!value) return row.def;
    for(size_t i=0;i<row.choices.size();++i) if(!std::strcmp(row.choices[i].env,value)) return (int)i;
    // A value the list does not offer (an Alt-wheel camera distance): keep it as a choice.
    const char* v=intern(value);
    row.choices.push_back({v,v,v});
    return (int)row.choices.size()-1;
}
const char* presentation_value(const char* key) {
    static std::string text=[]{const char* t=std::getenv("DNTTK_MENU_PRESENTATION");return std::string(t?t:"");}();
    const std::string want=std::string(key)+"=";
    size_t at=0;
    while(at<text.size()) {
        size_t end=text.find(',',at);if(end==std::string::npos) end=text.size();
        if(text.compare(at,want.size(),want)==0) return intern(text.substr(at+want.size(),end-at-want.size()).c_str());
        at=end+1;
    }
    return nullptr;
}
void init() {
    screens=make_screens();
    for(auto& s:screens) {
        Values v;
        for(size_t i=0;i<s.rows.size();++i) {
            Row& r=s.rows[i];
            const char* env=r.link ? nullptr : r.section==Section::Presentation ? presentation_value(r.key) : r.env ? std::getenv(r.env) : nullptr;
            if(r.slider) {
                int value=r.def;
                if(env) {const double d=std::strtod(env,nullptr)/r.scale;value=std::clamp((int)(d+0.5),r.lo,r.hi);}
                v.push_back(value);
            } else v.push_back(choice_from_env(r,env));
        }
        committed.push_back(v);
    }
    draft=committed;
}
void ensure_init() {std::call_once(init_once,init);}

bool authentic() {
    if(!identity_checked) {identity_checked=true;identity_ok=code_identity(guards,expected);
        if(!identity_ok) std::fprintf(stderr,"[TTK menu] executable identity mismatch: original OPTIONS kept\n");}
    return identity_ok;
}
bool enabled() {
    static const bool v=[]{const char* t=std::getenv("DNTTK_GAME_MENU");return !(t && !std::strcmp(t,"off"));}();
    return v && input_modernized();
}

// ---- D19G: key names ----
const char* const action_names[action_count]={
#define TTK_ACTION(name, token, code, pad) #name,
#include "input_bindings.def"
#undef TTK_ACTION
};
struct KeyName {int code;const char* token;const char* shown;};
// The profile's tokens (tools/local/pc_input.py TOKENS) for the codes that are
// not a plain letter or digit, and how the menu shows them (menu font: capitals,
// digits and a little punctuation, so no brackets).
const KeyName key_names[]={
    {0,"Unbound","NONE"},{44,"Space","SPACE"},{224,"LCtrl","LEFT CTRL"},{225,"LShift","SHIFT"},{226,"Alt","ALT"},
    {71,"ScrollLock","SCROLL LOCK"},{47,"LBracket","["},{48,"RBracket","]"},
    {74,"Home","HOME"},{75,"PageUp","PAGE UP"},{77,"End","END"},{78,"PageDown","PAGE DOWN"},
    {80,"Left","LEFT ARROW"},{79,"Right","RIGHT ARROW"},
    {61,"F4","F4"},{62,"F5","F5"},{63,"F6","F6"},{66,"F9","F9"},{69,"F12","F12"},
    {51,"Semicolon","SEMICOLON"},{52,"Apostrophe","APOSTROPHE"},{54,"Comma","COMMA"},{55,"Period","PERIOD"},
    {-1,"Mouse1","LEFT MOUSE"},{-3,"Mouse2","RIGHT MOUSE"},{-2,"Mouse3","MIDDLE MOUSE"},{-4,"Mouse4","MOUSE 4"},{-5,"Mouse5","MOUSE 5"},
};
std::string token_of(int code) {
    if(code>=SDL_SCANCODE_A && code<=SDL_SCANCODE_Z) return std::string(1,char('A'+code-SDL_SCANCODE_A));
    if(code>=SDL_SCANCODE_1 && code<=SDL_SCANCODE_9) return std::string(1,char('1'+code-SDL_SCANCODE_1));
    if(code==SDL_SCANCODE_0) return "0";
    for(const KeyName& k:key_names) if(k.code==code) return k.token;
    return "Unbound";
}
std::string key_shown(int code) {
    for(const KeyName& k:key_names) if(k.code==code) return k.shown;
    return token_of(code);
}
// In sentences (the text call has no bracket glyph): the brackets by name.
std::string key_said(int code) {
    if(code==47) return "LEFT BRACKET";
    if(code==48) return "RIGHT BRACKET";
    return key_shown(code);
}

// The confirmed settings, for run.py (DNTTK_MENU_SETTINGS_FILE).
void write_settings() {
    const char* path=std::getenv("DNTTK_MENU_SETTINGS_FILE");
    if(!path || !*path) return;
    // Vanilla is a launch flag (run.py --vanilla, D33); the side file keeps its
    // version 1 "active" field, always Modernized.
    std::string controls, presentation, bindings;
    for(size_t s=0;s<screens.size();++s) for(size_t i=0;i<screens[s].rows.size();++i) {
        const Row& r=screens[s].rows[i];const int v=committed[s][i];
        if(r.link || r.heading || r.section==Section::Game) continue;
        char value[64];
        if(r.slider) std::snprintf(value,sizeof value,"%.2f",v*r.scale);
        else std::snprintf(value,sizeof value,"%s",r.choices[v].json);
        std::string& out=r.section==Section::Controls ? controls : presentation;
        out+=std::string(out.empty()?"":",")+"\""+r.key+"\":"+value;
    }
    // D19G: the bindings in the profile's terms (run.py validates them).
    for(int a=0;a<action_count;++a)
        bindings+=std::string(a?",":"")+"\""+action_names[a]+"\":\""+token_of(input_binding_code(Action(a)))+"\"";
    const std::string text="{\"version\":1,\"active\":\"modernized\",\"controls\":{"+controls+"},\"presentation\":{"+presentation+"},\"bindings\":{"+bindings+"}}\n";
    const std::string temp=std::string(path)+".tmp";
    if(FILE* f=std::fopen(temp.c_str(),"wb")) {
        const bool ok=std::fwrite(text.data(),1,text.size(),f)==text.size();
        if(std::fclose(f)==0 && ok && std::rename(temp.c_str(),path)==0) {++writes;return;}
    }
    std::remove(temp.c_str());
    std::fprintf(stderr,"[TTK menu] could not write %s\n",path);
}

// ---- input (from input_pad) ----
struct Repeat {bool held=false;uint64_t next=0,since=0;};
Repeat rep_up,rep_down,rep_left,rep_right;
// A press, then 350 ms, then every 80 ms while held (D19B: predictable repeat).
// A press (a key-down event since the last read, so a tap shorter than the
// gap between two reads still counts and a key held when the screen opened
// does not), then 350 ms, then every 80 ms while held (D19B: predictable repeat).
int repeat(Repeat& r,bool held,int taps,uint64_t now) {
    if(taps>0) {r.held=held;r.since=now;r.next=now+350;return taps;}
    if(!held || !r.held) {r.held=false;return 0;}
    if(now<r.next) return 0;
    // D19O: after a gap with no read the steps go on from now; replaying the
    // missed ones ran the cursor through the list in a burst.
    r.next=now>r.next+80 ? now+80 : r.next+80;
    return 1;
}
// More than five rows: the compact layout (14 apart), nine at a time.
bool compact(const Screen& s) {return s.rows.size()>5;}
int rows_visible(const Screen& s) {return compact(s) ? 9 : (int)s.rows.size();}
// D19N: the cursor's row visible, with its section heading when that is just
// above (as KEY BINDINGS scrolls).
// In the compact layout a slider takes two row places: its disc is taller
// than the 14 between rows and overlapped the row above.
int row_units(const Screen& s,int i) {return compact(s) && s.rows[i].slider ? 2 : 1;}
int units_between(const Screen& s,int from,int to) {int u=0;for(int i=from;i<=to;++i) u+=row_units(s,i);return u;}
void settings_scroll(const Screen& s) {
    const int vis=rows_visible(s);
    if(cursor<top) top=cursor;
    while(top<cursor && units_between(s,top,cursor)>vis) ++top;
    if(top==cursor && top>0 && s.rows[top-1].heading) --top;
}
int first_row(const Screen& s) {int i=0;while(i<(int)s.rows.size()-1 && s.rows[i].heading) ++i;return i;}
void sound(int which) {if(pending_sound<0 || which==0) pending_sound=which;}
// ---- D19H: the pause and death lists ----
int level() {return (int)psx_mod_read_word(level_index);}
bool challenge() {return (unsigned)(level()-21)<6u;} // stages 21-26: no RESTART, REPLAY / GIVE UP
bool has_briefing() {
    const int n=level();
    if(n<0 || n>=13) return false;
    for(int i=0;i<10;++i) if(psx_mod_read_half(briefings+20*n+2*i)) return true;
    return false;
}
// The original's choices, in the original's terms: the pause menu's RESUME,
// OPTIONS, LOAD (one player only) and its QUIT prompt's RESTART (not in the
// challenge stages) and MAIN MENU; the death screen's CONTINUE, LOAD, RESTART
// and QUIT, or REPLAY LEVEL and GIVE UP in the challenge stages.
void build_list(Page p) {
    list.clear();
    if(p==Page::Main) {
        // D19J: the original main menu's items (its fifth, the level, only
        // while the original's debug level choice is on: 0x800d2528 = 5).
        list.push_back({"ONE PLAYER",Act::OnePlayer});
        list.push_back({"TWO PLAYER",Act::TwoPlayer});
        list.push_back({"LOAD GAME",Act::Load});
        list.push_back({"OPTIONS",Act::Options});
        if((int16_t)psx_mod_read_half(fe_items)>4) list.push_back({"LEVEL",Act::Level});
    } else if(p==Page::Pause) {
        list.push_back({"RESUME",Act::Resume});
        if(has_briefing()) list.push_back({"MISSION BRIEFING",Act::Brief});
        list.push_back({"OPTIONS",Act::Options});
        if(psx_mod_read_word(players)==1) {
            list.push_back({"SAVE GAME",Act::Save}); // D19Q
            list.push_back({"LOAD GAME",Act::Load});
        }
        if(!challenge()) list.push_back({"RESTART LEVEL",Act::Restart});
        list.push_back({"MAIN MENU",Act::MainMenu});
    } else if(challenge()) {
        list.push_back({"REPLAY LEVEL",Act::Continue});
        list.push_back({"GIVE UP",Act::Quit});
    } else {
        list.push_back({"CONTINUE",Act::Continue});
        list.push_back({"LOAD GAME",Act::Load});
        list.push_back({"RESTART LEVEL",Act::Restart});
        list.push_back({"QUIT TO MENU",Act::Quit}); // D19P: QUIT, beside QUIT GAME
    }
    list.push_back({"QUIT GAME",Act::QuitGame}); // D19M: closes the program
}
int item_of(Act a) {
    for(size_t i=0;i<list.size();++i) if(list[i].act==a) return (int)i;
    return 0;
}
void open_list(Page p,Act highlight) {
    page=p;build_list(p);cursor=item_of(highlight);top=0;screen=-1;direct_open=false;
}
// ---- D19G: KEY BINDINGS ----
// Every action, grouped; a row without an action is a section heading (the
// cursor skips it). Changes apply and are saved at once; DEFAULT asks first.
// fixed: a key that always does it too (not rebindable), shown first.
// hint: one line about the action, above the CONFIRM line.
struct KeyRow {const char* label;int action;const char* fixed=nullptr;const char* hint=nullptr;};
const KeyRow key_rows[]={
    {"MOVEMENT",-1},{"MOVE FORWARD",move_forward},{"MOVE BACK",move_back},{"STRAFE LEFT",move_left},{"STRAFE RIGHT",move_right},
    {"JUMP",jump},{"CROUCH",crouch},{"WALK / RUN",walk},{"USE / MANTLE",interact},
    {"GRAB",grab,nullptr,"HOLD TO GRAB, PUSH AND PULL"},{"GRAB 2",grab_alt,nullptr,"A SECOND GRAB KEY"},
    // The weapon keys select a group; each is named after its first weapon.
    {"WEAPONS",-1},{"FIRE",fire},{"QUICK KICK",quick_kick},{"HOLSTER",holster},{"PREVIOUS WEAPON",weapon_previous},
    {"NEXT WEAPON",weapon_next},{"LAST WEAPON",weapon_last},
    {"MIGHTY BOOT",weapon_group_1,nullptr,"ALSO THROWING KNIFE AND AXE"},{"DESERT EAGLE",weapon_group_2},
    {"COMBAT SHOTGUN",weapon_group_3},{"GATLING GUN",weapon_group_4},{"RPG",weapon_group_5},
    {"PIPE BOMB",weapon_group_6,nullptr,"ALSO DYNAMITE AND HOLY HAND GRENADE"},{"ENERGY WEAPON",weapon_group_7},
    {"FLAMETHROWER",weapon_group_8},{"BUFFALO RIFLE",weapon_group_9,nullptr,"ALSO THE CROSSBOW"},{"FREEZER",weapon_group_0},
    // inventory's key toggles the crosshair (the inventory screen is Right Shift).
    {"ITEMS",-1},{"CROSSHAIR",inventory,nullptr,"SHOWS OR HIDES THE CROSSHAIR"},{"PREVIOUS ITEM",item_previous},{"NEXT ITEM",item_next},
    {"USE ITEM",item_use,"ENTER","ENTER USES IT. A SECOND KEY IS OPTIONAL"},
    {"MEDKIT",medkit},{"STEROIDS",steroids},{"JETPACK",jetpack},{"GOGGLES",goggles},{"BIO MASK",biomask},
    {"PREVIOUS MISSION ITEM",mission_previous},{"NEXT MISSION ITEM",mission_next},
    {"CAMERA",-1},{"FIRST / THIRD PERSON",camera_view},{"SHOULDER",camera_shoulder},{"RECENTER CAMERA",camera_recenter},
    {"LOOK UP",look_up,nullptr,"HOLD, EASES BACK ON RELEASE"},{"LOOK DOWN",look_down,nullptr,"HOLD, EASES BACK ON RELEASE"},
    {"LOCK LOOK UP",lock_look_up,nullptr,"HOLD, STAYS UNTIL A LOOK KEY"},{"LOCK LOOK DOWN",lock_look_down,nullptr,"HOLD, STAYS UNTIL A LOOK KEY"},
    {"TURN LEFT",turn_left,nullptr,"HOLD, SPEEDS UP, FASTER RUNNING"},{"TURN RIGHT",turn_right,nullptr,"HOLD, SPEEDS UP, FASTER RUNNING"},
    // D20: EDuke32's quick keys (F2 / F3 always open SAVE / LOAD GAME).
    {"SAVES",-1},{"QUICK SAVE",quick_save,nullptr,"SAVES OVER THE LAST SLOT USED"},
    {"QUICK LOAD",quick_load,nullptr,"LOADS THE LAST SLOT USED"},
    {"ORIGINAL CONTROLS",-1},{"ORIGINAL AIM",original_aim},{"ORIGINAL STRAFE LEFT",original_strafe_left},
    {"ORIGINAL STRAFE RIGHT",original_strafe_right},
};
constexpr int key_row_count=sizeof key_rows/sizeof key_rows[0], key_rows_visible=9;
static_assert(key_row_count==action_count+6,"every action has one KEY BINDINGS row");
std::string key_note;         // the description's message after a change
int keys_cursor=1, keys_top=0; // kept while DEFAULT asks
const char* action_label(int a) {
    for(const KeyRow& k:key_rows) if(k.action==a) return k.label;
    return "";
}
void key_wait_end() {if(key_wait) {key_wait=false;input_capture_key(false);}}
// The cursor's row visible, with its section heading when that is just above.
void keys_scroll() {
    if(cursor<top) top=cursor;
    if(cursor>=top+key_rows_visible) top=cursor-key_rows_visible+1;
    if(top==cursor && top>0 && key_rows[top-1].action<0) --top;
}
void open_keys() {page=Page::Keys;cursor=1;top=0;key_note.clear();key_wait_end();sound(0);}
void keys_move(int dir) {
    do cursor=(cursor+dir+key_row_count)%key_row_count; while(key_rows[cursor].action<0);
    keys_scroll();key_note.clear();sound(1);
}
void keys_back() {
    key_wait_end();key_note.clear();
    page=Page::Options;screen=0;top=0;cursor=0;
    for(size_t i=0;i<screens[0].rows.size();++i) if(screens[0].rows[i].link) cursor=(int)i;
    sound(0);
}
void keys_ask_default() {
    keys_cursor=cursor;keys_top=top;key_note.clear();
    ask_from=Page::Keys;ask_act=Act::RestoreKeys;page=Page::Ask;cursor=1;sound(1);
}
void keys_return() {page=Page::Keys;cursor=keys_cursor;top=keys_top;}
void restore_keys() {
    int codes[action_count];
    for(int a=0;a<action_count;++a) codes[a]=input_default_code(Action(a));
    input_set_binding_codes(codes);
    write_settings();
    keys_return();key_note="DEFAULT KEYS RESTORED";sound(0);
}
// The key pressed while waiting: Escape cancels, Backspace clears; a key
// another action has swaps with it (that action takes this one's old key).
void key_pressed(int code) {
    const Action a=Action(key_rows[cursor].action);
    if(code==SDL_SCANCODE_ESCAPE) {key_wait_end();sound(0);return;}
    if(code==SDL_SCANCODE_BACKSPACE) code=0;
    else if(code==SDL_SCANCODE_RSHIFT) code=SDL_SCANCODE_LSHIFT;  // either Shift, as in play
    else if(code==SDL_SCANCODE_RALT) code=SDL_SCANCODE_LALT;      // either Alt
    if(code && !input_code_bindable(code,a)) {
        key_note=code==SDL_SCANCODE_C ? "C IS THE HOLSTER KEY ONLY. PRESS ANOTHER" : "THAT KEY IS RESERVED. PRESS ANOTHER";
        sound(1);return;
    }
    int codes[action_count];
    for(int i=0;i<action_count;++i) codes[i]=input_binding_code(Action(i));
    const int old=codes[a];
    key_note.clear();
    if(code!=old) {
        for(int b=0;code && b<action_count;++b) if(b!=a && codes[b]==code) {
            codes[b]=old==SDL_SCANCODE_C && b!=holster ? 0 : old; // C stays the holster's
            key_note=key_said(code)+" WAS "+action_label(b)+": "+(codes[b] ? "THAT IS ON "+key_said(codes[b])+" NOW" : "THAT HAS NO KEY NOW");
        }
        codes[a]=code;
        input_set_binding_codes(codes);
        write_settings();
    }
    key_wait_end();sound(0);
}
// ---- D19K: LOAD GAME from the savestate slots ----
struct SlotInfo {bool exists=false,compatible=false,named=false,complete=false;unsigned level=0;int64_t stamp=0;};
SlotInfo slot_info[SAVESTATE_SLOTS];
constexpr int load_rows_visible=9;
Page load_from=Page::Main;   // the list LOAD GAME was chosen on
uint32_t loading_from=0;     // psx_mod_savestate_loads() when the load was requested
std::string load_note;       // a refused or failed load
bool loadable(int slot) {return slot_info[slot].exists && slot_info[slot].compatible;}
void read_slots() {
    for(int i=0;i<SAVESTATE_SLOTS;++i) {
        SlotInfo& s=slot_info[i];
        s=SlotInfo{};
        s.exists=savestate_slot_exists(i);
        if(!s.exists) continue;
        s.compatible=savestate_slot_compatible(i,nullptr,0);
        s.named=savestate_slot_level(i,s.level);
        s.complete=s.named && savestate_slot_complete(i); // D20B
        if(!savestate_slot_mtime(i,&s.stamp)) s.stamp=0;
    }
}
void load_scroll() {
    if(cursor<top) top=cursor;
    if(cursor>=top+load_rows_visible) top=cursor-load_rows_visible+1;
}
// The list opens on slot 1 (user, 2026-10-20).
void open_load(Page from) {
    load_from=from;read_slots();thumb_hide(); // a slot saved again since: its new picture
    page=Page::Load;cursor=0;top=0;load_note.clear();
}
void act_now(Act a);
void load_back() {
    thumb_hide();load_note.clear();
    if(direct_open) {act_now(Act::Resume);return;} // D20: F3 / F9 from play: back to play
    // On the title as LOAD's CANCEL returns there (state 6: idle time, level choice).
    open_list(load_from,Act::Load);
    if(load_from==Page::Main) title_enter=true;
    sound(0);
}
void load_move(int dir) {
    cursor=(cursor+dir+SAVESTATE_SLOTS)%SAVESTATE_SLOTS;
    load_scroll();load_note.clear();sound(1);
}
// The load itself is F7's: the framework restores the slot at its next safe
// point (never inside one of our guest calls) and the menu starts again from
// the loaded state (savestate_check).
void load_now() {
    if(savestate_request_load(cursor)) {
        loading=true;loading_from=psx_mod_savestate_loads();sound(0);
        // Resume policy is set from the successfully applied load generation.
        // A paused save that does not resume keeps its own picture for SAVE GAME.
        pause_thumb_ok=thumb_from_load=savestate_read_thumb(cursor,pause_thumb,SAVESTATE_THUMB_W,SAVESTATE_THUMB_H)!=0;
        return;
    }
    page=Page::Load;load_note="THIS SAVE COULD NOT BE LOADED";sound(1);
}

// ---- D19Q: SAVE GAME to the savestate slots ----
// The page is LOAD GAME's (its slots, thumbnail and details) with every slot
// open: an empty one saves at once, a used one asks OVERWRITE THIS SAVE? (NO
// first, as the other prompts that lose something). The save is F7's: the
// framework writes the paused game at its next safe point; the page then says
// SAVED or that it could not.
int save_choice=1;  // SaveAsk: 0 YES, 1 NO
void open_save() {
    read_slots();thumb_hide();
    page=Page::Save;cursor=0;top=0;load_note.clear();
}
void save_now() {
    page=Page::Save;load_note.clear();
    if(savestate_request_save(cursor)) {saving=true;save_written=false;save_slot=cursor;sound(0);return;}
    load_note="THE GAME COULD NOT BE SAVED";sound(1);
}
void save_done() {
    saving=false;read_slots();thumb_hide();page=Page::Save;
    // D20: SAVE GAME opened from play (F2, or F6 with no slot yet) closes on a
    // save, as EDuke32's does.
    if(save_written && direct_open) {input_notice("GAME SAVED");load_note.clear();act_now(Act::Resume);return;}
    // D20B: from the statistics screen, back to it.
    if(save_written && stats_open) {input_notice("GAME SAVED");load_note.clear();page=Page::Stats;cursor=1;sound(0);return;}
    if(save_written) {char t[32];std::snprintf(t,sizeof t,"SAVED TO SLOT %02d",save_slot+1);load_note=t;sound(0);}
    else {load_note="THE GAME COULD NOT BE SAVED";sound(1);}
}
void save_confirm() {
    if(!slot_info[cursor].exists) {save_now();return;}
    page=Page::SaveAsk;save_choice=1;load_note.clear();sound(0);
}

// ---- D20B: the statistics screen ----
// The original's choices once its prompts show: CONTINUE (Cross), and SAVE
// (Square) on the statistics page before the last level's ending; after a
// memory card save (page 2, a Vanilla state) CONTINUE only. SAVE is our SAVE
// GAME; CONTINUE is handed to the original at the next flip.
// D20F: a list, CONTINUE then SAVE GAME (cursor 0 / 1); V stays a shortcut.
bool stats_can_save() {return psx_mod_read_half(stats_page)==0 && level()<29;}
int stats_items() {return stats_can_save() ? 2 : 1;}
void stats_confirm() {if(stats_wait<=0) stats_continue=true;}
void stats_save() {if(stats_wait<=0 && stats_can_save()) {open_save();sound(1);}}
void stats_choose() {if(cursor==1 && stats_can_save()) stats_save(); else stats_confirm();}
void stats_move(int dir) {
    if(stats_wait>0 || stats_items()<2) return;
    cursor=(cursor+dir+2)%2;sound(1);
}

// ---- D19S: Delete on a used slot of LOAD GAME or SAVE GAME ----
// DELETE SAVED GAME: and the slot's name, YES / NO with NO first. YES removes
// that slot's file, thumbnail and sidecar (never a card or another slot) and
// the list shows it EMPTY; a delete that fails leaves the slot as it was.
// Saves from another build can be deleted too (the only way to clear them).
Page delete_from=Page::Load;
int delete_choice=1;  // DeleteAsk: 0 YES, 1 NO
void delete_ask() {
    if((page!=Page::Load && page!=Page::Save) || !slot_info[cursor].exists) return;
    delete_from=page;page=Page::DeleteAsk;delete_choice=1;load_note.clear();sound(0);
}
void delete_now() {
    page=delete_from;
    char t[40];
    if(!savestate_pending() && savestate_slot_delete(cursor)) {std::snprintf(t,sizeof t,"DELETED SLOT %02d",cursor+1);sound(0);}
    else {std::snprintf(t,sizeof t,"SLOT %02d COULD NOT BE DELETED",cursor+1);sound(1);}
    read_slots();thumb_hide();load_note=t;
}

// ---- D20: quick save and quick load (EDuke32's F6 / F9) ----
// Duke is dead (player word 0 bit 1, health 0; see steroids.inc).
bool duke_dead() {return (psx_mod_read_word(player)&2) && (int16_t)psx_mod_read_half(player+0x32)<=0;}
// The notices once the framework has taken a quick save or load.
void quick_poll() {
    if(quick_saving && !savestate_pending()) {
        quick_saving=false;
        input_notice(quick_saved ? "GAME SAVED" : "THE GAME COULD NOT BE SAVED");
    }
    if(quick_loading && !savestate_pending()) {
        quick_loading=false;
        if(psx_mod_savestate_loads()==quick_loading_from) {
            resume_load=false;thumb_from_load=false;
            input_notice("THIS SAVE COULD NOT BE LOADED");
        }
    }
    if(direct!=Page::Options && SDL_GetTicks()>direct_until) direct=Page::Options;
}
// The pause menu about to open goes to SAVE GAME or LOAD GAME.
QuickResult quick_open(Page p) {direct=p;direct_until=SDL_GetTicks()+1500;return QuickResult::Pause;}

void ask_cancel() {
    if(ask_from==Page::Keys) keys_return();
    else open_list(ask_from,ask_act);
    sound(1);
}

void act_now(Act a) {
    const bool death=page==Page::Death || (page==Page::Ask && ask_from==Page::Death);
    switch(a) {
    case Act::Resume: pending_final=Final::Resume;break;
    case Act::Restart: pending_final=death ? Final::DeathRestart : Final::PauseRestart;break;
    case Act::MainMenu: pending_final=Final::PauseMainMenu;break;
    case Act::Continue: pending_final=Final::DeathContinue;break;
    case Act::Quit: pending_final=Final::DeathQuit;break;
    case Act::RestoreKeys: restore_keys();break;
    case Act::QuitGame: {
        // The window's close event: the main loop's own shutdown (settings
        // were written on CONFIRM; run.py folds them in when the game exits).
        quitting=true;sound(0);
        SDL_Event quit{};quit.type=SDL_QUIT;
        SDL_PushEvent(&quit);
        break;
    }
    default: break;
    }
}
// The original's prompt for each choice: CONTINUE (REPLAY) is confirmed as
// the original asks (YES first); RESTART, MAIN MENU, QUIT and QUIT GAME
// start on NO. QUIT GAME? and QUIT TO MENU? have no text id: -1 and -2.
int ask_heading() {
    switch(ask_act) {
    case Act::Continue: return challenge() ? text_replay : text_continue;
    case Act::Restart: return text_restart;
    case Act::QuitGame: return -1;
    case Act::Quit: return challenge() ? text_quit : -2; // D19P: GIVE UP keeps REALLY QUIT?
    case Act::RestoreKeys: return -3;
    default: return text_quit;
    }
}
void list_confirm() {
    const Act a=list[cursor].act;
    return_item=a;
    switch(a) {
    case Act::Brief: page=Page::Brief;sound(0);break;
    case Act::Options: page=Page::Options;screen=-1;cursor=0;top=0;back_label=contexts[1].back_label;sound(0);break;
    case Act::Load: open_load(page);sound(0);break;
    case Act::Save: open_save();sound(0);break;
    case Act::Resume: act_now(a);break;
    default: ask_from=page;ask_act=a;page=Page::Ask;cursor=a==Act::Continue ? 0 : 1;sound(1);break;
    }
}

void move(int dir) {
    if(title_page(page)) {title_cmds.push_back(dir<0 ? Cmd::Up : Cmd::Down);return;}
    if(page==Page::Load) {load_move(dir);return;}
    if(page==Page::LoadAsk) return;
    if(page==Page::Save) {load_move(dir);return;}
    if(page==Page::SaveAsk) {save_choice^=1;sound(1);return;}
    if(page==Page::DeleteAsk) {delete_choice^=1;sound(1);return;}
    if(page==Page::Keys) {if(!key_wait) keys_move(dir);return;}
    if(page==Page::Stats) {stats_move(dir);return;}
    if(page==Page::Brief) return;
    if(page!=Page::Options) {
        const int n=page==Page::Ask ? 2 : (int)list.size();
        if(n) cursor=(cursor+dir+n)%n;
        sound(1);
        return;
    }
    const int n=screen<0 ? option_count : (int)screens[screen].rows.size();
    cursor=(cursor+dir+n)%n;
    if(screen>=0) {
        while(screens[screen].rows[cursor].heading) cursor=(cursor+dir+n)%n;
        settings_scroll(screens[screen]);
    }
    sound(1);
}
void change(int dir,uint64_t held_ms) {
    if(title_page(page)) {title_cmds.push_back(dir<0 ? Cmd::Left : Cmd::Right);return;}
    if(page!=Page::Options || screen<0) return;
    Row& r=screens[screen].rows[cursor];
    int& v=draft[screen][cursor];
    const int before=v;
    if(r.slider) v=std::clamp(v+dir*r.step*(held_ms>1200?5:1),r.lo,r.hi);
    else {const int n=(int)r.choices.size();v=(v+dir+n)%n;}
    if(v!=before) sound(1);
}
// D19G: rows that apply when confirmed. D19G1: FRAME RATE wherever replay
// presentation can run (OpenGL), from any launch rate.
bool row_live(const Row& r) {
    if(!r.live) return false;
    if(r.key && !std::strcmp(r.key,"frame_rate")) return frame_rate_live();
    return true;
}
int row_of(int s,const char* key) {
    for(size_t i=0;i<screens[s].rows.size();++i) if(screens[s].rows[i].key && !std::strcmp(screens[s].rows[i].key,key)) return (int)i;
    return -1;
}
// A confirmed value, now. Settings read through ttk_live_env() take the new
// value; view, shoulder, distance and the display go to their owners.
void apply_live(const Row& r,int v) {
    const char* value;
    if(r.slider) {char t[32];std::snprintf(t,sizeof t,"%.2f",v*r.scale);value=intern(t);}
    else value=r.choices[v].env;
    const std::string key=r.key;
    if(r.section==Section::Game) {
        // D19N: the original's own values, as its GAME SETTINGS CONFIRM leaves them.
        const uint16_t w=psx_mod_read_half(settings_word);
        if(key=="gore") psx_mod_write_half(settings_word,(uint16_t)((w&~gore_bits)|((v&3)<<1)));
        else if(key=="status_bars") psx_mod_write_half(settings_word,(uint16_t)(r.choices[v].env[0]=='1' ? w|status_bars_bit : w&~status_bars_bit));
        else if(key=="game_speed") {psx_mod_write_half(game_speed,(uint16_t)v);pending_speed=v;}
        return;
    }
    if(key=="frame_rate" && !frame_rate_set(value)) return;
    // Until a change ttk_live_env() is plain getenv (an unset variable stays unset).
    if(r.env) ttk_live_env_set(r.env,value);
    if(key=="view") input_set_view(!std::strcmp(value,"first"));
    else if(key=="shoulder") input_set_shoulder(!std::strcmp(value,"right") ? 1 : !std::strcmp(value,"left") ? -1 : 0);
    else if(key=="camera_distance") camera_set_distance(std::atof(value));
    else if(key=="display") psx_mod_request_fullscreen_mode(!std::strcmp(value,"exclusive") ? 2 : !std::strcmp(value,"borderless") ? 1 : 0);
    else if(key=="widescreen") {
        unsigned num=4,den=3;
        if(!std::strcmp(value,"auto")) psx_mod_request_display_aspect(21,9,1);
        else {if(std::strcmp(value,"off")) std::sscanf(value,"%u:%u",&num,&den);psx_mod_request_display_aspect(num,den,0);}
    }
}
// P, H and Alt + wheel change view, shoulder and distance in game: the
// MODERNIZED screen opens on what is live now.
void refresh_from_game(int s) {
    auto set=[&](const char* key,const char* value) {
        const int i=row_of(s,key);
        if(i>=0) committed[s][i]=choice_from_env(screens[s].rows[i],value);
    };
    // D19N: GORE, STATUS BARS and GAME SPEED as the game has them now (a
    // memory card may have loaded others).
    const uint16_t w=psx_mod_read_half(settings_word);
    for(size_t i=0;i<screens[s].rows.size();++i) {
        const Row& r=screens[s].rows[i];
        if(r.section!=Section::Game) continue;
        const std::string key=r.key;
        if(key=="gore") committed[s][i]=std::min((w&gore_bits)>>1,2);
        else if(key=="status_bars") committed[s][i]=(w&status_bars_bit) ? 0 : 1;
        else if(key=="game_speed") committed[s][i]=std::clamp<int>(psx_mod_read_half(game_speed),r.lo,r.hi);
    }
    set("view",input_first_person() ? "first" : "third");
    const int side=input_shoulder();
    set("shoulder",side>0 ? "right" : side<0 ? "left" : "center");
    char d[32];std::snprintf(d,sizeof d,"%.0f",camera_distance_setting());
    if(row_of(s,"camera_distance")>=0) {
        Row& r=screens[s].rows[row_of(s,"camera_distance")];
        const char* match=nullptr;
        for(const Choice& c:r.choices) if(!std::strcmp(c.env,d)) match=c.env;
        set("camera_distance",match ? match : intern(d));
    }
}
void open_screen(int s) {
    refresh_from_game(s);
    screen=s;cursor=first_row(screens[s]);top=0;draft[s]=committed[s];sound(0);
}
void leave_screen(bool keep) {
    const int s=screen;
    if(keep && draft[s]!=committed[s]) {
        for(size_t i=0;i<screens[s].rows.size();++i) {
            const Row& r=screens[s].rows[i];
            if(draft[s][i]!=committed[s][i] && !r.link && !r.heading && row_live(r)) apply_live(r,draft[s][i]);
        }
        committed[s]=draft[s];
        write_settings();
    }
    draft[s]=committed[s];
    screen=-1;cursor=0;top=0;sound(0);
    for(int i=0;i<option_count;++i) if(options_items[i].screen==s) cursor=i;
}
void defaults() {
    if(page==Page::Stats) {stats_save();return;} // D20B: Square is SAVE
    if(page==Page::Keys) {if(!key_wait) keys_ask_default();return;}
    if(page!=Page::Options || screen<0) return;
    for(size_t i=0;i<screens[screen].rows.size();++i) draft[screen][i]=screens[screen].rows[i].def;
    sound(0);
}
void confirm() {
    switch(page) {
    case Page::Pause: case Page::Death: list_confirm();return;
    case Page::Brief: return;
    case Page::Ask:
        if(cursor==0) act_now(ask_act);
        else ask_cancel();
        return;
    case Page::Keys: if(!key_wait) {key_wait=true;key_note.clear();input_capture_key(true);sound(0);} return;
    case Page::Load: if(loadable(cursor)) {page=Page::LoadAsk;load_note.clear();sound(0);} return;
    case Page::LoadAsk: load_now();return;
    case Page::Stats: stats_choose();return;
    case Page::Save: save_confirm();return;
    case Page::SaveAsk: if(save_choice==0) save_now(); else {page=Page::Save;sound(1);} return;
    case Page::DeleteAsk: if(delete_choice==0) delete_now(); else {page=delete_from;sound(1);} return;
    case Page::Options: break;
    default: title_cmds.push_back(Cmd::Confirm);return; // D19J: the title pages
    }
    if(screen>=0 && screens[screen].rows[cursor].link) {open_keys();return;}
    if(screen>=0) {leave_screen(true);return;}
    const Item& it=options_items[cursor];
    if(it.guest_sub>=0) {pending_cursor=cursor;pending_sub=it.guest_sub;sound(0);}
    else open_screen(it.screen);
}
void back(uint16_t mode) {
    switch(page) {
    case Page::Pause: act_now(Act::Resume);return;  // Escape resumes, as the original's Start
    case Page::Death: return;                        // nothing to go back to
    case Page::Brief: open_list(Page::Pause,Act::Brief);sound(0);return;
    case Page::Ask: ask_cancel();return;
    case Page::Keys: keys_back();return;
    case Page::Load: load_back();return;
    case Page::LoadAsk: page=Page::Load;sound(1);return;
    case Page::Stats: return; // as the original: only CONTINUE leaves
    case Page::Save:
        thumb_hide();load_note.clear();
        if(stats_open) {page=Page::Stats;cursor=1;sound(0);return;} // D20B; D20F: on SAVE GAME
        if(direct_open) {act_now(Act::Resume);return;} // D20: F2 / F6 from play: back to play
        open_list(Page::Pause,Act::Save);sound(0);return;
    case Page::SaveAsk: page=Page::Save;sound(1);return;
    case Page::DeleteAsk: page=delete_from;sound(1);return;
    case Page::Options: break;
    default: title_cmds.push_back(Cmd::Back);return; // D19J: the title pages
    }
    if(screen>=0) {leave_screen(false);return;}
    // In gameplay OPTIONS goes back to our pause list (the original's 12 would
    // draw its own pause menu for a frame first).
    if(mode==1) {open_list(Page::Pause,Act::Options);sound(0);return;}
    // D19J: on the title to our main menu, OPTIONS highlighted (the original's
    // 10 is a tick with no menu, then state 6). The OPTIONS cursor resets.
    open_list(Page::Main,Act::Options);title_enter=true;
    pending_cursor=0;pending_sub=title_sub;sound(0);
}
} // namespace

// D20, from a key press or the console (not the emulation thread): EDuke32's
// Quick_Save, Quick_Load and its F2 / F3 save / load menus, in a one-player
// game (gameplay, or the death list for loads).
QuickResult game_menu_quick(Quick q) {
    if(!enabled() || psx_mod_replay_active()) return QuickResult::Ignored;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if(!identity_checked || !identity_ok || psx_mod_read_half(game_mode)!=1 || psx_mod_read_word(players)!=1)
        return QuickResult::Ignored;
    // One thing at a time; menus (other than the death list) keep their keys.
    if(quitting || loading || saving || quick_saving || quick_loading || savestate_pending()
       || pending_final!=Final::None || resumed || direct!=Page::Options) return QuickResult::Done;
    const bool death_list=owned && page==Page::Death && psx_mod_read_half(pause_screen)==death_sub;
    if(!death_list && (owned || psx_mod_read_half(paused) || psx_mod_read_half(pause_screen))) return QuickResult::Done;
    if(q==Quick::Save || q==Quick::SaveMenu) {
        if(death_list || duke_dead()) {input_notice("CAN'T SAVE WHILE DEAD");return QuickResult::Done;}
        if(q==Quick::SaveMenu || quick_slot<0) return quick_open(Page::Save);
        quick_save_slot=quick_slot;quick_saved=false;
        if(savestate_request_save(quick_slot)) quick_saving=true;
        else input_notice("THE GAME COULD NOT BE SAVED");
        return QuickResult::Done;
    }
    bool ready=q==Quick::Load && quick_slot>=0;
    if(ready) {read_slots();ready=loadable(quick_slot);}
    if(!ready) {
        // No slot yet (or it is gone): LOAD GAME, from the death list too.
        if(!death_list) return quick_open(Page::Load);
        open_load(Page::Death);sound(0);
        return QuickResult::Done;
    }
    const uint32_t from=psx_mod_savestate_loads();
    if(!savestate_request_load(quick_slot)) {input_notice("THIS SAVE COULD NOT BE LOADED");return QuickResult::Done;}
    quick_loading=true;quick_loading_from=from;
    // Resume policy is set from the successfully applied load generation.
    pause_thumb_ok=thumb_from_load=savestate_read_thumb(quick_slot,pause_thumb,SAVESTATE_THUMB_W,SAVESTATE_THUMB_H)!=0;
    return QuickResult::Load;
}

uint64_t drops=0; int drop_why=0; uint16_t drop_state=0, drop_sub=0;   // diagnostic
bool game_menu_input(const MenuKeys& keys) {
    if(!enabled()) return false;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    savestate_check();
    // Taps that arrive while the menu cannot act yet wait for the next read.
    static MenuKeys carry;
    MenuKeys k=keys;
    k.up_taps+=carry.up_taps;k.down_taps+=carry.down_taps;k.left_taps+=carry.left_taps;k.right_taps+=carry.right_taps;
    k.confirm_taps+=carry.confirm_taps;k.back_taps+=carry.back_taps;k.square_taps+=carry.square_taps;k.delete_taps+=carry.delete_taps;
    carry=MenuKeys{};
    // Busy drawing, or a screen change waits for the next tick: keys wait too.
    if(quitting) return owned;
    // D19J: starting a game; no key reaches the original handler meanwhile.
    if(handoff!=Handoff::None) {
        if(psx_mod_read_half(game_mode)==0) return true;
        handoff=Handoff::None;
    }
    if(loading) {
        // D19K: until the load (savestate_check ends this), or its failure.
        if(!savestate_pending() && psx_mod_savestate_loads()==loading_from) {
            loading=false;resume_load=false;page=Page::Load;load_note="THIS SAVE COULD NOT BE LOADED";sound(1);
        }
        return owned;
    }
    if(saving) {
        // D19Q: until the framework has taken the save (or refused it).
        if(!savestate_pending()) save_done();
        return owned;
    }
    if(busy || (owned && (pending_sub>=0 || pending_final!=Final::None || pending_state>=0 || stats_continue))) {
        if(owned) carry=k;
        return owned;
    }
    // D20B: the statistics screen has no menu context of its own.
    const bool stats=psx_mod_read_half(game_mode)==2;
    const MenuContext* c=stats ? nullptr : current_context();
    if(stats ? !(owned && stats_open) : (!owned || !c || !our_screen(c,psx_mod_read_half(c->sub)))) {
        if(rep_up.held || rep_down.held || rep_left.held || rep_right.held || k.up || k.down) {
            ++drops; drop_why=!owned?1:!c?2:3; drop_state=psx_mod_read_half(fe_state); drop_sub=c?psx_mod_read_half(c->sub):0xffff;
        }
        rep_up=rep_down=rep_left=rep_right=Repeat{};
        key_wait_end();
        return false;
    }
    // D19G: waiting for a new key, only that key counts.
    if(key_wait) {
        if(page!=Page::Keys) key_wait_end();
        else {
            const int code=input_take_captured_key();
            if(code!=input_no_key()) key_pressed(code);
            return true;
        }
    }
    const uint64_t now=SDL_GetTicks();
    for(int n=repeat(rep_up,k.up,k.up_taps,now);n>0;--n) move(-1);
    for(int n=repeat(rep_down,k.down,k.down_taps,now);n>0;--n) move(1);
    for(int n=repeat(rep_left,k.left,k.left_taps,now);n>0;--n) change(-1,now-rep_left.since);
    for(int n=repeat(rep_right,k.right,k.right_taps,now);n>0;--n) change(1,now-rep_right.since);
    for(int w=k.wheel;w>0;--w) move(-1);
    for(int w=k.wheel;w<0;++w) move(1);
    if(k.confirm_taps) confirm();
    else if(k.back_taps) back(stats ? 2 : c->mode);
    else if(k.square_taps) defaults();
    else if(k.delete_taps) delete_ask();
    // D19J: any key keeps the main menu from going back to PRESS START.
    if(k.up || k.down || k.left || k.right || k.confirm || k.back || k.square || k.wheel || k.up_taps || k.down_taps
       || k.left_taps || k.right_taps || k.confirm_taps || k.back_taps || k.square_taps || k.delete_taps) title_activity=true;
    return true;
}

namespace {
// ---- drawing: the original's own calls, on one saved stack and scratchpad ----
struct Guest {
    CPUState* source;uint32_t top,text,base;bool ok=true;
    uint32_t saved[1024],scratch[256];
    explicit Guest(CPUState* cpu):source(cpu),top(cpu->gpr[29]),text(top-0x80),base((top-4096)&~3u) {
        for(unsigned i=0;i<1024;++i) saved[i]=psx_mod_read_word(base+4*i);
        for(unsigned i=0;i<256;++i) scratch[i]=psx_mod_read_word(0x1f800000+4*i);
    }
    ~Guest() {
        for(unsigned i=0;i<1024;++i) if(psx_mod_read_word(base+4*i)!=saved[i]) psx_mod_write_word(base+4*i,saved[i]);
        for(unsigned i=0;i<256;++i) if(psx_mod_read_word(0x1f800000+4*i)!=scratch[i]) psx_mod_write_word(0x1f800000+4*i,scratch[i]);
    }
    uint32_t call(uint32_t address,uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t s16=0,uint32_t s20=0,uint32_t s24=0) {
        if(!ok) return 0;
        CPUState cpu=*source;cpu.pc=0;cpu.gpr[29]=top-0x100;cpu.gpr[31]=sentinel;
        cpu.gpr[4]=a0;cpu.gpr[5]=a1;cpu.gpr[6]=a2;cpu.gpr[7]=a3;
        psx_mod_write_word(top-0x100+16,s16);psx_mod_write_word(top-0x100+20,s20);psx_mod_write_word(top-0x100+24,s24);
        psx_mod_host_call_begin(); // D19J: no savestate inside our call
        psx_dispatch_call(&cpu,address,sentinel);
        psx_mod_host_call_end();
        ok=cpu.pc==0 && !g_psx_call_bail;
        return cpu.gpr[2];
    }
    void put(const char* s) {
        unsigned i=0;
        for(;s[i] && i<100;++i) psx_mod_write_byte(text+i,(uint8_t)s[i]);
        psx_mod_write_byte(text+i,0);
    }
    static bool drawn_char(char c,uint32_t flags) {return c=='"' || ((c=='!' || c=='%') && quote_font(flags)==0);}
    static bool has_drawn(const char* s,uint32_t flags) {for(;*s;++s) if(drawn_char(*s,flags)) return true;return false;}
    void say(const char* s,int x,int y,uint32_t flags) {
        if(has_drawn(s,flags)) {say_quoted(s,x,y,flags);return;}
        put(s);call(fn_text,text,(uint32_t)x,(uint32_t)y,flags);
    }
    // D19T: text with double quotes, as the text call would place it: the
    // runs between quotes left-aligned from the aligned start, '"' opening
    // and closing in turn.
    static int quote_font(uint32_t flags) {return (flags>>8&3)==1 ? 1 : 0;}
    int quote_width(uint32_t flags) {
        const int f=quote_font(flags);
        return quote_metrics[f].pair_step+psx_mod_read_byte(glyph_table+0x100u*(uint32_t)f+4u*'\''+2);
    }
    void say_quoted(const char* s,int x,int y,uint32_t flags) {
        const uint32_t left=flags&~(centre|right);
        const int w=width(s,flags&0x300);
        if(flags&centre) x-=w/2; else if(flags&right) x-=w;
        std::string run;bool open=true;
        for(const char* p=s;;++p) {
            if(*p && !drawn_char(*p,flags)) {run+=*p;continue;}
            if(!run.empty()) {const int rw=width(run.c_str(),flags&0x300);put(run.c_str());call(fn_text,text,(uint32_t)x,(uint32_t)y,left);x+=rw;run.clear();}
            if(!*p) break;
            if(*p=='"') {say_quote(x,y,left,open);x+=quote_width(flags);open=!open;}
            else {say_runs(*p,x,y,left);x+=drawn_width(*p);}
        }
    }
    // D20D: '!' and '%' in the small font, one glyph quad per run.
    int drawn_width(char c) {return c=='!' ? width("I",0) : width("/",0)+7;}
    void say_runs(char c,int x,int y,uint32_t flags) {
        const GlyphRun* r=c=='!' ? small_bang : small_percent;
        const size_t n=c=='!' ? std::size(small_bang) : std::size(small_percent);
        for(size_t i=0;i<n && ok;++i) {
            call(fn_glyph,'I',(uint32_t)x,(uint32_t)y,flags);
            if(!ok) return;
            const uint32_t p=psx_mod_read_word(glyph_next)-glyph_size;
            const uint16_t x0=(uint16_t)(psx_mod_read_half(p+8)+r[i].x),y0=(uint16_t)(psx_mod_read_half(p+0xa)+r[i].y);
            const uint16_t x1=(uint16_t)(x0+r[i].n),y1=(uint16_t)(y0+1);
            const uint8_t u0=r[i].u,u1=(uint8_t)(r[i].u+r[i].n),v0=r[i].v,v1=(uint8_t)(r[i].v+1);
            const uint16_t xs[4]={x0,x1,x0,x1},ys[4]={y0,y0,y1,y1};const uint8_t us[4]={u0,u1,u0,u1},vs[4]={v0,v0,v1,v1};
            for(uint32_t k=0;k<4;++k) {
                psx_mod_write_half(p+8+8*k,xs[k]);psx_mod_write_half(p+0xa+8*k,ys[k]);
                psx_mod_write_byte(p+0xc+8*k,us[k]);psx_mod_write_byte(p+0xd+8*k,vs[k]);
            }
        }
    }
    void say_quote(int x,int y,uint32_t flags,bool open) {
        const QuoteMetrics& m=quote_metrics[quote_font(flags)];
        for(int i=0;i<2;++i) {
            call(fn_glyph,'\'',(uint32_t)(x+i*m.pair_step),(uint32_t)y,flags);
            if(!open || !ok) continue;
            const uint32_t p=psx_mod_read_word(glyph_next)-glyph_size; // the packet just added
            const uint8_t u=psx_mod_read_byte(p+0xc),v=psx_mod_read_byte(p+0xd),u1=psx_mod_read_byte(p+0x14);
            const uint8_t v1=(uint8_t)(v+m.ink_rows-1);
            psx_mod_write_byte(p+0xc,u1);psx_mod_write_byte(p+0xd,v1);
            psx_mod_write_byte(p+0x14,u);psx_mod_write_byte(p+0x15,v1);
            psx_mod_write_byte(p+0x1c,u1);psx_mod_write_byte(p+0x1d,v);
            psx_mod_write_byte(p+0x24,u);psx_mod_write_byte(p+0x25,v);
            const uint16_t y1=(uint16_t)(psx_mod_read_half(p+0xa)+m.ink_rows-1);
            psx_mod_write_half(p+0x1a,y1);psx_mod_write_half(p+0x22,y1);
        }
    }
    // A string of the game's text table, by id, as the original passes it.
    void say_id(int id,int x,int y,uint32_t flags) {
        const uint32_t s=psx_mod_read_word(strings+4u*(uint32_t)id);
        if(s>=0x80010000u && s<0x80200000u) call(fn_text,s,(uint32_t)x,(uint32_t)y,flags);
    }
    int width(const char* s,uint32_t font) {
        const char* q=s;
        while(*q && !drawn_char(*q,font)) ++q;
        if(*q) {
            const std::string before(s,q);
            return (before.empty() ? 0 : width(before.c_str(),font))+(*q=='"' ? quote_width(font) : drawn_width(*q))+(q[1] ? width(q+1,font) : 0);
        }
        put(s);return (int)call(fn_width,text,font>>8,0,0);
    }
    // D19N: text drawn dimmer: the modulation of the glyph packets just added.
    void say_dim(const char* s,int x,int y,uint32_t flags) {
        const uint32_t before=psx_mod_read_word(glyph_next);
        say(s,x,y,flags);
        const uint32_t after=psx_mod_read_word(glyph_next);
        if(after<=before || after-before>glyph_size*128) return; // the buffer wrapped: leave it lit
        for(uint32_t p=before;p<after;p+=glyph_size)
            if(psx_mod_read_byte(p+7)==0x2c) for(uint32_t c=4;c<7;++c) psx_mod_write_byte(p+c,dim_level);
    }
    // D19G: text in another CLUT row of the font's column (the call offers
    // gold, blue and red): shift the CLUT of the glyph packets it just added.
    void say_clut(const char* s,int x,int y,uint32_t flags,int clut_delta) {
        const uint32_t before=psx_mod_read_word(glyph_next);
        say(s,x,y,flags);
        const uint32_t after=psx_mod_read_word(glyph_next);
        if(after<=before || after-before>glyph_size*128) return; // the buffer wrapped: leave it gold
        for(uint32_t p=before;p<after;p+=glyph_size) psx_mod_write_half(p+14,(uint16_t)(psx_mod_read_half(p+14)+clut_delta));
    }
    // D19J: the prompt helper itself, with a text id, as the title pages call it.
    void prompt_id(Icon icon,int x,int y,int id) {call(fn_prompt,(uint32_t)icon,(uint32_t)x,(uint32_t)y,(uint32_t)id);}
    // The prompt helper 0x8008411c with our own label: button left of the
    // centred small-font label.
    void prompt(Icon icon,const char* label,int x,int y) {
        const int w=width(label,font_small);
        call(fn_sprite,sprites+16*icon,(uint32_t)(x-w/2-26),(uint32_t)(y-3),0,100);
        say(label,x,y,font_small|centre);
    }
};
struct Geometry {int x0,y0,w,h;};
Geometry read_geometry() {
    return {(int16_t)psx_mod_read_half(geometry),(int16_t)psx_mod_read_half(geometry+2),
            (int16_t)psx_mod_read_half(geometry+4),(int16_t)psx_mod_read_half(geometry+6)};
}
// Prompt row at the original's y. OPTIONS: its three, w/4 apart. Settings:
// GAME SETTINGS' four (SELECT -3w/8, CHANGE 5 - w/8, CONFIRM w/8 - 5, CANCEL
// 3w/8), and DEFAULT on the heading line in the CANCEL column.
void prompts(Guest& g,const Geometry& geo,bool settings) {
    const int y=-geo.y0-(geo.h>>4)-19;
    if(!settings) {
        g.prompt(icon_select,"SELECT",-geo.w/4,y);
        g.prompt(icon_cross,"CONFIRM",0,y);
        g.prompt(icon_triangle,back_label,geo.w/4,y);
        return;
    }
    const int eighth=geo.w/8;
    g.prompt(icon_select,"SELECT",-3*eighth,y);
    g.prompt(icon_change,"CHANGE",5-eighth,y);
    g.prompt(icon_cross,"CONFIRM",eighth-5,y);
    g.prompt(icon_triangle,"CANCEL",3*eighth,y);
    g.prompt(icon_square,"DEFAULT",3*eighth,geo.y0+(171*geo.h>>11)+3);
}
void draw_options(Guest& g,const Geometry& geo) {
    // The handler's 19-line spacing, centred where its three were.
    const int y=(geo.h-57)/2+geo.y0-19*(option_count-3)/2;
    for(int i=0;i<option_count;++i)
        g.say(options_items[i].label,0,y+19*i,font_menu|centre|(i==cursor?blue:0));
    prompts(g,geo,false);
}
void wrap(Guest& g,const char* text,int max,std::vector<std::string>& lines) {
    std::string line,word;
    auto flush=[&]{
        const std::string t=line.empty()?word:line+" "+word;
        if(!line.empty() && g.width(t.c_str(),font_small)>max) {lines.push_back(line);line=word;}
        else line=t;
        word.clear();
    };
    for(const char* p=text;;++p) {
        if(*p==' ' || !*p) {if(!word.empty()) flush();if(!*p) break;}
        else word+=*p;
    }
    if(!line.empty()) lines.push_back(line);
}
// D19N: the value a row shows now (the draft on the open screen).
const char* value_of(const char* key) {
    for(size_t s=0;s<screens.size();++s) for(size_t i=0;i<screens[s].rows.size();++i) {
        const Row& r=screens[s].rows[i];
        if(r.key && !r.slider && !r.link && !std::strcmp(r.key,key)) return r.choices[(int)s==screen ? draft[s][i] : committed[s][i]].env;
    }
    return "";
}
// D19N: what a row needs to have an effect, or null. Such a row stays
// changeable; it is drawn grey and its description says what it needs.
const char* needs(const Row& r) {
    if(!r.key) return nullptr;
    const std::string k=r.key;
    auto is=[](const char* key,const char* v) {return !std::strcmp(value_of(key),v);};
    if((k=="shoulder" || k=="camera_distance" || k=="weapon_aim") && is("camera","original")) return "NEEDS CAMERA INDEPENDENT, IN ADVANCED";
    if(k=="crosshair" && is("weapon_aim","original")) return "NEEDS WEAPON AIM VIEW, IN ADVANCED";
    if(k=="red_dot" && is("aim_assist","off")) return "NEEDS AUTO AIM ON";
    if((k=="internal_scale" || k=="geometry_precision" || k=="texture_precision") && is("renderer","software")) return "NEEDS RENDERER OPENGL, IN ADVANCED";
    return nullptr;
}
void draw_settings(Guest& g,const Geometry& geo) {
    const Screen& s=screens[screen];
    const Values& v=draft[screen];
    // GAME SETTINGS: heading y0 + 171h/2048, rows from y0/2 + 10, 20 apart,
    // label right-aligned at -10, value at +10; slider label right-aligned at
    // x0 + w/3, its bar 20 further on.
    g.say(s.title,0,geo.y0+(171*geo.h>>11),font_menu|centre);
    const int n=(int)s.rows.size(), vis=rows_visible(s);
    const bool tight=compact(s);
    const int spacing=tight?14:20, y0=tight?geo.y0/2-14:geo.y0/2+10;
    int slot=0, used=0, last=top-1;
    for(int i=top;i<n && used+row_units(s,i)<=vis;++i) {
        // A two-place slider row is drawn halfway down its places.
        const int y=y0+used*spacing+(row_units(s,i)-1)*spacing/2;
        used+=row_units(s,i);last=i;
        const Row& r=s.rows[i];
        const uint32_t hi=i==cursor?blue:0;
        if(r.heading) {g.say_clut(r.label,0,y,font_small|centre,clut_amber);continue;}
        if(needs(r)) {
            // D19N: a row with no effect now: dimmed (its label blue under the cursor).
            const std::string label=std::string(r.label)+":";
            if(hi) g.say(label.c_str(),-10,y,font_small|right|hi);
            else g.say_dim(label.c_str(),-10,y,font_small|right);
            g.say_dim(r.choices[v[i]].label,10,y,font_small);
            continue;
        }
        if(r.slider && slot<slider_slots) {
            const uint32_t obj=sliders+slider_size*slot++;
            g.say(r.label,geo.x0+geo.w/3,y,font_small|right|hi);
            g.call(fn_slider_init,obj,0,(uint32_t)(y+5),(uint32_t)r.lo,(uint32_t)r.hi,(uint32_t)v[i],(uint32_t)r.step);
            g.call(fn_slider_draw,obj,hi,0,0);
        } else if(r.slider) {
            char value[16];std::snprintf(value,sizeof value,"%.2f",v[i]*r.scale);
            g.say((std::string(r.label)+":").c_str(),-10,y,font_small|right|hi);
            g.say(value,10,y,font_small);
        } else {
            g.say((std::string(r.label)+":").c_str(),-10,y,font_small|right|hi);
            g.say(r.choices[v[i]].label,10,y,font_small);
        }
    }
    if(tight) {
        // Scroll marks: the SELECT button beside the list while rows are hidden.
        if(top>0 || last<n-1) g.call(fn_sprite,sprites+16*icon_select,(uint32_t)(geo.w/2-geo.w/8),(uint32_t)(y0+spacing*vis/2-8),0,100);
    }
    const Row& r=s.rows[cursor];
    std::string desc=r.desc;
    if(const char* need=needs(r)) desc=need;
    else if(!row_live(r)) desc+=". NEXT LAUNCH";
    std::vector<std::string> lines;
    wrap(g,desc.c_str(),geo.w*3/4,lines);
    const int ybase=-geo.y0-(geo.h>>4)-19-14-12*((int)std::min<size_t>(lines.size(),2)-1);
    for(size_t l=0;l<lines.size() && l<2;++l) g.say(lines[l].c_str(),0,ybase+12*(int)l,font_small|centre);
    prompts(g,geo,true);
}

// D19G: a key name; [ and ] (the menu font has no brackets) are built from
// its own glyphs, as D24C built '!' from I and a period: "[" is L (stem and
// bottom bar in one glyph, no seam) with a hyphen as its top bar; "]" is I
// with hyphen bars, the stem queued first so the top bar does not cut it.
void say_key(Guest& g,const std::string& key,int x,int y,uint32_t flags) {
    if(key=="[") {g.say("L",x,y,flags);g.say("-",x+3,y-5,flags);return;}
    if(key=="]") {g.say("I",x+6,y,flags);g.say("-",x+4,y-5,flags);g.say("-",x,y+3,flags);return;}
    g.say(key.c_str(),x,y,flags);
}
// D19G: KEY BINDINGS, laid out as the compact settings screens.
void draw_keys(Guest& g,const Geometry& geo) {
    const int hy=geo.y0+(171*geo.h>>11);
    g.say("KEY BINDINGS",0,hy,font_menu|centre);
    const int spacing=14, y0=geo.y0/2-14;
    for(int j=0;j<key_rows_visible && top+j<key_row_count;++j) {
        const int i=top+j, y=y0+j*spacing;
        const KeyRow& k=key_rows[i];
        if(k.action<0) {g.say_clut(k.label,0,y,font_small|centre,clut_amber);continue;}
        g.say((std::string(k.label)+":").c_str(),-10,y,font_small|right|(i==cursor?blue:0));
        if(i==cursor && key_wait) g.say("PRESS A KEY",10,y,font_small|blue);
        else {
            const int code=input_binding_code(Action(k.action));
            int x=10;
            if(k.fixed) {
                g.say(code ? (std::string(k.fixed)+" /").c_str() : k.fixed,x,y,font_small);
                x+=g.width((std::string(k.fixed)+" / ").c_str(),font_small);
            }
            if(code || !k.fixed) say_key(g,key_shown(code),x,y,font_small);
        }
    }
    if(top>0 || top+key_rows_visible<key_row_count)
        g.call(fn_sprite,sprites+16*icon_select,(uint32_t)(geo.w/2-geo.w/8),(uint32_t)(y0+spacing*key_rows_visible/2-8),0,100);
    std::string desc=key_note;
    std::vector<std::string> lines;
    if(!desc.empty()) wrap(g,desc.c_str(),geo.w*3/4,lines);
    else if(key_wait) lines={"PRESS A KEY OR MOUSE BUTTON.","ESCAPE CANCELS, BACKSPACE CLEARS"};
    else if(key_rows[cursor].hint) lines={key_rows[cursor].hint,"CONFIRM, THEN PRESS THE NEW KEY"};
    else wrap(g,(std::string(key_rows[cursor].label)+". CONFIRM, THEN PRESS THE NEW KEY").c_str(),geo.w*3/4,lines);
    const int py=-geo.y0-(geo.h>>4)-19;
    const int ybase=py-14-12*((int)std::min<size_t>(lines.size(),2)-1);
    for(size_t l=0;l<lines.size() && l<2;++l) g.say(lines[l].c_str(),0,ybase+12*(int)l,font_small|centre);
    if(key_wait) {g.prompt(icon_triangle,"CANCEL",0,py);return;}
    g.prompt(icon_select,"SELECT",-geo.w/4,py);
    g.prompt(icon_cross,"CHANGE",0,py);
    g.prompt(icon_triangle,"RETURN",geo.w/4,py);
    g.prompt(icon_square,"DEFAULT",3*(geo.w/8),hy+3);
}

// ---- D19H pages ----
// The original's text height: headings at y0 + 171h/2048 in the menu font.
int heading_y(const Geometry& geo) {return geo.y0+(171*geo.h>>11);}
// A centred list in the menu font, 19 apart, centred where OPTIONS' three were
// (the original's own arithmetic), plus a shift.
void draw_list(Guest& g,const Geometry& geo,int shift) {
    const int n=(int)list.size(), y=(geo.h-57)/2+geo.y0-19*(n-3)/2+shift;
    for(int i=0;i<n;++i) g.say(list[i].label,0,y+19*i,font_menu|centre|(i==cursor?blue:0));
}
void level_title(Guest& g,const Geometry& geo) {
    const int n=level();
    if(n<0 || n>=32) return;
    const int id=(int16_t)psx_mod_read_half(level_names+2*n);
    if(id>0 && id<=text_last) g.say_id(id,0,heading_y(geo),font_menu|centre);
}
int prompt_y(const Geometry& geo) {return -geo.y0-(geo.h>>4)-19;}
void draw_pause(Guest& g,const Geometry& geo) {
    level_title(g,geo);
    draw_list(g,geo,0);
    const int y=prompt_y(geo);
    g.prompt(icon_select,"SELECT",-geo.w/4,y);
    g.prompt(icon_cross,"CONFIRM",0,y);
    g.prompt(icon_triangle,"RESUME",geo.w/4,y);
}
void draw_death(Guest& g,const Geometry& geo) {
    // The original's "N CONTINUES REMAINING" (its "%d %s", at y -80).
    const int n=psx_mod_read_byte(continues);
    const uint32_t s=psx_mod_read_word(strings+4u*(n==1 ? text_remaining : text_remaining_many));
    std::string words;
    for(uint32_t a=s;s>=0x80010000u && s<0x80200000u && words.size()<40;++a) {const char ch=(char)psx_mod_read_byte(a);if(!ch) break;words+=ch;}
    char line[64];std::snprintf(line,sizeof line,"%d %s",n,words.c_str());
    g.say(line,0,-80,font_menu|centre);
    draw_list(g,geo,8);
    const int y=prompt_y(geo);
    g.prompt(icon_select,"SELECT",-geo.w/8,y);
    g.prompt(icon_cross,"CONFIRM",geo.w/8,y);
}
void draw_brief(Guest& g,const Geometry& geo) {
    // The original pause screen's briefing: title, then up to ten lines from
    // x0 + w/32 + 15, 26 below the title and 15 apart, in the small font.
    level_title(g,geo);
    const int n=level(), x=geo.x0+(geo.w>>5)+15, y=heading_y(geo)+26;
    for(int i=0;i<10;++i) {
        const int id=(int16_t)psx_mod_read_half(briefings+20*n+2*i);
        if(id>0 && id<=text_last) g.say_id(id,x,y+15*i,font_small);
    }
    g.prompt(icon_triangle,"RETURN",0,prompt_y(geo));
}
void draw_ask(Guest& g,const Geometry& geo) {
    const int heading=ask_heading();
    if(heading<0) g.say(heading==-1 ? "QUIT GAME?" : heading==-2 ? "QUIT TO MENU?" : "RESTORE DEFAULT KEYS?",0,-40,font_menu|centre);
    else g.say_id(heading,0,-40,font_menu|centre);
    g.say_id(text_yes,0,-6,font_menu|centre|(cursor==0?blue:0));
    g.say_id(text_no,0,13,font_menu|centre|(cursor==1?blue:0));
    const int y=prompt_y(geo);
    g.prompt(icon_select,"SELECT",-geo.w/4,y);
    g.prompt(icon_cross,"CONFIRM",0,y);
    g.prompt(icon_triangle,"CANCEL",geo.w/4,y);
}

// ---- D19J title pages: the originals' calls, arguments and order ----
bool show_headings() {return psx_mod_read_half(pause_screen)!=screen_no_titles;}
int16_t half(uint32_t a) {return (int16_t)psx_mod_read_half(a);}
// State 7 (0x80022b64): the title in two lines, the items from y -30, 15
// apart, in font 0 (blue highlighted), then SELECT and CONFIRM.
void draw_main(Guest& g,const Geometry& geo) {
    const int hy=heading_y(geo);
    if(show_headings()) {g.say_id(text_title1,0,hy,font_menu|centre);g.say_id(text_title2,0,hy+19,font_menu|centre);}
    for(int i=0;i<(int)list.size();++i) {
        const int y=-30+15*i;
        const uint32_t f=font_small|centre|(i==cursor?blue:0);
        switch(list[i].act) {
        case Act::OnePlayer: g.say_id(text_one,0,y,f);break;
        case Act::TwoPlayer: g.say_id(text_two,0,y,f);break;
        case Act::Load: g.say_id(text_load,0,y,f);break;
        case Act::Options: g.say_id(text_options,0,y,f);break;
        case Act::Level: {
            const int id=half(level_names+2*(psx_mod_read_half(level_choice)&31));
            if(id>0 && id<=text_last) g.say_id(id,0,y,f);
            break;
        }
        default: g.say(list[i].label,0,y,f);break; // QUIT GAME
        }
    }
    const int y=prompt_y(geo);
    g.prompt_id(icon_select,-(geo.w/4),y,text_select);
    g.prompt_id(icon_cross,geo.w/4,y,text_confirm);
}
// 0x80084534: SELECT, CONFIRM and RETURN, the heading at y -76, then the
// four difficulties from y -47, 19 apart.
void draw_difficulty(Guest& g,const Geometry& geo) {
    const int y=prompt_y(geo);
    g.prompt_id(icon_select,-(geo.w/4),y,text_select);
    g.prompt_id(icon_cross,0,y,text_confirm);
    g.prompt_id(icon_triangle,geo.w/4,y,text_return);
    if(show_headings()) g.say_id(text_difficulty,0,-76,font_menu|centre);
    const int cur=half(diff_cursor);
    for(int i=0;i<4;++i) g.say_id((int)psx_mod_read_word(diff_ids+4*i),0,19*i-47,font_menu|centre|(i==cur?blue:0));
}
// 0x8008484c: four prompts w/8 apart, the heading, then LEVEL (menu font) and
// TIME, KILLS and SPLIT (font 0), labels right-aligned at 0, values from 0.
void draw_two(Guest& g,const Geometry& geo) {
    const int y=prompt_y(geo), e=geo.w/8;
    g.prompt_id(icon_select,15-3*e,y,text_select);
    g.prompt_id(icon_change,5-e,y,text_change);
    g.prompt_id(icon_cross,e-5,y,text_confirm);
    g.prompt_id(icon_triangle,3*e,y,text_cancel);
    if(show_headings()) g.say_id(text_two_heading,0,heading_y(geo),font_menu|centre);
    const int cur=half(two_cursor);
    int row=geo.y0+(geo.h-76)/2;
    g.say_id(text_level,0,row,font_menu|right|(cur==0?blue:0));
    g.say_id(half(two_levels+2*std::clamp<int>(half(two_level),0,5)),0,row,font_menu);
    row+=38;
    // The original's formats (0x80016e98, guarded): "%d %s" with MINUTES, "%d".
    char value[48];
    g.say_id(text_time,0,row,font_small|right|(cur==1?blue:0));
    if(const int t=half(two_time)) {
        const uint32_t s=psx_mod_read_word(strings+4u*text_minutes);
        std::string minutes;
        for(uint32_t a=s;s>=0x80010000u && s<0x80200000u && minutes.size()<20;++a) {const char ch=(char)psx_mod_read_byte(a);if(!ch) break;minutes+=ch;}
        std::snprintf(value,sizeof value,"%d %s",t,minutes.c_str());
        g.say(value,0,row,font_small);
    } else g.say_id(text_unlimited,0,row,font_small);
    row+=15;
    g.say_id(text_kills,0,row,font_small|right|(cur==2?blue:0));
    if(const int k=half(two_kills)) {std::snprintf(value,sizeof value,"%d",k);g.say(value,0,row,font_small);}
    else g.say_id(text_unlimited,0,row,font_small);
    row+=15;
    g.say_id(text_split,0,row,font_small|right|(cur==3?blue:0));
    g.say_id(half(two_split) ? text_vertical : text_horizontal,0,row,font_small);
}
// 0x80085060: the heading, PLAYER 1 and PLAYER 2 (a name blue until that
// player confirms), then CHANGE and CONFIRM, or START once both have; CANCEL.
void draw_character(Guest& g,const Geometry& geo) {
    if(show_headings()) g.say_id(text_character,0,heading_y(geo),font_menu|centre);
    const uint32_t locks=psx_mod_read_word(char_locks);
    int row=geo.y0+(geo.h-76)/2;
    for(uint32_t p=0;p<2;++p,row+=38) {
        g.say_id(p ? text_player2 : text_player1,0,row,font_menu|right);
        const uint32_t sel=psx_mod_read_word(char_choice+4*p);
        g.say_id((int)psx_mod_read_word(char_ids+4*(sel<6 ? sel : 0)),0,row,font_menu|(locks&(1u<<p) ? 0 : blue));
    }
    const int y=prompt_y(geo);
    if(locks!=3) {g.prompt_id(icon_change,-(geo.w/4),y,text_change);g.prompt_id(icon_cross,0,y,text_confirm);}
    else g.prompt_id(icon_cross,geo.x0+geo.w/3,y,text_start);
    g.prompt_id(icon_triangle,geo.w/4,y,text_cancel);
}

// ---- D19K: LOAD GAME ----
// The page's host image: the chosen slot's thumbnail (the menu font cannot
// draw a picture) and the date / warning lines in Console steel (TTK Medium
// Italic with its navy shadow, the console's set; user 2026-10-20). They are
// composed in the 4:3 menu area at 960x720, the text at 2x (a third larger
// than the console's 1x in the 480-line space, so it reads beside the menu
// font), cropped to what is drawn, and the renderer places that rectangle on the game
// picture after the guest's image. Shown while the page draws it; gone 250 ms
// after the last draw (a page left, the game resumed) or at once on a change.
constexpr int canvas_w=960, canvas_h=720, steel_set=2, steel_scale=2;
struct OverlayText {std::string text;int x,y;bool operator==(const OverlayText& o) const {return text==o.text && x==o.x && y==o.y;}};
struct Overlay {
    int slot=-1;bool dim=false;int tx=0,ty=0,tw=0,th=0; // thumbnail, canvas pixels
    std::vector<OverlayText> lines;                      // centred at x, top at y
    bool operator==(const Overlay& o) const {return slot==o.slot && dim==o.dim && tx==o.tx && ty==o.ty && tw==o.tw && th==o.th && lines==o.lines;}
};
std::mutex thumb_mutex;
Overlay overlay_next, overlay_shown;
std::vector<uint32_t> overlay_px, overlay_out;
int overlay_x=0, overlay_y=0, overlay_w=0, overlay_h=0;
bool overlay_on=false, overlay_valid=false;
uint64_t overlay_until=0;
void thumb_hide() {std::lock_guard<std::mutex> lock(thumb_mutex);overlay_on=false;overlay_valid=false;}
bool steel_text() {return ttk_font_line_height(steel_set)>0;}
// Letter spacing one pixel wider than the console's, for dates and times.
int steel_width(const std::string& t) {
    int w=0;
    for(char c:t) {const char one[2]={c,0};w+=ttk_font_text_width(steel_set,one)+1;}
    return w>0 ? w*steel_scale : 0;
}
int canvas_x(const Geometry& geo,int x) {return (x-geo.x0)*canvas_w/geo.w;}
int canvas_y(const Geometry& geo,int y) {return (y-geo.y0)*canvas_h/geo.h;}
void overlay_thumb(int slot,bool dim,const Geometry& geo,int x,int y,int w,int h) {
    Overlay& o=overlay_next;
    o.slot=slot;o.dim=dim;o.tx=canvas_x(geo,x);o.ty=canvas_y(geo,y);
    o.tw=canvas_x(geo,x+w)-o.tx;o.th=canvas_y(geo,y+h)-o.ty;
}
void overlay_text(const Geometry& geo,const std::string& text,int x,int y) {overlay_next.lines.push_back({text,canvas_x(geo,x),canvas_y(geo,y)});}
// The slot's picture (bilinear) in the gold frame of the menu art with a dark
// outer line; an incompatible slot darkened. Then the text lines.
void overlay_compose(const Overlay& o) {
    static uint32_t argb[SAVESTATE_THUMB_W*SAVESTATE_THUMB_H];
    const bool picture=o.slot>=0 && savestate_read_thumb(o.slot,argb,SAVESTATE_THUMB_W,SAVESTATE_THUMB_H);
    const int lh=(ttk_font_line_height(steel_set)+1)*steel_scale;
    int x0=canvas_w,y0=canvas_h,x1=0,y1=0;
    auto grow=[&](int a,int b,int c,int d) {x0=std::min(x0,a);y0=std::min(y0,b);x1=std::max(x1,c);y1=std::max(y1,d);};
    if(picture) grow(o.tx,o.ty,o.tx+o.tw,o.ty+o.th);
    for(const OverlayText& t:o.lines) {const int w=steel_width(t.text)+1;grow(t.x-w/2,t.y,t.x-w/2+w,t.y+lh);}
    x0=std::max(x0,0);y0=std::max(y0,0);x1=std::min(x1,canvas_w);y1=std::min(y1,canvas_h);
    if(x1<=x0 || y1<=y0) {overlay_w=overlay_h=0;return;}
    overlay_x=x0;overlay_y=y0;overlay_w=x1-x0;overlay_h=y1-y0;
    overlay_px.assign(size_t(overlay_w)*overlay_h,0u);
    if(picture) for(int y=0;y<o.th;++y) for(int x=0;x<o.tw;++x) {
        const int px=o.tx+x-x0, py=o.ty+y-y0;
        if(px<0 || py<0 || px>=overlay_w || py>=overlay_h) continue;
        const bool outer=x<2 || y<2 || x>=o.tw-2 || y>=o.th-2;
        const bool inner=!outer && (x<5 || y<5 || x>=o.tw-5 || y>=o.th-5);
        uint32_t c;
        if(outer) c=0x160f33u;
        else if(inner) c=0xc9861au;
        else {
            const float fx=std::max(0.f,(x+0.5f)*SAVESTATE_THUMB_W/o.tw-0.5f), fy=std::max(0.f,(y+0.5f)*SAVESTATE_THUMB_H/o.th-0.5f);
            const int ix=std::min((int)fx,SAVESTATE_THUMB_W-1), iy=std::min((int)fy,SAVESTATE_THUMB_H-1);
            const int jx=std::min(ix+1,SAVESTATE_THUMB_W-1), jy=std::min(iy+1,SAVESTATE_THUMB_H-1);
            const float ax=fx-ix, ay=fy-iy;
            c=0;
            for(int sh=0;sh<24;sh+=8) {
                auto ch=[&](int sx,int sy) {return float((argb[sy*SAVESTATE_THUMB_W+sx]>>sh)&255);};
                const float v=(ch(ix,iy)*(1-ax)+ch(jx,iy)*ax)*(1-ay)+(ch(ix,jy)*(1-ax)+ch(jx,jy)*ax)*ay;
                c|=uint32_t(std::min(255.f,v+0.5f))<<sh;
            }
            if(o.dim) c=(c>>2)&0x3f3f3fu;
        }
        overlay_px[size_t(py)*overlay_w+px]=0xff000000u|c;
    }
    // Each line at 1x, one pixel between letters, then scaled up.
    for(const OverlayText& t:o.lines) {
        const int w=steel_width(t.text)/steel_scale+1, h=ttk_font_line_height(steel_set)+1;
        std::vector<uint32_t> line(size_t(w)*h,0u);
        ttk_font_draw_spaced(steel_set,t.text.c_str(),line.data(),w,h,0,0,1);
        const int left=t.x-w*steel_scale/2-x0, top=t.y-y0;
        for(int y=0;y<h*steel_scale;++y) for(int x=0;x<w*steel_scale;++x) {
            const uint32_t c=line[size_t(y/steel_scale)*w+x/steel_scale];
            const int px=left+x, py=top+y;
            if((c>>24) && px>=0 && py>=0 && px<overlay_w && py<overlay_h) overlay_px[size_t(py)*overlay_w+px]=c;
        }
    }
}
// The page drew its image: compose what changed and show it.
void overlay_commit() {
    std::lock_guard<std::mutex> lock(thumb_mutex);
    Overlay o=overlay_next;overlay_next=Overlay{};
    if(o.slot<0 && o.lines.empty()) {overlay_on=false;overlay_valid=false;return;}
    if(!overlay_valid || !(o==overlay_shown)) {overlay_compose(o);overlay_shown=o;overlay_valid=true;}
    overlay_on=overlay_w>0;overlay_until=SDL_GetTicks()+250;
}
// A string of the game's text table as host text.
std::string text_of(int id) {
    const uint32_t s=psx_mod_read_word(strings+4u*(uint32_t)id);
    std::string t;
    for(uint32_t a=s;s>=0x80010000u && s<0x80200000u && t.size()<40;++a) {const char ch=(char)psx_mod_read_byte(a);if(!ch) break;t+=ch;}
    return t;
}
std::string slot_name(int slot) {
    const SlotInfo& s=slot_info[slot];
    if(!s.exists) return "EMPTY";
    const int id=s.named ? half(level_names+2*s.level) : 0;
    const std::string name=id>0 && id<=text_last ? text_of(id) : "";
    return name.empty() ? "SAVED GAME" : name; // saved before D24B, or outside gameplay
}
std::string slot_time(int slot) {
    std::time_t t=std::time_t(slot_info[slot].stamp);std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm,&t);
#else
    localtime_r(&t,&tm);
#endif
    char line[32];std::strftime(line,sizeof line,"%Y-%m-%d %H:%M",&tm);
    return line;
}
// Layout A (user-approved mockup, 2026-10-20): slots on the left in the main
// menu's font, the chosen one's thumbnail and details on the right.
// D19Q: SAVE GAME is the same page with every slot open.
void draw_load(Guest& g,const Geometry& geo,bool save=false) {
    if(save) g.say("SAVE GAME",0,heading_y(geo),font_menu|centre);
    else g.say_id(text_load,0,heading_y(geo),font_menu|centre);
    const int x=-geo.w/2+36, y0=-66, spacing=14;
    for(int j=0;j<load_rows_visible && top+j<SAVESTATE_SLOTS;++j) {
        const int i=top+j, y=y0+spacing*j;
        char number[16];std::snprintf(number,sizeof number,"%02d",i+1);
        const std::string name=slot_name(i);
        if(i==cursor || loadable(i) || save) {
            g.say(number,x,y,font_small|(i==cursor?blue:0));
            g.say(name.c_str(),x+28,y,font_small|(i==cursor?blue:0));
        } else {g.say_dim(number,x,y,font_small);g.say_dim(name.c_str(),x+28,y,font_small);}
    }
    if(top>0 || top+load_rows_visible<SAVESTATE_SLOTS)
        g.call(fn_sprite,sprites+16*icon_select,(uint32_t)(x-26),(uint32_t)(y0+spacing*load_rows_visible/2-8),0,100);
    // 4:3 on screen: the menu area is 4:3, so w = h * geo.w / geo.h.
    const SlotInfo& s=slot_info[cursor];
    const int cx=geo.w/4+12, th=84, tw=th*geo.w/geo.h, ty=-68;
    if(!s.exists) {
        if(save) g.say("NEW SAVE",cx,ty+th/2-4,font_small|centre);
        else g.say_dim("EMPTY",cx,ty+th/2-4,font_small|centre);
    } else {
        overlay_thumb(cursor,!s.compatible,geo,cx-tw/2,ty,tw,th);
        if(!s.compatible) {
            g.say(save ? "SAVE OVER" : "CANNOT LOAD",cx,ty+th+16,font_small|centre);
            g.say_dim("FROM ANOTHER BUILD",cx,ty+th+30,font_small|centre);
        } else {
            // D20B: a save on the statistics screen: LEVEL COMPLETE, not the era.
            if(s.complete) g.say("LEVEL COMPLETE",cx,ty+th+16,font_small|centre);
            else if(s.named && *level_era(s.level)) g.say(level_era(s.level),cx,ty+th+16,font_small|centre);
            // The save time in Console steel (the guest font without the font pack).
            if(steel_text()) overlay_text(geo,slot_time(cursor),cx,ty+th+32);
            else g.say(slot_time(cursor).c_str(),cx,ty+th+30,font_small|centre);
        }
    }
    overlay_commit();
    const int py=prompt_y(geo);
    if(!load_note.empty()) g.say(load_note.c_str(),0,py-18,font_small|centre);
    g.prompt_id(icon_select,-(geo.w/4),py,text_select);
    g.prompt_id(icon_cross,0,py,text_confirm);
    g.prompt_id(icon_triangle,geo.w/4,py,text_return);
}
// D19Q: OVERWRITE THIS SAVE?, the slot's picture smaller to leave room for
// YES and NO (NO first).
void draw_save_ask(Guest& g,const Geometry& geo) {
    g.say("SAVE GAME",0,heading_y(geo),font_menu|centre);
    const int th=62, tw=th*geo.w/geo.h, ty=-76;
    overlay_thumb(cursor,!slot_info[cursor].compatible,geo,-tw/2,ty,tw,th);
    overlay_commit();
    char slot[24];std::snprintf(slot,sizeof slot,"SLOT %02d",cursor+1);
    const std::string line=slot_info[cursor].named ? std::string(slot)+" - "+slot_name(cursor) : std::string(slot);
    g.say(line.c_str(),0,ty+th+14,font_small|centre);
    g.say("OVERWRITE THIS SAVE?",0,ty+th+32,font_menu|centre);
    g.say_id(text_yes,0,ty+th+54,font_menu|centre|(save_choice==0?blue:0));
    g.say_id(text_no,0,ty+th+72,font_menu|centre|(save_choice==1?blue:0));
    const int py=prompt_y(geo);
    g.prompt(icon_select,"SELECT",-geo.w/4,py);
    g.prompt(icon_cross,"CONFIRM",0,py);
    g.prompt(icon_triangle,"CANCEL",geo.w/4,py);
}
// D19S: DELETE SAVED GAME: and the slot's name (no quotes, D19T1), under the
// slot's picture as OVERWRITE THIS SAVE? shows it; YES / NO with NO first.
void draw_delete_ask(Guest& g,const Geometry& geo) {
    if(delete_from==Page::Save) g.say("SAVE GAME",0,heading_y(geo),font_menu|centre);
    else g.say_id(text_load,0,heading_y(geo),font_menu|centre);
    const int th=62, tw=th*geo.w/geo.h, ty=-76;
    overlay_thumb(cursor,!slot_info[cursor].compatible,geo,-tw/2,ty,tw,th);
    char slot[24];std::snprintf(slot,sizeof slot,"SLOT %02d",cursor+1);
    const std::string name=slot_info[cursor].named ? std::string(slot)+" - "+slot_name(cursor) : std::string(slot);
    g.say("DELETE SAVED GAME:",0,ty+th+14,font_menu|centre);
    // The save's name in Console steel, as the save time on the list.
    if(steel_text()) overlay_text(geo,name,0,ty+th+34);
    else g.say(name.c_str(),0,ty+th+32,font_small|centre);
    overlay_commit();
    g.say_id(text_yes,0,ty+th+54,font_menu|centre|(delete_choice==0?blue:0));
    g.say_id(text_no,0,ty+th+72,font_menu|centre|(delete_choice==1?blue:0));
    const int py=prompt_y(geo);
    g.prompt(icon_select,"SELECT",-geo.w/4,py);
    g.prompt(icon_cross,"CONFIRM",0,py);
    g.prompt(icon_triangle,"CANCEL",geo.w/4,py);
}
// LOAD THIS GAME?, as the original LOAD screen asks, with CONFIRM and CANCEL.
void draw_load_ask(Guest& g,const Geometry& geo) {
    g.say_id(text_load,0,heading_y(geo),font_menu|centre);
    const int th=96, tw=th*geo.w/geo.h, ty=-80;
    overlay_thumb(cursor,false,geo,-tw/2,ty,tw,th);
    char slot[24];std::snprintf(slot,sizeof slot,"SLOT %02d",cursor+1);
    const std::string line=slot_info[cursor].named ? std::string(slot)+" - "+slot_name(cursor) : std::string(slot);
    g.say(line.c_str(),0,ty+th+14,font_small|centre);
    g.say("LOAD THIS GAME?",0,ty+th+34,font_menu|centre);
    if(load_from!=Page::Main) {
        static const char warning[]="UNSAVED PROGRESS WILL BE LOST";
        if(steel_text()) overlay_text(geo,warning,0,ty+th+54);
        else g.say_dim(warning,0,ty+th+52,font_small|centre);
    }
    overlay_commit();
    const int py=prompt_y(geo);
    g.prompt_id(icon_cross,-(geo.w/4),py,text_confirm);
    g.prompt_id(icon_triangle,geo.w/4,py,text_cancel);
}

// ---- D20B: the statistics screen, as 0x800277dc and 0x80027214 draw it ----
// The original: the level's name (small font, blue) 30 above the first row,
// at y0 + (h - 60) / 2. One player: labels left-aligned at -(w/4) - 20,
// values right-aligned at w/4 + 20, 15 apart: kills, time (mm:ss), continues,
// then secrets (not the bosses 27-30) and the challenge item (levels 0, 2, 5,
// 7, 9, 11). Challenge stages: COMPLETED or FAILED, centred on the first row.
// D20F: the choices are a list in the menu font (CONTINUE, SAVE GAME; 19
// apart, blue highlighted) at row + stats_list_y, with SELECT / CONFIRM.
// Nothing of it before the wait ends.
// D20D (layout approved 2026-10-21): the name 44 above the row and
// DIFFICULTY: under it; the rows 13 apart from 12 above it (the original's
// columns), adding BEST TIME under TIME TAKEN, MISSION ITEMS (levels that
// have them) before the challenge item and HEALTH / ARMOR last. The list
// keeps its place.
constexpr int stats_lift=25, stats_list_y=95;
constexpr int stats_title=44, stats_step=13;
// D20E: the level's enemy total, by the rule the level load 0x800242b0 uses
// for its enemies-left count 0x800d253c: placement rows (*0x800de720, 48
// bytes, count 0x800de724) whose type is not Duke (0x20), has type flag bit
// 0 (*0x800d2660 + 28*type) and is not in the load's exclusion list. The
// count itself is no use here: dancers are counted but never subtracted.
// The rows stay through deaths, Continue and the statistics screen. -1 when
// the table does not read sensibly.
int enemy_total() {
    static constexpr uint32_t excluded[]={0x7a,0x7b,0xa7,0xcc,0x112,0x13f,0x158,0x21a,0x21c,0x243,0x25d};
    const uint32_t table=psx_mod_read_word(0x800de720),count=psx_mod_read_word(0x800de724),types=psx_mod_read_word(0x800d2660);
    auto inside=[](uint32_t a,uint32_t n) {return !(a&3) && a>=0x80010000 && a<=0x80200000-n;};
    if(!count || count>1024 || !inside(table,count*48) || !inside(types,28*1100)) return -1;
    int total=0;
    for(uint32_t i=0;i<count;++i) {
        const uint32_t type=psx_mod_read_word(table+48*i);
        if(type>=1100) return -1;
        if(type==0x20 || !(psx_mod_read_word(types+28*type)&1) ||
           std::find(std::begin(excluded),std::end(excluded),type)!=std::end(excluded)) continue;
        ++total;
    }
    return total;
}
void draw_stats(Guest& g,const Geometry& geo) {
    const int n=level(), row=(geo.h-60)/2+geo.y0-stats_lift, title=row-stats_title;
    if(n>=0 && n<32) {
        const int id=half(level_names+2*n);
        if(id>0 && id<=text_last) g.say_id(id,0,title,font_small|centre|blue);
    }
    const int d=std::clamp<int>(half(diff_cursor),0,3);
    g.say(("DIFFICULTY: "+text_of((int)psx_mod_read_word(diff_ids+4*d))).c_str(),0,title+14,font_small|centre);
    if(challenge()) g.say_id((psx_mod_read_byte(challenges_done)>>(n-21))&1 ? text_completed : text_failed,0,row,font_small|centre);
    else {
        const int lx=-(geo.w/4)-20, vx=geo.w/4+20;
        int y=title+32;
        char value[48];
        auto line=[&](const char* label,const char* text,uint32_t vflags=0) {g.say(label,lx,y,font_small);g.say(text,vx,y,font_small|right|vflags);y+=stats_step;};
        auto line_id=[&](int label,const char* text) {g.say_id(label,lx,y,font_small);g.say(text,vx,y,font_small|right);y+=stats_step;};
        auto clock=[&](uint32_t ms) {const unsigned t=ms/1000;std::snprintf(value,sizeof value,"%02u:%02u",t/60,t%60);return value;};
        const int enemies=enemy_total(), kills=(int)half(player+0x3b6);
        if(enemies>0) std::snprintf(value,sizeof value,"%d OF %d",kills,enemies);
        else std::snprintf(value,sizeof value,"%d",kills);
        line_id(text_kills_label,value);
        line_id(text_time_label,clock(psx_mod_read_word(player+0x398)));
        if(stats_new_best) line("BEST TIME:","NEW BEST!",blue);
        else line("BEST TIME:",stats_best_ms ? clock(stats_best_ms) : "--:--");
        std::snprintf(value,sizeof value,"%d",(int)psx_mod_read_byte(player+0x279));
        line_id(text_continues_label,value);
        if((unsigned)(n-27)>=4u) {
            std::snprintf(value,sizeof value,"%d OF %d",(int)half(player+0x3b0),(int)half(secrets_total));
            line_id(text_secrets_label,value);
        }
        uint16_t flags[17]{};
        for(unsigned i=6;i<17;++i) flags[i]=psx_mod_read_half(player+0x354+4*i);
        int got=0,total=0;
        mission_tally((unsigned)n,flags,got,total);
        if(total) {std::snprintf(value,sizeof value,"%d OF %d",got,total);line("MISSION ITEMS:",value);}
        if(n==0 || n==2 || n==5 || n==7 || n==9 || n==11) {
            g.say_id(text_challenge_label,lx,y,font_small);
            g.say_id(psx_mod_read_byte(challenge_found) ? text_yes : text_no,vx,y,font_small|right);
            y+=stats_step;
        }
        std::snprintf(value,sizeof value,"%d%% / %d%%",(int16_t)half(player+0x32)/100,(int16_t)half(player+0x234)/100);
        line("HEALTH / ARMOR:",value);
    }
    if(stats_wait>0) return;
    const int ly=row+stats_list_y, items=stats_items();
    g.say("CONTINUE",0,ly,font_menu|centre|(cursor==0 || items<2 ? blue : 0));
    if(items>1) g.say("SAVE GAME",0,ly+19,font_menu|centre|(cursor==1 ? blue : 0));
    const int py=prompt_y(geo);
    if(items>1) {
        g.prompt(icon_select,"SELECT",-geo.w/8,py);
        g.prompt(icon_cross,"CONFIRM",geo.w/8,py);
    } else g.prompt(icon_cross,"CONFIRM",0,py);
}

// ---- D19J: the title pages' actions, in the frontend tick ----
// Each page's entry is what its handler does while its fresh flag is set.
// The main menu on the original's cursor (the item a page was entered from).
void enter_main() {
    open_list(Page::Main,Act::OnePlayer);
    cursor=std::clamp<int>(half(fe_cursor),0,(int)list.size()-2);
    title_enter=true;
}
void enter_difficulty() {
    page=Page::Difficulty;
    if(psx_mod_read_word(diff_fresh)) {psx_mod_write_word(diff_fresh,0);psx_mod_write_half(diff_saved,psx_mod_read_half(diff_cursor));}
}
void enter_two() {
    page=Page::TwoPlayer;
    if(psx_mod_read_word(two_fresh)) {
        psx_mod_write_half(two_cursor,0);
        const uint32_t m=psx_mod_read_word(split_mode);
        if(m==1) psx_mod_write_half(two_split,1);
        else if(m==2) psx_mod_write_half(two_split,0);
        psx_mod_write_word(two_fresh,0);
    }
}
void enter_character() {
    page=Page::Character;
    if(psx_mod_read_word(char_fresh)) {
        for(uint32_t p=0;p<2;++p) psx_mod_write_word(char_choice+4*p,(uint32_t)(int32_t)half(char_saved+2*p));
        psx_mod_write_word(char_locks,0);psx_mod_write_word(char_fresh,0);
    }
}
// Wrap v + step into [0, n), as the handlers do for their values.
int16_t wrap_add(int16_t v,int step,int n) {const int r=v+step;return (int16_t)(r>=n ? 0 : r<0 ? n-1 : r);}
// The original main menu's level choice (its fifth item): 0-3, 5-12, 21-29.
void step_level(int dir) {
    int v=half(level_choice)+dir;
    if(dir>0) {if(v>=32) v=0;if(v==13) v=21;else if(v==4) v=5;else if(v==30) v=0;}
    else {if(v<0) v=31;if(v==20) v=12;else if(v==4) v=3;else if(v==31) v=29;}
    psx_mod_write_half(level_choice,(uint16_t)v);
}
// One key on a title page: the original handler's response. Returns the sound.
int title_command(Cmd cmd) {
    switch(page) {
    case Page::Main: {
        const int n=(int)list.size();
        const Act a=list[cursor].act;
        int s=-1;
        switch(cmd) {
        case Cmd::Up: cursor=(cursor+n-1)%n;s=1;break;
        case Cmd::Down: cursor=(cursor+1)%n;s=1;break;
        case Cmd::Left: case Cmd::Right: if(a==Act::Level) {step_level(cmd==Cmd::Right ? 1 : -1);s=1;} break;
        case Cmd::Back: break; // nothing, as the original
        case Cmd::Confirm:
            s=1;
            switch(a) {
            case Act::OnePlayer: case Act::Level: psx_mod_write_half(players_choice,1);enter_difficulty();break;
            case Act::TwoPlayer: psx_mod_write_half(players_choice,2);enter_two();break;
            case Act::Load: psx_mod_write_half(players_choice,1);open_load(Page::Main);break; // D19K
            case Act::Options:
                // State 11: sub-screen 9, which D19 makes ours in this very tick.
                page=Page::Options;screen=-1;cursor=0;top=0;back_label=contexts[0].back_label;
                psx_mod_write_half(contexts[0].sub,contexts[0].options);
                break;
            default: ask_from=Page::Main;ask_act=Act::QuitGame;page=Page::Ask;cursor=1;break; // QUIT GAME?
            }
            break;
        }
        if(page==Page::Main && list[cursor].act!=Act::QuitGame) psx_mod_write_half(fe_cursor,(uint16_t)cursor);
        return s;
    }
    case Page::Difficulty: {
        const int16_t c=half(diff_cursor);
        switch(cmd) {
        case Cmd::Up: psx_mod_write_half(diff_cursor,(uint16_t)wrap_add(c,-1,4));return 1;
        case Cmd::Down: psx_mod_write_half(diff_cursor,(uint16_t)wrap_add(c,1,4));return 1;
        case Cmd::Back:
            psx_mod_write_half(diff_cursor,psx_mod_read_half(diff_saved));psx_mod_write_word(diff_fresh,1);
            enter_main();return 0;
        case Cmd::Confirm: pending_state=state_difficulty;handoff=Handoff::Pending;return -1; // its handler sounds
        default: return -1;
        }
    }
    case Page::TwoPlayer: {
        const int16_t c=half(two_cursor);
        switch(cmd) {
        case Cmd::Up: psx_mod_write_half(two_cursor,(uint16_t)wrap_add(c,-1,4));return 1;
        case Cmd::Down: psx_mod_write_half(two_cursor,(uint16_t)wrap_add(c,1,4));return 1;
        case Cmd::Left: case Cmd::Right: {
            const int d=cmd==Cmd::Right ? 1 : -1;
            switch(c) {
            case 0: psx_mod_write_half(two_level,(uint16_t)wrap_add(half(two_level),d,6));break;
            case 1: psx_mod_write_half(two_time,(uint16_t)(d>0 ? (half(two_time)+5>=16 ? 0 : half(two_time)+5) : (half(two_time)-5<0 ? 15 : half(two_time)-5)));break;
            case 2: psx_mod_write_half(two_kills,(uint16_t)(d>0 ? (half(two_kills)+5>=16 ? 0 : half(two_kills)+5) : (half(two_kills)-5<0 ? 15 : half(two_kills)-5)));break;
            case 3: psx_mod_write_half(two_split,(uint16_t)wrap_add(half(two_split),1,2));break; // both keys toggle
            }
            return 1;
        }
        case Cmd::Back: psx_mod_write_word(two_fresh,1);enter_main();return 0;
        case Cmd::Confirm: {
            const int16_t split=half(two_split);
            if(split==0) psx_mod_write_word(split_mode,2);
            else if(split==1) psx_mod_write_word(split_mode,1);
            psx_mod_write_word(two_fresh,1);
            psx_mod_write_half(level_choice,(uint16_t)(half(two_level)+15));
            enter_character();
            return 1;
        }
        }
        return -1;
    }
    case Page::Character: {
        // One keyboard for both players: player 1 chooses and confirms, then
        // player 2 (the original reads each player's own pad).
        const uint32_t locks=psx_mod_read_word(char_locks);
        const int row=!(locks&1) ? 0 : !(locks&2) ? 1 : -1;
        switch(cmd) {
        case Cmd::Left: case Cmd::Right: {
            if(row<0) return -1;
            const uint32_t a=char_choice+4*row, v=psx_mod_read_word(a);
            psx_mod_write_word(a,cmd==Cmd::Right ? (v+1>=6 ? 0 : v+1) : (v==0 || v>=6 ? 5 : v-1));
            return 1;
        }
        case Cmd::Confirm:
            if(row>=0) {psx_mod_write_word(char_locks,locks|(1u<<row));return 1;}
            pending_state=state_character;handoff=Handoff::Pending;return -1; // its handler sounds
        case Cmd::Back:
            if(locks&2) {psx_mod_write_word(char_locks,locks&~2u);return 4;}
            if(locks&1) {psx_mod_write_word(char_locks,locks&~1u);return 4;}
            psx_mod_write_word(char_fresh,1);enter_two();return 0;
        default: return -1;
        }
    }
    default: return -1;
    }
}
} // namespace

bool gameplay_tick(CPUState* cpu);
bool title_tick(CPUState* cpu,const MenuContext* c);
void draw_page(CPUState* cpu,const MenuContext* c);

// Frontend state 12 calls 0x800220f4 each tick just before the sub-screen
// dispatch: the place to apply the keys' actions (the original screen then
// runs in this very tick) and to take over the original OPTIONS before its
// handler draws anything.
bool trace() {static const bool v=std::getenv("DNTTK_MENU_TRACE")!=nullptr;return v;}
void game_menu_tick(CPUState* cpu) {
    if(trace()) {static uint32_t n;if(n++<40) std::fprintf(stderr,"[TTK menu] tick pc=%08x mode %u enabled %d replay %d precise %d ls %d bail %d\n",
        cpu->pc,psx_mod_read_half(game_mode),enabled(),psx_mod_replay_active(),g_precise_mode,g_ls_mode,g_psx_call_bail);}
    if(!enabled() || psx_mod_replay_active() || g_precise_mode || g_ls_mode || g_psx_call_bail) return;
    if(handoff==Handoff::Armed) {
        // D19J: the original difficulty (8) or character (9) handler is about to
        // run: one Cross press on pad 1, nothing else new. It draws the page we
        // showed and starts the game, as on the original's own Cross.
        const uint32_t ra=cpu->gpr[31];
        const uint16_t state=psx_mod_read_half(fe_state);
        if(psx_mod_read_half(game_mode)==0 && ((ra==from_difficulty && state==state_difficulty) || (ra==from_character && state==state_character))) {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            for(uint32_t pad:{0u,0x54u}) for(uint32_t at:history_checked) {
                const uint32_t a=pad_history+pad+at, w=psx_mod_read_word(a);
                if(w&3u) psx_mod_write_word(a,w&~3u);
            }
            psx_mod_write_word(pad_history+history_cross,(psx_mod_read_word(pad_history+history_cross)&~3u)|1u);
            handoff=Handoff::None;++finals;
            if(trace()) std::fprintf(stderr,"[TTK menu] title handoff %u\n",state);
        }
        return;
    }
    const MenuContext* c=current_context();
    if(!c) return;
    ensure_init();
    if(!authentic()) return;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    savestate_check();
    // D19I: in gameplay our pages are drawn here, after the actions, like the
    // original's pause text (see draw_page).
    struct DrawAfter {CPUState* cpu;const MenuContext* c;~DrawAfter() {if(c->mode==1) draw_page(cpu,c);}} draw_after{cpu,c};
    // D19O: the blocking load of a demo scene reads no pad; our menu still does.
    if(c->mode==0 && owned && cpu->gpr[31]!=from_options && psx_mod_read_half(demo_load)==0) input_menu_poll();
    if(pending_sound>=0) {
        busy=true;
        {Guest g(cpu);g.call(fn_sound,(uint32_t)pending_sound,0,0,0);}
        busy=false;
        pending_sound=-1;
    }
    if(pending_speed>=0) {
        // D19N: GAME SPEED confirmed: the original's CONFIRM call.
        busy=true;
        {Guest g(cpu);g.call(fn_game_speed,(uint32_t)pending_speed,0,0,0);}
        busy=false;
        pending_speed=-1;
    }
    if(c->mode==1) {
        if(gameplay_tick(cpu)) return;
    }
    uint16_t sub=psx_mod_read_half(c->sub);
    if(pending_sub!=-1) {
        // D19J: back from OPTIONS to our main menu keeps the keys.
        const bool title=pending_sub==title_sub && sub==ours_sub;
        if(sub==ours_sub) {
            if(pending_cursor>=0) psx_mod_write_half(c->cursor,(uint16_t)pending_cursor);
            if(c->mode==0) psx_mod_write_half(controller_pad,0);
            psx_mod_write_half(c->sub,(uint16_t)(pending_sub==-2 ? c->back : pending_sub));
        }
        pending_sub=pending_cursor=-1;owned=title;screen=-1;
        if(!title && title_page(page)) page=Page::Options;
        return;
    }
    if(c->mode==0) {
        if(sub==title_sub) {
            if(title_tick(cpu,c)) return;
            sub=psx_mod_read_half(c->sub); // OPTIONS chosen: sub-screen 9, below
        } else if(page!=Page::Keys && !(page==Page::Ask && ask_from==Page::Keys)) page=Page::Options;
    }
    if(sub==c->options) {
        // The original OPTIONS (entered, or back from SOUND, CONTROLLER or GAME
        // SETTINGS): ours instead, that item highlighted.
        cursor=std::clamp<int>((int16_t)psx_mod_read_half(c->cursor),0,2);
        screen=-1;top=0;owned=true;back_label=c->back_label;++entries;
        psx_mod_write_half(c->sub,ours_sub);
    } else if(sub==ours_sub && !owned) {
        // A savestate saved on one of our screens: start again at OPTIONS.
        cursor=0;screen=-1;top=0;owned=true;back_label=c->back_label;
    } else if(sub!=ours_sub) owned=false;
}

// D19H, in gameplay (mode 1) at the pause dispatch, before the screen's
// handler runs: our actions, and the original pause menu and death screen
// replaced by our lists before they draw. True when the tick is done.
bool gameplay_tick(CPUState* cpu) {
    uint16_t sub=psx_mod_read_half(pause_screen);
    if(resume_load) {
        resume_load=false; // one load, one resume; never consume a later Escape pause
        if(psx_mod_read_half(paused) && sub!=death_top && sub!=death_sub && !duke_dead()) {
            // Includes original pause/options screens and metadata-free F7 saves.
            // Run the normal resume finalizer, including sound/music and flags.
            sub=ours_sub;
            psx_mod_write_half(pause_screen,sub);
            pending_final=Final::Resume;
            quiet_resume=true;
        }
    }
    if(pending_final!=Final::None) {
        const Final f=pending_final;
        pending_final=Final::None;
        const bool pause=f==Final::Resume || f==Final::PauseRestart || f==Final::PauseMainMenu;
        if(sub==(pause ? ours_sub : death_sub)) {
            // D19I: this image still shows the page, as the original's
            // handler draws its screen in the tick that acts on the press.
            // D19Q: not when a loaded SAVE GAME save resumes at once.
            if(!quiet_resume) draw_page(cpu,&contexts[1]);
            quiet_resume=false;
            busy=true;
            {
                Guest g(cpu);
                // The handler's part (pause 0x80085c04 / 0x80086130, death
                // 0x8008692c / 0x80086c04 / 0x80086aa4).
                g.call(fn_sound,f==Final::Resume ? 0u : 1u,0,0,0);
                switch(f) {
                case Final::Resume: psx_mod_write_half(pause_reset,1);break;
                case Final::PauseRestart: psx_mod_write_half(pause_reset,1);psx_mod_write_half(pause_screen,restart_code);break;
                case Final::PauseMainMenu: psx_mod_write_half(pause_reset,1);psx_mod_write_half(pause_screen,main_menu_code);break;
                case Final::DeathContinue:
                    if(challenge()) {psx_mod_write_byte(continues,(uint8_t)(psx_mod_read_byte(continues)-1));g.call(fn_event,event_replay,0,0,0);}
                    else psx_mod_write_byte(continue_request,2);
                    psx_mod_write_half(death_reset,1);break;
                case Final::DeathRestart: g.call(fn_event,event_restart,0,0,0);psx_mod_write_half(death_reset,1);break;
                case Final::DeathQuit: psx_mod_write_half(death_reset,1);g.call(fn_event,challenge() ? event_restart : event_quit,0,0,0);break;
                case Final::None: break;
                }
                // The handler returned 0 (resume, continue, restart and quit
                // from death): the main loop's resume (0x80026950-0x80026994).
                if(f!=Final::PauseRestart && f!=Final::PauseMainMenu) {
                    const uint32_t flags=player+player_size*psx_mod_read_half(controller_pad)+player_flags;
                    psx_mod_write_half(paused,0);
                    psx_mod_write_word(flags,psx_mod_read_word(flags)|0x4000u);
                    g.call(fn_resume_sound,0,0,0,0);g.call(fn_resume_music,0,0,0,0);
                    resumed=true;
                }
            }
            busy=false;
            ++finals;
            if(trace()) std::fprintf(stderr,"[TTK menu] final %d\n",(int)f);
        }
        owned=false;page=Page::Options;screen=-1;return_item=Act::Resume;direct_open=false;
        return true;
    }
    if(sub==pause_top) {
        // D19Q: the displayed image is still the last of play.
        pause_thumb_ok=savestate_grab_thumb(pause_thumb)!=0;
        open_list(Page::Pause,return_item);return_item=Act::Resume;
        // D20: F2 / F3 (or a quick key with no slot yet): straight to the page.
        if(direct!=Page::Options && SDL_GetTicks()<=direct_until) {
            if(direct==Page::Save) open_save(); else open_load(Page::Pause);
            direct_open=true;
        }
        direct=Page::Options;
        owned=true;back_label="RESUME";++entries;
        input_release_for_menu();
        psx_mod_write_half(pause_screen,ours_sub);
        return true;
    }
    if(sub==death_top) {
        open_list(Page::Death,Act::Continue);owned=true;++entries;
        input_release_for_menu();
        psx_mod_write_half(pause_screen,death_sub);
        return true;
    }
    if(!sub) return_item=Act::Resume;
    if(!owned && !resumed && (sub==ours_sub || sub==death_sub)) {
        // A savestate saved on one of our screens: the list again. (Not after
        // our own resume: the level start after the death screen's RESTART
        // LEVEL runs this tick through 0x800220f4 before the composition
        // clears the value.)
        open_list(sub==death_sub ? Page::Death : Page::Pause,Act::Resume);
        owned=true;back_label="RESUME";
        if(!thumb_from_load) pause_thumb_ok=false; // an F7 load: no picture of its play
        thumb_from_load=false;
        return true;
    }
    if(sub==death_sub) return true;
    if(page!=Page::Options && sub==ours_sub) return true;
    page=Page::Options;
    return false; // OPTIONS and its screens: D19's own handling
}

// D19J: the title pages, at state 12's call before its sub-screen dispatch
// (the pad was read just before). True when the tick is done.
bool title_tick(CPUState* cpu,const MenuContext* c) {
    if(cpu->gpr[31]!=from_options) {
        // A load loop's backdrop (D19O): the menu is drawn over the loading
        // logo, so it moves there too. Confirm and Back wait for a normal tick.
        if(!owned || !title_page(page)) return true;
        size_t i=0;
        busy=true;
        {
            Guest g(cpu);
            for(;i<title_cmds.size() && title_cmds[i]!=Cmd::Confirm && title_cmds[i]!=Cmd::Back && title_page(page);++i) {
                const int s=title_command(title_cmds[i]);
                if(s>=0) g.call(fn_sound,(uint32_t)s,0,0,0);
            }
        }
        busy=false;
        title_cmds.erase(title_cmds.begin(),title_cmds.begin()+(std::ptrdiff_t)i);
        return true;
    }
    if(!owned) {
        // A savestate saved on one of our pages: the main menu again.
        enter_main();owned=true;back_label=c->back_label;title_cmds.clear();
    }
    if(!title_page(page) && page!=Page::Ask && page!=Page::Load && page!=Page::LoadAsk && page!=Page::DeleteAsk) return false;
    busy=true;
    {
        Guest g(cpu);
        size_t i=0;
        for(;i<title_cmds.size() && pending_state<0 && title_page(page);++i) {
            const int s=title_command(title_cmds[i]);
            if(s>=0) g.call(fn_sound,(uint32_t)s,0,0,0);
        }
        // D19K: keys queued behind the one that opened LOAD GAME (or QUIT
        // GAME?) act on that page, as they would have a moment later.
        for(;i<title_cmds.size() && pending_state<0 && (page==Page::Load || page==Page::LoadAsk || page==Page::DeleteAsk || page==Page::Ask);++i) {
            switch(title_cmds[i]) {
            case Cmd::Up: move(-1);break;
            case Cmd::Down: move(1);break;
            case Cmd::Confirm: confirm();break;
            case Cmd::Back: back(0);break;
            default: break;
            }
        }
        title_cmds.clear();
        if(title_enter) {
            // State 6: the idle time starts again and the level choice resets.
            psx_mod_write_half(fe_timer,idle_fields);psx_mod_write_half(level_choice,0);
            title_enter=false;
        }
        if(page==Page::Main && pending_state<0) {
            // State 7: back to PRESS START (state 4) after the idle time.
            if(title_activity) psx_mod_write_half(fe_timer,idle_fields);
            const uint16_t t=(uint16_t)(psx_mod_read_half(fe_timer)-psx_mod_read_half(fe_fields));
            psx_mod_write_half(fe_timer,t);
            if((int16_t)t<=0) pending_state=state_start;
        }
        title_activity=false;
    }
    busy=false;
    return psx_mod_read_half(c->sub)==title_sub;
}

// D19J: the frontend tick's entry (0x80022324), before the pad read and the
// state dispatch: the original main menu (state 6 or 7) and its pages (8, 9)
// become ours before their handlers draw anything, and a state chosen on our
// pages in the last tick is set.
void title_entry(CPUState* cpu) {
    (void)cpu;
    if(!enabled() || psx_mod_replay_active() || g_precise_mode || g_ls_mode || g_psx_call_bail) return;
    if(psx_mod_read_half(game_mode)!=0) return;
    ensure_init();
    if(!authentic()) return;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    savestate_check();
    if(handoff==Handoff::Armed) handoff=Handoff::None; // the handler never ran
    const uint16_t state=psx_mod_read_half(fe_state);
    // D19R: the card probe (state 1) and the file read (state 2) block the
    // main loop in the BIOS card driver, which advances once per VBlank, for
    // about 90 fields with nothing new on screen. Those VBlanks run unpaced:
    // the guest sees the same fields, the player no black pause. The budget
    // is renewed each tick and bounds a wait that never returns.
    if(state==state_card || state==state_save_file) {psx_mod_request_unpaced(card_unpaced_budget);card_unpaced=true;}
    else if(card_unpaced) {psx_mod_request_unpaced(0);card_unpaced=false;}
    if(pending_state>=0) {
        if(state==contexts[0].state_value && psx_mod_read_half(contexts[0].sub)==title_sub) {
            psx_mod_write_half(fe_state,(uint16_t)pending_state);
            if(handoff==Handoff::Pending) handoff=Handoff::Armed;
        } else handoff=Handoff::None;
        pending_state=-1;owned=false;page=Page::Options;screen=-1;title_cmds.clear();
        return;
    }
    if(handoff!=Handoff::None) return;
    // D19K: the memory card check without its PlayStation presentation: no
    // waits before the probe and the file read (the card work itself runs as
    // before, so the saved settings still load), and no card message: state 3
    // continues as its CONTINUE does.
    if(state==state_card && (int16_t)psx_mod_read_half(card_status)<-1) {psx_mod_write_half(card_status,0xffff);++card_skips;}
    else if(state==state_save_file && (int16_t)psx_mod_read_half(card_wait)<0) {psx_mod_write_half(card_wait,0);++card_skips;}
    else if(state==state_card_message) {psx_mod_write_half(fe_state,state_start);++card_skips;return;}
    if(state==6 || state==7) {
        enter_main();
    } else if(state==state_difficulty || state==state_character) {
        // A savestate saved on the original's pages: ours.
        if(state==state_character) enter_character();
        else if(half(players_choice)==2) enter_two();
        else enter_difficulty();
    } else return;
    owned=true;screen=-1;top=0;back_label=contexts[0].back_label;title_cmds.clear();title_activity=false;++entries;
    psx_mod_write_half(fe_state,contexts[0].state_value);
    psx_mod_write_half(contexts[0].sub,title_sub);
}

// Our page for this image, once per image. The title draws it at the view
// composition (game_menu_frame). D19I: gameplay draws it at the pause
// dispatch, after 0x80025f28 has put the paused world's darkening quad into the
// drawing chain that follows the composition: the original pause text is drawn
// there too. Drawn at the composition, our text went into the world's chain,
// under that quad and behind Duke, at about half brightness.
uint64_t skip_owned=0,skip_screen=0,skip_image=0,drawn_calls=0;uint32_t skip_ra=0;   // diagnostic
void draw_page(CPUState* cpu,const MenuContext* c) {
    if(!owned) {++skip_owned;skip_ra=cpu->gpr[31];return;}
    if(!our_screen(c,psx_mod_read_half(c->sub))) {++skip_screen;skip_ra=cpu->gpr[31];return;}
    if(c->mode==1 && !psx_mod_read_half(paused)) return; // only over the paused world
    const uint32_t image=psx_mod_read_word(image_counter);
    if(image==last_image && cpu->gpr[31]==last_draw_ra) {++skip_image;skip_ra=cpu->gpr[31];return;}
    ++drawn_calls;
    last_image=image;last_draw_ra=cpu->gpr[31];
    if(page!=Page::Load && page!=Page::LoadAsk && page!=Page::Save && page!=Page::SaveAsk && page!=Page::DeleteAsk) thumb_hide();
    busy=true;
    {
        Guest g(cpu);
        const Geometry geo=read_geometry();
        switch(page) {
        case Page::Pause: draw_pause(g,geo);break;
        case Page::Death: draw_death(g,geo);break;
        case Page::Brief: draw_brief(g,geo);break;
        case Page::Ask: draw_ask(g,geo);break;
        case Page::Options: if(screen<0) draw_options(g,geo); else draw_settings(g,geo);break;
        case Page::Main: draw_main(g,geo);break;
        case Page::Difficulty: draw_difficulty(g,geo);break;
        case Page::TwoPlayer: draw_two(g,geo);break;
        case Page::Character: draw_character(g,geo);break;
        case Page::Keys: draw_keys(g,geo);break;
        case Page::Load: draw_load(g,geo);break;
        case Page::LoadAsk: draw_load_ask(g,geo);break;
        case Page::Save: draw_load(g,geo,true);break;
        case Page::SaveAsk: draw_save_ask(g,geo);break;
        case Page::DeleteAsk: draw_delete_ask(g,geo);break;
        }
        // Diagnostics only (DNTTK_TEST_MENU_TEXT=<text>): the text in both
        // menu fonts at the top of every page (D19T glyph checks).
        static const char* test_text=std::getenv("DNTTK_TEST_MENU_TEXT");
        if(test_text) {g.say(test_text,0,-110,font_small|centre);g.say(test_text,0,96,font_menu|centre);}
    }
    busy=false;
    ++draws;
}

// D20B: the one-player statistics screen is ours (Modernized, the authentic
// code, not a replay, not the memory card's SAVE GAME page).
bool stats_ours() {
    return enabled() && !psx_mod_replay_active() && !g_precise_mode && !g_ls_mode && !g_psx_call_bail
        && psx_mod_read_half(game_mode)==2 && psx_mod_read_word(players)==1 && psx_mod_read_half(stats_page)!=1 && authentic();
}
// The original screen's text (0x8008b454 called from its drawing) draws the
// string's empty end: our page is drawn in its place.
void stats_text_entry(CPUState* cpu) {
    const uint32_t ra=cpu->gpr[31];
    if(!((ra>=stats_one_begin && ra<stats_one_end) || (ra>=stats_challenge_begin && ra<stats_challenge_end))) return;
    if(!stats_ours()) return;
    uint32_t end=cpu->gpr[4];
    for(int i=0;i<64 && psx_mod_read_byte(end);++i) ++end;
    if(!psx_mod_read_byte(end)) cpu->gpr[4]=end;
}
uint64_t stats_entries=0, stats_continues=0;   // diagnostic
uint32_t end_thumb[SAVESTATE_THUMB_W*SAVESTATE_THUMB_H];
bool end_thumb_ok=false;
// Gameplay ends towards the statistics screen: keep the level's last image.
void gameplay_exit_entry() {
    if(!enabled() || psx_mod_replay_active() || g_precise_mode || g_ls_mode || g_psx_call_bail) return;
    const uint8_t code=psx_mod_read_byte(end_code);
    if(psx_mod_read_word(players)!=1 || !(code==4 || (code==0xfe && challenge()))) return;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    end_thumb_ok=savestate_grab_thumb(end_thumb)!=0;
}
// The flip at the end of the statistics screen's tick (0x8001fcbc called from
// 0x80027df8): open the page, act, draw it into this image, and hold the
// original's timer so it never shows its prompts or reads the pad.
// ---- D20D: best times, per level and difficulty ----
// ttk-best-times.txt beside the savestates (never the memory card): one line
// "level difficulty milliseconds" per record. A finished level (not a
// savestate loaded onto the statistics screen) records its TIME TAKEN when it
// is the first or a shorter one; the screen then reads NEW BEST!.
std::string best_times_path() {
    const char* dir=savestate_dir();
    return dir && *dir ? std::string(dir)+"/ttk-best-times.txt" : std::string();
}
struct BestTime {int level,difficulty;uint32_t ms;};
std::vector<BestTime> best_times_read(const std::string& path) {
    std::vector<BestTime> out;
    if(FILE* f=std::fopen(path.c_str(),"rb")) {
        int l,d;unsigned long ms;
        while(std::fscanf(f,"%d %d %lu",&l,&d,&ms)==3) if(l>=0 && l<32 && d>=0 && d<4 && ms) out.push_back({l,d,(uint32_t)ms});
        std::fclose(f);
    }
    return out;
}
void best_times_write(const std::string& path,const std::vector<BestTime>& all) {
    std::string text;
    for(const BestTime& b:all) {char line[48];std::snprintf(line,sizeof line,"%d %d %u\n",b.level,b.difficulty,b.ms);text+=line;}
    const std::string temp=path+".tmp";
    if(FILE* f=std::fopen(temp.c_str(),"wb")) {
        const bool ok=std::fwrite(text.data(),1,text.size(),f)==text.size();
        if(std::fclose(f)==0 && ok && std::rename(temp.c_str(),path.c_str())==0) return;
    }
    std::remove(temp.c_str());
    std::fprintf(stderr,"[TTK menu] could not write %s\n",path.c_str());
}
void stats_best_update(bool finished) {
    stats_best_ms=0;stats_new_best=false;
    const int n=level(), d=half(diff_cursor);
    const std::string path=best_times_path();
    if(path.empty() || n<0 || n>=32 || d<0 || d>3 || challenge()) return;
    std::vector<BestTime> all=best_times_read(path);
    auto it=std::find_if(all.begin(),all.end(),[&](const BestTime& b) {return b.level==n && b.difficulty==d;});
    if(it!=all.end()) stats_best_ms=it->ms;
    const uint32_t ms=psx_mod_read_word(player+0x398);
    if(!finished || !ms || (stats_best_ms && ms>=stats_best_ms)) return;
    if(it!=all.end()) it->ms=ms; else all.push_back({n,d,ms});
    best_times_write(path,all);
    stats_best_ms=ms;stats_new_best=true;
}
void stats_flip(CPUState* cpu) {
    if(cpu->gpr[31]!=stats_flip_ra) return;
    ensure_init();
    if(!stats_ours()) return;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    savestate_check();
    if(!stats_open) {
        // The original's own wait on entry; none after a savestate load.
        stats_wait=stats_after_load ? 0 : stats_wait_fields;
        // The picture of play for SAVE GAME: the level's last image (taken
        // as gameplay ended), or the loaded slot's.
        if(!stats_after_load) {
            pause_thumb_ok=end_thumb_ok;
            if(end_thumb_ok) std::memcpy(pause_thumb,end_thumb,sizeof pause_thumb);
        } else if(!thumb_from_load) pause_thumb_ok=false;
        end_thumb_ok=false;
        stats_best_update(!stats_after_load);
        thumb_from_load=false;resume_load=false;
        stats_after_load=false;stats_open=true;stats_continue=false;
        owned=true;page=Page::Stats;screen=-1;cursor=0;top=0;direct_open=false;++entries;++stats_entries;
        input_release_for_menu();
    } else stats_wait-=(int)psx_mod_read_word(tick_fields);
    busy=true;
    {
        Guest g(cpu);
        if(pending_sound>=0) {g.call(fn_sound,(uint32_t)pending_sound,0,0,0);pending_sound=-1;}
        const Geometry geo=read_geometry();
        switch(page) {
        case Page::Save: draw_load(g,geo,true);break;
        case Page::SaveAsk: draw_save_ask(g,geo);break;
        case Page::DeleteAsk: draw_delete_ask(g,geo);break;
        default: thumb_hide();draw_stats(g,geo);break;
        }
        if(stats_continue) {
            // The original's Cross: its sound, then -1, and the run handler
            // leaves the mode after this flip (this image still shows the page).
            g.call(fn_sound,1,0,0,0);
            psx_mod_write_half(stats_result,0xffff);
            stats_continue=false;stats_open=false;owned=false;page=Page::Options;thumb_hide();++stats_continues;
        }
    }
    busy=false;
    ++draws;
    if(stats_open) psx_mod_write_half(stats_timer,stats_wait_fields);
}

// ---- D19N: the original pad layout and the profile's AUTO AIM ----
// Modernized keys send fixed pad buttons (Space Square, E Cross, ...), so play
// needs the original CONTROLLER's default layout; AUTO AIM is the profile's
// aim assist. Both are kept in the game every frame. The memory card keeps
// what Vanilla (or the card) had: the card's pack gets those values back for
// the save (the next frame restores ours), and a card load updates them.
namespace {
struct CardCopy {bool known=false;uint8_t layouts[2*layout_size];bool auto_aim=true;};
CardCopy card;
uint64_t enforced=0, card_packs=0, card_loads=0;
void card_capture() {
    for(uint32_t i=0;i<2*layout_size;++i) card.layouts[i]=psx_mod_read_byte(pad_layouts+i);
    card.auto_aim=psx_mod_read_half(settings_word)&auto_aim_bit;
    card.known=true;
}
bool want_auto_aim() {const char* v=ttk_live_env("DNTTK_AIM_ASSIST");return v && !std::strcmp(v,"original-lock");}
bool modern_settings_ready() {
    return enabled() && !psx_mod_replay_active() && !g_precise_mode && !g_ls_mode && !g_psx_call_bail && authentic();
}
void enforce_modern_settings() {
    if(!card.known) card_capture();
    bool wrote=false;
    for(uint32_t i=0;i<2*layout_size;++i) {
        const uint8_t want=psx_mod_read_byte(default_layout+i%layout_size);
        if(psx_mod_read_byte(pad_layouts+i)!=want) {psx_mod_write_byte(pad_layouts+i,want);wrote=true;}
    }
    const uint16_t w=psx_mod_read_half(settings_word);
    const uint16_t aim=want_auto_aim() ? (uint16_t)(w|auto_aim_bit) : (uint16_t)(w&~auto_aim_bit);
    if(aim!=w) {psx_mod_write_half(settings_word,aim);wrote=true;}
    if(wrote) ++enforced;
}
} // namespace
void card_pack_entry() {
    if(!modern_settings_ready() || !card.known) return;
    for(uint32_t i=0;i<2*layout_size;++i) psx_mod_write_byte(pad_layouts+i,card.layouts[i]);
    const uint16_t w=psx_mod_read_half(settings_word);
    psx_mod_write_half(settings_word,(uint16_t)(card.auto_aim ? w|auto_aim_bit : w&~auto_aim_bit));
    ++card_packs;
}
void card_unpack_done(CPUState* cpu) {
    if(cpu->gpr[31]!=card_unpacked || !modern_settings_ready()) return;
    card_capture();enforce_modern_settings();++card_loads;
}

void game_menu_frame(CPUState* cpu) {
    if(!enabled() || psx_mod_replay_active() || g_precise_mode || g_ls_mode || g_psx_call_bail) return;
    {std::lock_guard<std::recursive_mutex> lock(mutex);savestate_check();quick_poll();}
    if(authentic()) enforce_modern_settings();
    if(resumed) {
        // After our resume (or continue, restart, quit) the screen value goes
        // back to the original's 0, in whatever mode the game is in now.
        std::lock_guard<std::recursive_mutex> lock(mutex);
        const uint16_t v=psx_mod_read_half(pause_screen);
        if(!psx_mod_read_half(paused) && (v==ours_sub || v==death_sub)) psx_mod_write_half(pause_screen,0);
        resumed=false;
    }
    const MenuContext* c=current_context();
    if(!c) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if(stats_open) return; // D20B: the statistics screen keeps its page
        owned=false;screen=-1;thumb_hide();
        return;
    }
    if(c->mode==1) return; // D19I: drawn at the pause dispatch (game_menu_tick)
    ensure_init();
    if(!authentic()) return;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    draw_page(cpu,c);
}
} // namespace ttk

extern "C" const char* ttk_game_menu_json() {
    using namespace ttk;
    static char buffer[1280];
    std::snprintf(buffer,sizeof buffer,"{\"settings\":{\"word\":%u,\"game_speed\":%u,\"enforced\":%llu,\"card_packs\":%llu,\"card_loads\":%llu,\"card_auto_aim\":%s},\"owned\":%s,\"page\":%d,\"screen\":%d,\"cursor\":%d,\"top\":%d,\"items\":%d,\"entries\":%llu,\"draws\":%llu,\"writes\":%llu,\"finals\":%llu,\"handoff\":%d,\"pending_state\":%d,\"loading\":%s,\"card\":[%llu,%llu,%llu],\"identity\":%s,\"drops\":%llu,\"drop\":[%d,%u,%u],\"skips\":[%llu,%llu,%llu,%llu],\"skip_ra\":\"%x\",\"quick\":[%d,%d,%d],\"stats\":[%d,%d,%llu,%llu]}",
        psx_mod_read_half(settings_word),psx_mod_read_half(game_speed),(unsigned long long)enforced,(unsigned long long)card_packs,(unsigned long long)card_loads,card.auto_aim?"true":"false",
        owned?"true":"false",(int)page,screen,cursor,top,(int)list.size(),(unsigned long long)entries,(unsigned long long)draws,(unsigned long long)writes,
        (unsigned long long)finals,(int)handoff,pending_state,loading?"true":"false",(unsigned long long)card_skips,(unsigned long long)card_hidden,(unsigned long long)psx_mod_unpaced_vblanks(),identity_checked?(identity_ok?"true":"false"):"null",(unsigned long long)drops,drop_why,drop_state,drop_sub,(unsigned long long)skip_owned,(unsigned long long)skip_screen,(unsigned long long)skip_image,(unsigned long long)drawn_calls,skip_ra,quick_slot,direct_open?1:0,(int)direct,stats_open?1:0,stats_wait,(unsigned long long)stats_entries,(unsigned long long)stats_continues);
    return buffer;
}

// D19K: the LOAD GAME host image for the renderer (gl_swap_with_osd,
// host_osd_draw_sdl): its pixels (transparent around the thumbnail and text)
// and its rectangle (x, y, w, h) as fractions of the 4:3 menu area of the game
// picture. Valid until the next call.
extern "C" int ttk_menu_thumb_pixels(const uint32_t* pixels) {
    using namespace ttk;
    return pixels && pixels==overlay_out.data();
}
extern "C" int ttk_menu_thumb_image(const uint32_t** pixels,int* width,int* height,float* rect) {
    using namespace ttk;
    std::lock_guard<std::mutex> lock(thumb_mutex);
    if(!overlay_on || SDL_GetTicks()>overlay_until) return 0;
    overlay_out=overlay_px;
    *pixels=overlay_out.data();*width=overlay_w;*height=overlay_h;
    rect[0]=float(overlay_x)/canvas_w;rect[1]=float(overlay_y)/canvas_h;
    rect[2]=float(overlay_w)/canvas_w;rect[3]=float(overlay_h)/canvas_h;
    return 1;
}

// D19Q: the framework wrote a savestate slot (ttk_savestate_saved). A save
// taken while our pause menu is up gets the picture of play from when the
// pause opened. True when it is SAVE GAME's own save (it then resumes play
// when LOAD GAME restores it).
namespace ttk {
bool game_menu_saved(int slot) {
    if(!enabled()) return false;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if(pause_thumb_ok && owned && psx_mod_read_half(game_mode)==1 && psx_mod_read_half(paused) && psx_mod_read_half(pause_screen)==ours_sub)
        savestate_write_thumb(slot,pause_thumb);
    // D20B: a save on the statistics screen gets the picture of the level's end.
    else if(pause_thumb_ok && stats_open && psx_mod_read_half(game_mode)==2) savestate_write_thumb(slot,pause_thumb);
    const bool ours=saving && slot==save_slot;
    if(ours) save_written=true;
    quick_slot=slot; // D20: any save
    if(quick_saving && slot==quick_save_slot) quick_saved=true;
    return ours;
}
}

// D19K: CHECKING MEMORY CARD and CHECKING SAVE GAME FILE, drawn by states 1
// and 2 with the text call 0x8008b454: the call draws the string's empty end
// instead. The title above them stays, as on PRESS START that follows.
namespace ttk {
void card_text_entry(CPUState* cpu) {
    if(!enabled() || psx_mod_read_half(game_mode)!=0) return;
    const uint32_t ra=cpu->gpr[31];
    if(ra<card_states_begin || ra>=card_states_end) return;
    const uint16_t state=psx_mod_read_half(fe_state);
    if(state!=state_card && state!=state_save_file) return;
    ensure_init();
    if(!authentic()) return;
    const uint32_t s=cpu->gpr[4];
    if(s!=psx_mod_read_word(strings+4u*text_checking_card) && s!=psx_mod_read_word(strings+4u*text_checking_save)) return;
    uint32_t end=s;
    while(end<s+64 && psx_mod_read_byte(end)) ++end;
    if(psx_mod_read_byte(end)) return;
    cpu->gpr[4]=end;++card_hidden;
}
}

PSX_MOD_CONSTRUCTOR(register_ttk_game_menu) {
    ttk::input_set_menu_handler(ttk::game_menu_input);
    ttk::input_set_quick_handler(ttk::game_menu_quick);
    // Each context's tick, just before its screen dispatch: the frontend's
    // background 0x800220f4 and the pause menu dispatch 0x80025fb0.
    for(uint32_t a:{0x800220f4u,0x80025fb0u})
        psx_mod_register_function_entry_plugin("ttk.game.menu",a,[](CPUState* cpu,uint32_t){ttk::game_menu_tick(cpu);});
    // D19J: the frontend tick itself, before its pad read and state dispatch.
    psx_mod_register_function_entry_plugin("ttk.game.menu",0x80022324u,[](CPUState* cpu,uint32_t){ttk::title_entry(cpu);});
    // D19K: the startup memory card texts. D20B: the statistics screen's.
    psx_mod_register_function_entry_plugin("ttk.game.menu",ttk::fn_text,[](CPUState* cpu,uint32_t){ttk::card_text_entry(cpu);ttk::stats_text_entry(cpu);});
    // D20B: gameplay's exit (the level's last image) and the statistics screen's flip.
    psx_mod_register_function_entry_plugin("ttk.game.menu",ttk::gameplay_exit,[](CPUState*,uint32_t){ttk::gameplay_exit_entry();});
    psx_mod_register_function_entry_plugin("ttk.game.menu",ttk::flip,[](CPUState* cpu,uint32_t){ttk::stats_flip(cpu);});
    // D19N: the memory card's settings pack and the end of its unpack.
    psx_mod_register_function_entry_plugin("ttk.game.menu",ttk::fn_card_pack,[](CPUState*,uint32_t){ttk::card_pack_entry();});
    psx_mod_register_function_entry_plugin("ttk.game.menu",ttk::fn_game_speed,[](CPUState* cpu,uint32_t){ttk::card_unpack_done(cpu);});
}
