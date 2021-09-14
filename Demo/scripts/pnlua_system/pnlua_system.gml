global.___pnlua_states = ds_map_create()
global.___pnlua_buffer = buffer_create(PNLUA_MAX_BUFFER_SIZE, buffer_fixed, 1);
pnlua_internal_init(buffer_get_address(global.___pnlua_buffer))

/// @desc Create a Lua state. Return its pointer.
function pnlua_state_create() {
	var state = pnlua_internal_state_create()

	if state != -1 {
		ds_map_add_map(global.___pnlua_states, state, ds_map_create())
	}

	return state
}

/// @desc Destroy a Lua state.
/// @param {PNLuaState} id The pointer of the Lua state to destroy.
function pnlua_state_destroy(id) {
	if pnlua_internal_state_destroy(id) {
		ds_map_delete(global.___pnlua_states, id)
	}
}

/// @desc Load a Lua script and execute anything outside of the script's functions.
/// @param {PNLuaState} id The pointer of the Lua state to load the script into.
/// @param {string} filename The file name of the Lua script to load.
function pnlua_state_load(id, file_name) {
	if pnlua_internal_state_load(id, file_name) == -1 {
		// There was an error loading the script, read the message from the buffer.
		buffer_seek(global.___pnlua_buffer, buffer_seek_start, 0)
		show_error("!!! PNLua: Error in state " + string(id) + " while loading '" + file_name + "': " + buffer_read(global.___pnlua_buffer, buffer_string), true)
	}
}

/// @desc Register a GML function to a Lua state.
/// @param {PNLuaState} id The pointer of the Lua state to register the function in.
/// @param {string} function_name The name of the function in Lua.
/// @param {function} callback The function to call.
/*function pnlua_state_register(id, function_name, callback) {
	var state = global.___pnlua_states[? id]

	if is_undefined(state) or ds_map_exists(state, function_name) {
		exit
	}

	ds_map_add(state, function_name, callback)
	pnlua_internal_state_register(id, function_name)
}