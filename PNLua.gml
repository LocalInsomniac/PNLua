global.___pnlua_states = ds_map_create()

function pnlua_state_create() {
	var state = pnlua_state_create_internal()
	
	if state != -1 {
		ds_map_add_map(global.___pnlua_states, state, ds_map_create())
	}
	
	return state
}

function pnlua_state_destroy(id) {
	if pnlua_state_destroy_internal(id) {
		ds_map_delete(global.___pnlua_states, id)
	}
}

function pnlua_state_register(id, function_name, callback) {
	var state = global.___pnlua_state[? id]
	
	if is_undefined(state) or ds_map_exists(state, function_name) {
		exit
	}
	
	ds_map_add(state, function_name, callback)
	pnlua_state_register_internal(state, function_name)
}

function pnlua_process_call(async_map) {
	var state = global.___pnlua_states[? async_map[? "state"]]
	
	if is_undefined(state) {
		exit
	}
	
	var callback = state[? async_map[? "function"]]
	
	if is_undefined(callback) {
		exit
	}
	
	var args = ds_map_size(async_map) - 3
	var arg = []
	var i = 0
	
	repeat args {
		arg[@ i] = async_map[? string(i)]
		i++
	}
	
	callback()
}