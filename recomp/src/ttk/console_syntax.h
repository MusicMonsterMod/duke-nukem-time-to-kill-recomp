#pragma once
// D26L2: the backtick console's one table of commands and their words, shared
// by the syntax highlighter, `help` and (later) tab completion; see
// documentation/567-d26l2-console-syntax-highlighting.md.
#include <stdint.h>
#include <string>
#include <vector>
#ifdef __cplusplus
namespace ttk {
enum class Syn : uint8_t {
    plain,       // the font's own colour
    prompt,      // the "> " before a typed line
    command,     // a known command word
    keyword,     // a word that command knows (room, face, on, off ...)
    number,      // 12, -300, +45, 0.5, 0x100f, 16:9
    name,        // a run-time name: spawn item, landmark, option key or value
    placeholder, // help text's N, X, [radius], <type>
    unknown,     // a word nothing knows: a typo shows before Enter
    error,       // the text of an error reply
    warning,     // the text of a usage / Try: / note reply
    heading,     // D26L4: a reply's heading line or a tabbed row's label cell
};
// Run-time names the table cannot hold (they come from game data). Each
// returns 0 for no, 1 for a whole name, 2 when `word` only begins one (asked
// with prefix set, for the word still being typed).
int console_spawn_name(const std::string& rest,bool prefix);        // spawn.inc items and crystals
int console_landmark_name(const std::string& word,bool prefix);     // goto.inc landmarks.txt
int console_option_key(const std::string& word,bool prefix);        // game_menu.cpp settings keys
int console_option_value(const std::string& key,const std::string& word,bool prefix);
// D26L tab completion: the same names as lists (spawn: what `items` lists here,
// or every name outside play).
void console_spawn_names(std::vector<std::string>& out);
void console_landmark_names(std::vector<std::string>& out);
void console_option_keys(std::vector<std::string>& out);
void console_option_values(const std::string& key,std::vector<std::string>& out);
// Classifies `text` byte by byte into out[0..n). typed: a command line (the
// prompt, or one recalled); caret: the byte the caret is at (the word ending
// there may be half typed, so it is not called a typo), or -1. Otherwise a
// console reply (echoed commands, help, errors).
void console_classify(const char* text,bool typed,int caret,Syn* out,int n);
uint32_t console_colour(Syn s);
// Help from the table: the everyday commands, or with all every command.
void console_help(void (*say)(const char*),bool all);
// D26L4: says the usage of line's first word when the table knows it
// (returns false for an unknown command).
bool console_usage(const char* line,void (*say)(const char*));
// D26L4: "Unknown command: X" with the nearest command, from the table.
void console_unknown(const char* line,void (*say)(const char*));
// D26L: tab completion of the word ending at `caret` in line[0..len). Sets
// *from to where that word starts (for spawn, the rest of the line: item names
// have spaces) and fills out with the sorted candidates, lower case.
void console_complete(const char* line,int len,int caret,int* from,std::vector<std::string>& out);
// D26L: what Tab does to the line. Returns the new line and caret in line and
// *caret; *list is set (and the line unchanged) when the candidates should be
// listed: the word is already as long as they agree on.
void console_tab(std::string& line,int* caret,bool* list,std::vector<std::string>& candidates);
// D26L: says the candidates as tabbed rows (the console aligns them).
void console_tab_list(const std::vector<std::string>& candidates,void (*say)(const char*));
// The table's first words, for the anti-drift test and tab completion.
int console_command_count();
const char* console_command_name(int i);
}
extern "C" {
#endif
// The framework console's hook (host_osd.c): tints for each byte of a row,
// 0 to keep the font's colour. kind 0: the command line (caret: the caret's
// byte or -1); kind 1: a scrollback row. Returns 0 to leave the row untinted.
int ttk_console_tints(const char* text,int kind,int caret,uint32_t* tints,int n);
#ifdef __cplusplus
}
#endif
