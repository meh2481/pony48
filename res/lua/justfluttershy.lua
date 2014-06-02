--Lua fx file for song "Just... Fluttershy"
--Pony48 source - justfluttershy.lua
--Copyright (c) 2014 Mark Hutcheson

local function jf_init()
	--print("init")
end
setglobal("jf_init", jf_init)

local function start(first)
	if first == true then
		pinwheelspeed(50)
	end
end

local function mid1(first)
	if first == true then
		resetparticles("bgponies")
		showparticles("bgponies", true)
		fireparticles("bgponies", true)
		pinwheelspeed(100)
	end
end

local function mid2(first)
	if first == true then
		pinwheelspeed(-100)
	end
end

local function mid3(first)
	if first == true then
		resetparticles("bgponies")
		showparticles("bgponies", true)
		fireparticles("bgponies", true)
		pinwheelspeed(-100)
	end
end

local function drop(first)
	if first == true then
		showparticles("bgponies", false)
		pinwheelspeed(0)
	end
end

local function main(first)
	if first == true then
		pinwheelspeed(200)
	end
	
end

local function stutter(first)
	
end

local lasttime = -0.001
local timetab = {
	[0]={func = start, endat = 34.9},
	[34.9]={func = mid1, endat = 46.4},
	[46.4]={func = mid2, endat = 69.2},
	[69.2]={func = drop, endat = 69.848},
	[69.848]={func = main, endat = 116.08},
	[116.308]={func = start, endat = 140.895},
	[140.895]={func = mid3, endat = 163.863},
	[163.863]={func = drop, endat = 164.395},
	[164.395]={func = main, endat = 210.936},
	[167.306]={func = stutter, endat = 168.714},
	[173.847]={func = stutter, endat = 174.565},
	[178.943]={func = stutter, endat = 180.356},
	[185.485]={func = stutter, endat = 186.180},
	[210.936]={func = start, endat = 224},
}

local function jf_update(curtime)
	for key,val in pairs(timetab) do
		if curtime > key and curtime < val.endat then
			val.func(lasttime < key or lasttime > val.endat)
		end
	end
	lasttime = curtime
end
setglobal("jf_update", jf_update)

--[[
	<mid>
		<fire particle="test"/>
		<seg time="0.725">
			<colorize tiles="0" color="255,0,0,255"/>
			<colorize tiles="1" color="255,128,0,255"/>
			<colorize tiles="2" color="255,255,0,255"/>
			<colorize tiles="3" color="0,255,0,255"/>
		</seg>
		<seg time="0.725">
			<colorize tiles="4" color="0,255,255,255"/>
			<colorize tiles="5" color="0,0,255,255"/>
			<colorize tiles="6" color="128,0,255,255"/>
			<colorize tiles="7" color="255,0,255,255"/>
		</seg>
		<seg time="0.725">
			<colorize tiles="11" color="0,255,255,255"/>
			<colorize tiles="10" color="0,0,255,255"/>
			<colorize tiles="9" color="128,0,255,255"/>
			<colorize tiles="8" color="255,0,255,255"/>
		</seg>
		<seg time="0.725">
			<colorize tiles="15" color="255,0,0,255"/>
			<colorize tiles="14" color="255,128,0,255"/>
			<colorize tiles="13" color="255,255,0,255"/>
			<colorize tiles="12" color="0,255,0,255"/>
		</seg>
	</mid>
--]]