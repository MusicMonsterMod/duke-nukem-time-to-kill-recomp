// D19: ttk_live_env() is getenv() until the OPTIONS screens confirm a value.
#include "live_env.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

int main() {
    unsetenv("DNTTK_LIVE_TEST_A");
    setenv("DNTTK_LIVE_TEST_B","original",1);
    assert(ttk_live_env("DNTTK_LIVE_TEST_A")==nullptr);              // unset stays unset
    assert(!std::strcmp(ttk_live_env("DNTTK_LIVE_TEST_B"),"original"));
    ttk_live_env_set("DNTTK_LIVE_TEST_B","modern");
    assert(!std::strcmp(ttk_live_env("DNTTK_LIVE_TEST_B"),"modern"));  // the menu wins
    assert(!std::strcmp(std::getenv("DNTTK_LIVE_TEST_B"),"original")); // the environment is untouched
    ttk_live_env_set("DNTTK_LIVE_TEST_A","1");
    ttk_live_env_set("DNTTK_LIVE_TEST_B","original");
    assert(!std::strcmp(ttk_live_env("DNTTK_LIVE_TEST_A"),"1"));
    assert(!std::strcmp(ttk_live_env("DNTTK_LIVE_TEST_B"),"original"));
    assert(ttk_live_env("DNTTK_LIVE_TEST_C")==nullptr);
    // D40: the world renderer starts from the launch choice and follows a switch;
    // a developer switch is explicit only when set.
    setenv("DNTTK_WORLD_RENDERER","classic",1);
    assert(ttk_world_classic()==1);
    ttk_world_classic_set(0);
    assert(ttk_world_classic()==0);
    ttk_world_classic_set(1);
    assert(ttk_world_classic()==1);
    unsetenv("DNTTK_LIVE_TEST_S");
    assert(ttk_env_switch("DNTTK_LIVE_TEST_S")==-1);
    setenv("DNTTK_LIVE_TEST_S","",1);assert(ttk_env_switch("DNTTK_LIVE_TEST_S")==-1);
    setenv("DNTTK_LIVE_TEST_S","0",1);assert(ttk_env_switch("DNTTK_LIVE_TEST_S")==0);
    setenv("DNTTK_LIVE_TEST_S","1",1);assert(ttk_env_switch("DNTTK_LIVE_TEST_S")==1);
    std::puts("PASS: live settings start from the environment and follow the menu");
    return 0;
}
