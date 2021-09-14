show_debug_overlay(true)

// Create a Lua state. This is what will handle our functions and Lua scripts.
my_state = pnlua_state_create()

// Register the functions we want to use in our Lua script.
/*pnlua_state_register(my_state, "show_message", function () {
	show_message(ARG[0])
})
pnlua_state_register(my_state, "variable_instance_set", function () {
	variable_instance_set(id, ARG[1], ARG[2])
})*/

// Load the test script, pass our instance ID to the "create" function and call it.
pnlua_state_load(my_state, "main.lua")
//pnlua_state_call(my_state, "create", id)

// Lua will manipulate these variables every step using variable_instance_set.
random_number = 0
spin_x = 0
spin_y = 0