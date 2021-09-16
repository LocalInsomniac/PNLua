#include "lua/lua.hpp"
#include "pnlua.h"
#include <string>
#include <iostream>
using namespace std;

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
    state->thread = lua_newthread(state->state);

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

        lua_pushstring(state->thread, function_name);
        lua_pushcclosure(state->thread, gm_call, 1);
        lua_setglobal(state->thread, function_name);

        return GM_TRUE;
    }

    GM_EXPORT double pnlua_internal_state_load(double id, char* file_name) {
        PNLuaState* state = (PNLuaState*)(size_t)id;

        if (state == NULL) {
            return GM_FALSE;
        }

        if (luaL_loadfile(state->thread, file_name) != LUA_OK) {
            // File load has failed, send the error to PNLua's buffer.
            unsigned char* buffer = (unsigned char*)current_buffer;
            strcpy((char*)buffer, lua_tostring(state->thread, -1));

            return -1.0;
        }

        return GM_TRUE;
    }

    GM_EXPORT double pnlua_internal_state_call(double id) {
        PNLuaState* state = (PNLuaState*)(size_t)id;

        if (state == NULL) {
            return GM_FALSE;
        }

        int args = 0;
        float* buffer = (float*)current_buffer;
        
        /* Check the header of the buffer to see what type of call we're
           dealing with. */

        int call_type = (int)buffer[0];

        if (!call_type) {
            /* We're calling the entire script, so duplicate it to prevent any
               errors when calling it more than just once. */
            lua_pushvalue(state->thread, 1);
        } else {
            /* We're calling a function either with or without any arguments.
               Get the arguments and the name of the function. */

            size_t function_length = (size_t)buffer[1];
            char* function_name = new char[function_length + 1];

            for (size_t i = 0; i < function_length; i++) {
                function_name[i] = (int)buffer[2 + i];
            }

            function_name[function_length] = '\0';
            lua_getglobal(state->thread, function_name);

            delete[] function_name;

            if (call_type == 2) {
                // If the buffer contains arguments, push them into the stack.
                int i = 2 + function_length;
                args = (int)buffer[i];

                i++;

                for (int j = 0; j < args; j++) {
                    int data_type = (int)buffer[i++];

                    switch (data_type) {
                        case 0: // nil
                            lua_pushnil(state->thread);

                            break;

                        case 1: // number
                            lua_pushnumber(state->thread, buffer[i++]);

                            break;

                        case 2: // string
                            size_t length = (size_t)buffer[i++];
                            char* argument = new char[length + 1];

                            for (size_t k = 0; k < length; k++) {
                                argument[k] = (int)buffer[i++];
                            }

                            argument[length] = '\0';
                            lua_pushstring(state->thread, argument);

                            delete[] argument;

                            break;
                    }
                }
            }
        }

        switch (lua_pcall(state->thread, args, 1, 0)) {
            case LUA_OK: {
                int idx = lua_gettop(state->thread);

                switch (lua_type(state->thread, idx)) {
                case LUA_TNONE:
                case LUA_TNIL:
                    buffer[0] = (float)0;

                    break;

                case LUA_TBOOLEAN:
                case LUA_TNUMBER:
                    buffer[0] = (float)1;
                    buffer[1] = (float)lua_tonumber(state->thread, idx);

                    break;

                case LUA_TSTRING:
                    buffer[0] = (float)2;

                    char* ret = (char*)lua_tostring(state->thread, idx);
                    size_t length = sizeof(ret);

                    buffer[1] = (float)length;

                    for (size_t i = 0; i < length; i++) {
                        buffer[2 + i] = (float)ret[i];
                    }

                    break;
                }

                return GM_TRUE;
            }

            case LUA_YIELD:
                // Lua is waiting for a return value from GameMaker.
                return 2.0;

            default:
                // Lua call has failed, send the error to PNLua's buffer.
                strcpy((char*)buffer, lua_tostring(state->thread, -1));

                return -1.0;
        }

        return GM_FALSE;
    }
}