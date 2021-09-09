/* ---
   LUA
   --- */

#include "lua/lua.hpp"

/* -------------
   GML FUNCTIONS
   ------------- */

#include "pnlua.h"

#define GM_EXPORT extern "C" __declspec(dllexport)

#define GM_TRUE 1.0
#define GM_FALSE 0.0

GM_EXPORT double pnlua_state_create() {
    lua_State* state = luaL_newstate();

    if (state == NULL) {
        return -1.0;
    }

    return (double)(size_t)state;
}

GM_EXPORT double pnlua_state_destroy(double id) {
    lua_State* state = (lua_State*)(size_t)id;

    if (state == NULL) {
        return GM_FALSE;
    }

    lua_close(state);

    return GM_TRUE;
}

/* ------------
   ASYNCHRONOUS
   ------------ */

const int EVENT_OTHER_SOCIAL = 70;

__declspec (dllexport) void RegisterCallbacks(char *arg1, char *arg2, char *arg3, char *arg4)
{
    void (*CreateAsynEventWithDSMapPtr)(int, int) = (void (*)(int, int))(arg1);
    int (*CreateDsMapPtr)(int _num, ...) = (int(*)(int _num, ...)) (arg2);
    CreateAsynEventWithDSMap = CreateAsynEventWithDSMapPtr;
    CreateDsMap = CreateDsMapPtr;

    bool (*DsMapAddDoublePtr)(int _index, char *_pKey, double value) = (bool(*)(int, char*, double))(arg3);
    bool (*DsMapAddStringPtr)(int _index, char *_pKey, char *pVal) = (bool(*)(int, char*, char*))(arg4);

    DsMapAddDouble = DsMapAddDoublePtr;
    DsMapAddString = DsMapAddStringPtr;
}

void send_error(lua_State* state) {
    /* The error message is on top of the stack.
       Fetch it, send it asynchronously to GameMaker and then pop it off the
       stack. */

    const char* message = lua_tostring(state, -1);
    int async_load = CreateDsMap( 0 );

    DsMapAddString(async_load, (char*)"event_type", (char*)"PNLuaError");
    DsMapAddString(async_load, (char*)"error_message", (char*)message);
    CreateAsynEventWithDSMap(async_load, EVENT_OTHER_SOCIAL);

    lua_pop(state, 1);
}
