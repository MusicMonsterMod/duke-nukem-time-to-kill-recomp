#pragma once
#ifdef __cplusplus
extern "C" {
#endif
// D19: a DNTTK_* setting as last confirmed in the OPTIONS screens, else getenv().
const char* ttk_live_env(const char* name);
void ttk_live_env_set(const char* name, const char* value);
#ifdef __cplusplus
}
#endif
