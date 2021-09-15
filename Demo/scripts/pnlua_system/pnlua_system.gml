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

/// @desc Load a Lua script into the state.
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
}*/

/// @desc Execute a script/function loaded within a Lua state.
/// @param {PNLuaState} id The pointer of the Lua state to execute.
/// @param {string} [function_name] The name of the function to call from the script.
/// @param {real/string} [arguments...] The arguments to pass to the Lua function.
function pnlua_state_call() {
	var _id = argument[0]
	var function_name = undefined
	var function_args = undefined
	
	if argument_count > 1 {
		function_name = argument[1]
		
		if is_undefined(function_name) or not is_string(function_name) {
			show_error("!!! PNLua: Error in state " + string(_id) + " while calling: Invalid function name", true)
		}
		
		if argument_count > 2 {
			function_args = []
			
			var i = 2
			
			repeat argument_count - 2 {
				array_push(function_args, argument[i])
				i++
			}
		}
	}
	
	// Pass stuff to the state using a buffer
	buffer_seek(global.___pnlua_buffer, buffer_seek_start, 0)
	
	/* Header types:
	   0 = no function specified
	   1 = function without arguments
	   2 = function with arguments */

	buffer_write(global.___pnlua_buffer, buffer_u8, is_undefined(function_name) and is_undefined(function_args) ? 0 : (is_undefined(function_args) ? 1 : 2))

	if not is_undefined(function_name) {
		buffer_write(global.___pnlua_buffer, buffer_string, function_name)
		
		if not is_undefined(function_args) {
			/* Argument types:
			   0 = nil
			   1 = number
			   2 = string
			   
			   Any other unsupported data type will result in an error. */
			
			var args_length = array_length(function_args)
			var i = 0
			
			buffer_write(global.___pnlua_buffer, buffer_u32, args_length)

			repeat args_length {
				var arg = function_args[i]
				
				if is_undefined(arg) {
					repeat 2 {
						buffer_write(global.___pnlua_buffer, buffer_u8, 0)
					}
				} else {
					if is_real(arg) {
						buffer_write(global.___pnlua_buffer, buffer_u8, 1)
						buffer_write(global.___pnlua_buffer, buffer_f32, arg)
					} else {
						if is_string(arg) {
							buffer_write(global.___pnlua_buffer, buffer_u8, 2)
							buffer_write(global.___pnlua_buffer, buffer_string, arg)
						} else {
							show_error("!!! PNLua: Error in state " + string(_id) + " while calling '" + function_name + "': Unsupported data type in argument " + string(i) + " (" + string(arg) + ")", true)
						}
					}
				}
				
				i++
			}
		}
	}
	
	switch pnlua_internal_state_call(_id) {
		case -1:
			// There was an error calling the script, read the message from the buffer.
			buffer_seek(global.___pnlua_buffer, buffer_seek_start, 0)
			show_error("!!! PNLua: Error in state " + string(_id) + " while calling: " + buffer_read(global.___pnlua_buffer, buffer_string), true)
		break
		
		case 2:
			// The state is waiting for a return value, send it over.
		break
	}
}