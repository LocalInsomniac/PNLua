global.___pnlua_states = ds_map_create()
global.___pnlua_argument = []

#macro ARG global.___pnlua_argument

/// @desc Create a Lua state. Return its pointer.
function pnlua_state_create() {
	var state = pnlua_state_create_internal()

	if state != -1 {
		ds_map_add_map(global.___pnlua_states, state, ds_map_create())
	}

	return state
}

/// @desc Destroy a Lua state.
/// @param {PNLuaState} id The pointer of the Lua state to destroy.
function pnlua_state_destroy(id) {
	if pnlua_state_destroy_internal(id) {
		ds_map_delete(global.___pnlua_states, id)
	}
}

/* pnlua_state_load(id, filename)
   
   Load a Lua script and execute anything outside of the script's functions.
   
   Arguments:
   {PNLuaState} id The pointer of the Lua state to load the script into.
   {string} filename The file name of the Lua script to load. */

/// @desc Register a GML function to a Lua state.
/// @param {PNLuaState} id The pointer of the Lua state to register the function in.
/// @param {string} function_name The name of the function in Lua.
/// @param {function} callback The function to call.
function pnlua_state_register(id, function_name, callback) {
	var state = global.___pnlua_states[? id]

	if is_undefined(state) or ds_map_exists(state, function_name) {
		exit
	}

	ds_map_add(state, function_name, callback)
	pnlua_state_register_internal(id, function_name)
}

/* pnlua_state_call(id, function_name, object) 
   
   Call a Lua function in the specified Lua state.
   
   Arguments:
   {PNLuaState} id The pointer of the Lua state to call the function from.
   {string} function_name The name of the function to call from Lua.
   {object} object The ID of the calling object instance.*/

/// @desc Process a Lua script's GML function call.
/// @param {ds_map} async_map The ds_map of the asynchronous event.
function pnlua_process_call(async_map) {
	var state = global.___pnlua_states[? async_map[? "state"]]

	if is_undefined(state) {
		show_message("idk state")
		exit
	}
	
	var callback = state[? async_map[? "function"]]

	if is_undefined(callback) {
		exit
	}

	// Get argument values for the function's callback
	var args = ds_map_size(async_map) - 3
	
	ARG = []
	
	var i = 0

	repeat args {
		ARG[@ i] = async_map[? string(i)]
		i++
	}

	// Activate the function correspondent to the Lua one
	callback()
	
	// TODO: Yield the Lua state until the GML callback returns a value.
	// pnlua_state_resume_internal(state)
}