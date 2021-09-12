#include "lua/lua.hpp"
#include "pnlua.h"
#include <string>

#define GM_EXPORT extern "C" __declspec(dllexport)
#define GM_TRUE 1.0
#define GM_FALSE 0.0

/* ------------
   ASYNCHRONOUS
   ------------ */

const int EVENT_OTHER_SOCIAL = 70;

void (*CreateAsynEventWithDSMap)(int, int) = NULL;
int (*CreateDsMap)(int _num, ...) = NULL;
bool (*DsMapAddDouble)(int _index, char* _pKey, double value) = NULL;
bool (*DsMapAddString)(int _index, char* _pKey, char* pVal) = NULL;

GM_EXPORT void RegisterCallbacks(char* arg1, char* arg2, char* arg3, char* arg4) {
    void (*CreateAsynEventWithDSMapPtr)(int, int) = (void(*)(int, int))(arg1);
    int (*CreateDsMapPtr)(int _num, ...) = (int(*)(int _num, ...))(arg2);

    CreateAsynEventWithDSMap = CreateAsynEventWithDSMapPtr;
    CreateDsMap = CreateDsMapPtr;

    bool (*DsMapAddDoublePtr)(int _index, char* _pKey, double value) = (bool(*)(int, char*, double))(arg3);
    bool (*DsMapAddStringPtr)(int _index, char* _pKey, char* pVal) = (bool(*)(int, char*, char*))(arg4);

    DsMapAddDouble = DsMapAddDoublePtr;
    DsMapAddString = DsMapAddStringPtr;
}

void send_error(lua_State* state) {
    /* The error message is on top of the stack.
       Fetch it, send it asynchronously to GameMaker and then pop it off the
       stack. */

    char* message = (char*)lua_tostring(state, -1);
    int ds_map = CreateDsMap(0);

    DsMapAddString(ds_map, (char*)"event_type", (char*)"PNLuaError");
    DsMapAddString(ds_map, (char*)"error_message", (char*)message);
    CreateAsynEventWithDSMap(ds_map, EVENT_OTHER_SOCIAL);

    lua_pop(state, 1);
}

int gm_call(lua_State* state) {
    lua_pushvalue(state, lua_upvalueindex(1));

    int args = lua_gettop(state) - 1;
    char* callback = (char*)lua_tostring(state, args + 1);

    int ds_map = CreateDsMap(0);

    DsMapAddString(ds_map, (char*)"event_type", (char*)"PNLuaCall");
    DsMapAddDouble(ds_map, (char*)"state", (double)(size_t)state);
    DsMapAddString(ds_map, (char*)"function", callback);

    for (int i = 0; i < args; i++) {
        std::string arg_str = std::to_string(i);
        char* arg = (char*)arg_str.c_str();

        if (lua_type(state, i + 1) == LUA_TSTRING) {
            DsMapAddString(ds_map, arg, (char*)lua_tostring(state, i + 1));
        } else {
            if (lua_type(state, i + 1) == LUA_TNUMBER) {
                DsMapAddDouble(ds_map, arg, lua_tonumber(state, i + 1));
            }
        }
    }

    CreateAsynEventWithDSMap(ds_map, EVENT_OTHER_SOCIAL);

    return lua_yield(state, 0);
}

/* -------------
   GML FUNCTIONS
   ------------- */

GM_EXPORT double pnlua_state_create_internal() {
    lua_State* state = luaL_newstate();

    if (state == NULL) {
        return -1.0;
    }

    luaL_openlibs(state);

    return (double)(size_t)state;
}

GM_EXPORT double pnlua_state_destroy_internal(double id) {
    lua_State* state = (lua_State*)(size_t)id;

    if (state == NULL) {
        return GM_FALSE;
    }

    lua_close(state);

    return GM_TRUE;
}

GM_EXPORT double pnlua_state_load(double id, char* filename) {
    lua_State* state = (lua_State*)(size_t)id;

    if (state == NULL) {
        return GM_FALSE;
    }

    if (luaL_loadfile(state, filename) != LUA_OK || lua_pcall(state, 0, LUA_MULTRET, 0) != LUA_OK) {
        send_error(state);

        return GM_FALSE;
    }

    return GM_TRUE;
}

GM_EXPORT double pnlua_state_register_internal(double id, char* function_name) {
    lua_State* state = (lua_State*)(size_t)id;

    if (state == NULL) {
        return GM_FALSE;
    }

    lua_pushstring(state, function_name);
    lua_pushcclosure(state, gm_call, 1);

    std::string function_string = function_name;

    lua_setglobal(state, function_string.c_str());

    return GM_TRUE;
}

GM_EXPORT double pnlua_state_call(double id, char* function_name) {
    lua_State* state = (lua_State*)(size_t)id;

    if (state == NULL) {
        return GM_FALSE;
    }

	lua_getglobal(state, function_name);

	if (lua_pcall(state, 0, 1, 0) != LUA_OK) {
        send_error(state);

        return GM_FALSE;
	}
    
    lua_pop(state, 1);

    return GM_TRUE;
}

GM_EXPORT double pnlua_state_resume_internal(double id) {
    lua_State* state = (lua_State*)(size_t)id;

    if (state == NULL) {
        return GM_FALSE;
    }

    int result = lua_resume(state, NULL, 0, 0);

    if (result == LUA_OK) {
        return GM_TRUE;
    } else {
        if (result > LUA_YIELD) {
            send_error(state);

            return GM_FALSE;
        }
    }

    return GM_FALSE;
}