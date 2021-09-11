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

    luaL_openlibs(state);

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

GM_EXPORT double pnlua_state_load(double id, char* filename) {
    lua_State* state = (lua_State*)(size_t)id;

    if (state == NULL) {
        return GM_FALSE;
    }

    if (luaL_loadfile(state, filename) != LUA_OK) {
        send_error(state);

        return GM_FALSE;
    }

    return GM_TRUE;
}

/* ------------
   ASYNCHRONOUS
   ------------ */

const int EVENT_OTHER_SOCIAL = 70;

void (*CreateAsynEventWithDSMap)(int, int) = NULL;
int (*CreateDsMap)(int _num, ...) = NULL;
bool (*DsMapAddDouble)(int _index, char* _pKey, double value) = NULL;
bool (*DsMapAddString)(int _index, char* _pKey, char* pVal) = NULL;

GM_EXPORT void RegisterCallbacks(char *arg1, char *arg2, char *arg3, char *arg4) {
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
