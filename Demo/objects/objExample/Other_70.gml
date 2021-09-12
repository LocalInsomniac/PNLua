// This event is where PNLua handles errors and GML function calls.
switch async_load[? "event_type"] {
	case "PNLuaError":
		// We've received an error from Lua, display it in a GameMaker error message.
		show_error(async_load[? "error_message"], true)
	break
	
	case "PNLuaCall":
		/* We've received a GML function call from Lua, send the async ds_map over to
		   PNLua for processing. */
		pnlua_process_call(async_load)
	break
}