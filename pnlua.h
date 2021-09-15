#if !defined( _MSC_VER)
#define GM_EXPORT __attribute__((visibility("default")))
#else
#define GM_EXPORT __declspec(dllexport)
#endif

/* -----------
   PNLUA STATE
   ----------- */

class PNLuaState {
    public:
        PNLuaState();

        lua_State* state;
        lua_State* thread;
};

PNLuaState* new_pnluastate();

/* -------------
   GML FUNCTIONS
   ------------- */

extern "C" {
    GM_EXPORT double pnlua_internal_init(void* buffer);
    GM_EXPORT double pnlua_internal_state_create();
    GM_EXPORT double pnlua_internal_state_destroy(double id);
    GM_EXPORT double pnlua_internal_state_register(double id, char* function_name);
    GM_EXPORT double pnlua_internal_state_load(double id, char* file_name);
    GM_EXPORT double pnlua_internal_state_call(double id);
}