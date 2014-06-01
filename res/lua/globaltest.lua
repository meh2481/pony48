-- for safety: warn at undefined variable accesses in global namespace

local function msgbox(...) 
	print(...) 
end

local msgbox_internal = msgbox

local function looksLikeGlobal(s)
	return type(s) == "string" and not s:match("[^_%u%d]")
end

local function getbt(msg, level)
	if debug then
		return debug.traceback(msg, (level or 1) + 1)
	end
	return "[No stacktrace available]\n\n" .. msg
end

local function msgbox_bt(str, level)
	return msgbox_internal(getbt(str, level or 2))
end

rawset(_G, "msgbox", msgbox_bt)

local gmeta = {

	__index = function(t, k)
		if not looksLikeGlobal(k) then
			msgbox_bt("WARNING: Access to undefined global variable '" .. tostring(k) .. "'\n", 3)
		end
	end,
	
	__newindex = function(t, k, v)
		if not looksLikeGlobal(k) then
			msgbox_bt("WARNING: Setting global variable '" .. tostring(k) .. "' (type: " .. type(v) .. ") = " .. tostring(v) .. "\n", 3)
		end
		rawset(t, k, v)
	end,
}

rawset(_G, "setglobal", function(name, v)
	rawset(_G, name, v)
end)

setmetatable(_G, gmeta)


