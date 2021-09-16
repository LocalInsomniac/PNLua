show_debug_overlay(true)

// Create a Lua state. This is what will handle our functions and Lua scripts.
my_state = pnlua_state_create()

// Load the test script and call the main script, including our Lua functions.
pnlua_state_load(my_state, "main.lua")
pnlua_state_call(my_state)
pnlua_state_call(my_state, "foo")
show_debug_message(string(pnlua_state_call(my_state, "cluster", 1.337, 666, 420.69)))
game_end()

exit

// Lua will manipulate these variables every step using variable_instance_set. (TODO)
random_number = 0
spin_x = 0
spin_y = 0