#include "lua/lua.hpp"
#include "pnlua.h"
#include <string>

#if !defined(_MSC_VER)
#define GM_EXPORT __attribute__((visibility("default")))
#else
#define GM_EXPORT __declspec(dllexport)
#endif

#define GM_TRUE 1.0
#define GM_FALSE 0.0

/* ----------------
   GLOBAL VARIABLES
   ---------------- */

static PNLuaState* current_state = NULL;
static void* current_buffer = NULL;

/* -----------
   PNLUA STATE
   ----------- */

PNLuaState::PNLuaState() {}

/* This is a wrapper for creating a PNLuaState, since directly creating a
   PNLuaState in a GM_EXPORT function will cause compile errors. */
PNLuaState* new_pnluastate() {
    lua_State* lua_state = luaL_newstate();

    if (lua_state == NULL) {
        return NULL;
    }

    luaL_openlibs(lua_state);

    PNLuaState* state = new PNLuaState();

    state->state = lua_state;

    return state;
}

/* -------------
   LUA FUNCTIONS
   ------------- */

int gm_call(lua_State* lua_state) {
    // TODO: Buffer writing stuff goes here.

    return lua_yield(lua_state, 1);
}

/* -------------
   GML FUNCTIONS
   ------------- */

extern "C" {
    GM_EXPORT double pnlua_internal_init(void* buffer) {
        current_buffer = buffer;

        return GM_TRUE;
    }

    GM_EXPORT double pnlua_internal_state_create() {
        PNLuaState* state = new_pnluastate();

        if (state == NULL) {
            return -1.0;
        }

        return (double)(size_t)state;
    }

    GM_EXPORT double pnlua_internal_state_destroy(double id) {
        PNLuaState* state = (PNLuaState*)(size_t)id;

        if (state == NULL) {
            return GM_FALSE;
        }

        lua_close(state->state);

        delete state;

        return GM_TRUE;
    }

    GM_EXPORT double pnlua_internal_state_register(double id, char* function_name) {
        PNLuaState* state = (PNLuaState*)(size_t)id;

        if (state == NULL) {
            return GM_FALSE;
        }

        lua_State* lua_state = state->state;

        lua_pushstring(lua_state, function_name);
        lua_pushcclosure(lua_state, gm_call, 1);
        lua_setglobal(lua_state, function_name);

        return GM_TRUE;
    }

    GM_EXPORT double pnlua_internal_state_load(double id, char* file_name) {
        PNLuaState* state = (PNLuaState*)(size_t)id;

        if (state == NULL) {
            return GM_FALSE;
        }

        lua_State* lua_state = state->state;

        int result = luaL_loadfile(lua_state, file_name);

        if (result != LUA_OK) {
            // File load has failed, send the error to PNLua's buffer.
            unsigned char* buffer = (unsigned char*)current_buffer;
            strcpy((char*)buffer, lua_tostring(lua_state, -1));

            return -1.0;
        }

        return GM_TRUE;
    }
}