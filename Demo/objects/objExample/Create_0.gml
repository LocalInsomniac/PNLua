show_debug_overlay(true)

// Create a Lua state. This is what will handle our functions and Lua scripts.
my_state = pnlua_state_create()

pnlua_state_register(my_state, "test", function (args) {
	show_message(string(args[0] + args[1]))
})

// Load the test script and call the main script, including our Lua functions.
pnlua_state_load(my_state, "main.lua")
pnlua_state_call(my_state)
game_end()

exit

// Lua will manipulate these variables every step using variable_instance_set. (TODO)
random_number = 0
spin_x = 0
spin_y = 0