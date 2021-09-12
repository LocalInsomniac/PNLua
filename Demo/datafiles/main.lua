print("PNLua OK")

function create(id)
	show_message("Object #" .. id .. " says: You are running a barebones version of PNLua.")
end

function step(id)
	variable_instance_set(id, "random_number", math.random(100))
	
	local clock
	
	clock = math.rad(os.time() * 6)
	variable_instance_set(id, "spin_x", 160 + (math.cos(clock) * 64))
	variable_instance_set(id, "spin_y", 120 + (-math.sin(clock) * 64))
end