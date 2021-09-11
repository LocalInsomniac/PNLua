/* ---
   LUA
   --- */

/* -------------
   GML FUNCTIONS
   ------------- */

#define GM_EXPORT extern "C" __declspec(dllexport)

GM_EXPORT double pnlua_state_create();
GM_EXPORT double pnlua_state_destroy(double id);
GM_EXPORT double pnlua_state_load(double id, char* filename);

/* ------------
   ASYNCHRONOUS
   ------------ */

GM_EXPORT void RegisterCallbacks(char* arg1, char* arg2, char* arg3, char* arg4);

void send_error(lua_State* state);
