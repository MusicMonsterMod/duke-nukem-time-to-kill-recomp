#pragma once

namespace ttk {
// D26A debug level select for the backtick console (runtime event-pump thread,
// between guest frames). Handles "levels" and "level N"; returns false for any
// other line. close is set when the console should close so play resumes.
bool level_console_command(const char* line, void (*say)(const char*), bool& close);
}
