#define GM_EXPORT extern "C" __declspec(dllexport)

/* ------------
   ASYNCHRONOUS
   ------------ */

GM_EXPORT void RegisterCallbacks(char* arg1, char* arg2, char* arg3, char* arg4);

void send_error(lua_State* state);

int gm_call(lua_State* state);

/* -------------
   GML FUNCTIONS
   ------------- */

GM_EXPORT double pnlua_state_create_internal();
GM_EXPORT double pnlua_state_destroy_internal(double id);
GM_EXPORT double pnlua_state_load(double id, char* filename);
GM_EXPORT double pnlua_state_register_internal(double id, char* function_name);
GM_EXPORT double pnlua_state_call(double id, char* function_name, double object);