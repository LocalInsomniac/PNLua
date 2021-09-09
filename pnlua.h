/* ---
   LUA
   --- */

/* -------------
   GML FUNCTIONS
   ------------- */

#define GM_EXPORT extern "C" __declspec(dllexport)

GM_EXPORT double pnlua_state_create();
GM_EXPORT double pnlua_state_destroy(double id);

/* ------------
   ASYNCHRONOUS
   ------------ */

void (*CreateAsynEventWithDSMap)(int, int) = NULL;
int (*CreateDsMap)(int _num, ... ) = NULL;
bool (*DsMapAddDouble)(int _index, char *_pKey, double value) = NULL;
bool (*DsMapAddString)(int _index, char *_pKey, char *pVal) = NULL;

void RegisterCallbacks(char *arg1, char *arg2, char *arg3, char *arg4);

void send_error(lua_State* state);
