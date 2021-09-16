print("PNLua OK")

function foo()
    print("bar")
end

function cluster(a, b, c)
    print(a .. " + " .. b .. " + " .. c)

    return (a + b + c)
end