global.___pnlua_states = ds_map_create()
global.___pnlua_buffer = buffer_create(PNLUA_MAX_BUFFER_SIZE, buffer_fixed, 1);
___pnlua_internal_init(buffer_get_address(global.___pnlua_buffer))

function ___pnlua_buffer_start() {
	buffer_seek(global.___pnlua_buffer, buffer_seek_start, 0)
}

function ___pnlua_buffer_read_string() {
	var strlen = buffer_read(global.___pnlua_buffer, buffer_f32)
	var str = ""
	
	repeat strlen {
		str += chr(buffer_read(global.___pnlua_buffer, buffer_f32))
	}
	
	return str
}

function ___pnlua_buffer_write_string(str) {
	var strlen = string_length(str)
	
	buffer_write(global.___pnlua_buffer, buffer_f32, strlen)
	
	for (var i = 0; i < strlen; i++) {
		buffer_write(global.___pnlua_buffer, buffer_f32, ord(string_char_at(str, i + 1)))
	}
}

/// @desc Create a Lua state. Return its pointer.
function pnlua_state_create() {
	var state = ___pnlua_internal_state_create()

	if state != -1 {
		ds_map_add_map(global.___pnlua_states, state, ds_map_create())
	}

	return state
}

/// @desc Destroy a Lua state.
/// @param {PNLuaState} id The pointer of the Lua state to destroy.
function pnlua_state_destroy(id) {
	if ___pnlua_internal_state_destroy(id) {
		ds_map_delete(global.___pnlua_states, id)
	}
}

/// @desc Load a Lua script into the state.
/// @param {PNLuaState} id The pointer of the Lua state to load the script into.
/// @param {string} filename The file name of the Lua script to load.
function pnlua_state_load(id, file_name) {
	if ___pnlua_internal_state_load(id, file_name) == -1 {
		// There was an error loading the script, read the message from the buffer.
		___pnlua_buffer_start()
		show_error("!!! PNLua: Error in state " + string(id) + " while loading '" + file_name + "': " + buffer_read(global.___pnlua_buffer, buffer_string), true)
	}
}

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
	___pnlua_internal_state_register(id, function_name)
}

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
				function_args[@ array_length(function_args)] = argument[i]
				i++
			}
		}
	}
	
	// Start passing stuff to the state using a buffer
	___pnlua_buffer_start()
	
	/* Header types:
	   0 = no function specified
	   1 = function without arguments
	   2 = function with arguments */

	buffer_write(global.___pnlua_buffer, buffer_f32, is_undefined(function_name) and is_undefined(function_args) ? 0 : (is_undefined(function_args) ? 1 : 2))

	if not is_undefined(function_name) {
		___pnlua_buffer_write_string(function_name)
		
		if not is_undefined(function_args) {
			/* Argument types:
			   0 = nil
			   1 = number
			   2 = string
			   
			   Any other unsupported data type will result in an error. */
			
			var args_length = array_length(function_args)
			var i = 0
			
			buffer_write(global.___pnlua_buffer, buffer_f32, args_length)

			repeat args_length {
				var arg = function_args[i]
				
				if is_undefined(arg) {
					buffer_write(global.___pnlua_buffer, buffer_f32, 0)
				} else {
					if is_real(arg) {
						buffer_write(global.___pnlua_buffer, buffer_f32, 1)
						buffer_write(global.___pnlua_buffer, buffer_f32, arg)
					} else {
						if is_string(arg) {
							buffer_write(global.___pnlua_buffer, buffer_f32, 2)
							___pnlua_buffer_write_string(arg)
						} else {
							show_error("!!! PNLua: Error in state " + string(_id) + " while calling '" + function_name + "': Invalid data type in argument " + string(i) + " (" + string(arg) + ")", true)
						}
					}
				}
				
				i++
			}
		}
	}
	
	switch ___pnlua_internal_state_call(_id) {
		case -1:
			// There was an error calling the script, read the message from the buffer.
			___pnlua_buffer_start()
			show_error("!!! PNLua: Error in state " + string(_id) + " while calling " + (is_undefined(function_name) ? "" : "'" + function_name + "'") + ": " + buffer_read(global.___pnlua_buffer, buffer_string), true)
		break
		
		case 1:
			// Lua call has finished, get the return value from the buffer.
			___pnlua_buffer_start()
			
			switch buffer_read(global.___pnlua_buffer, buffer_f32) {
				case 0: // none, nil or unsupported
					return undefined
				
				case 1: // number
					return buffer_read(global.___pnlua_buffer, buffer_f32)
				
				case 2: // string
					return ___pnlua_buffer_read_string()
			}
		break
		
		case 2:
			// The state is waiting for a return value, send it over.
			show_debug_message("WAITING FOR RESUME")
			___pnlua_buffer_start()
			buffer_read(global.___pnlua_buffer, buffer_f32) // PNLuaState
			
			var callback_name = ___pnlua_buffer_read_string()
			var callback_args = undefined
			var args = buffer_read(global.___pnlua_buffer, buffer_f32)
			
			if args {
				callback_args = []
				
				repeat args {
					var arg_value = undefined
					
					switch buffer_read(global.___pnlua_buffer, buffer_f32) {
						case 1:
							arg_value = buffer_read(global.___pnlua_buffer, buffer_f32)
						break
						
						case 2:
							arg_value = ___pnlua_buffer_read_string()
						break
					}
					
					callback_args[@ array_length(callback_args)] = arg_value
				}
			}
			
			var callback_return = global.___pnlua_states[? _id][? callback_name](callback_args)
			
			___pnlua_buffer_start()
			
			if is_undefined(callback_return) {
				buffer_write(global.___pnlua_buffer, buffer_f32, 0)
			} else {
				if is_real(callback_return) {
					buffer_write(global.___pnlua_buffer, buffer_f32, 1)
					buffer_write(global.___pnlua_buffer, buffer_f32, callback_return)
				} else {
					if is_string(callback_return) {
						buffer_write(global.___pnlua_buffer, buffer_f32, 2)
						___pnlua_buffer_write_string(callback_return)
					} else {
						show_error("!!! PNLua: Error in state " + string(_id) + " while calling '" + callback_name + "' in '" + function_name + "': Invalid return value (" + string(callback_return) + ")", true)
					}
				}
			}
			
			//___pnlua_internal_state_resume(_id)
		break
	}
}