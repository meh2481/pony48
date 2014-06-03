--Lua fx file for song "Just... Fluttershy"
--Pony48 source - justfluttershy.lua
--Copyright (c) 2014 Mark Hutcheson

local function jf_init()
	--print("init")
end
setglobal("jf_init", jf_init)

local function start(first)
	if first == true then
		showparticles("bgflash", false)
		pinwheelspeed(40)
	end
end

local function mid1(first)
	if first == true then
		resetparticles("bgponies")
		showparticles("bgponies", true)
		fireparticles("bgponies", true)
		pinwheelspeed(120)
	end
end

local function mid2(first)
	if first == true then
		pinwheelspeed(-120)
	end
end

local function mid3(first)
	if first == true then
		resetparticles("bgponies")
		showparticles("bgponies", true)
		fireparticles("bgponies", true)
		pinwheelspeed(-120)
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
		pinwheelspeed(240)
		showparticles("bgflash", true)
		fireparticles("bgflash", true)
	end
	setcameraxy(0,0)	--Cause this is called before stutter, it all works out
end

local function stutter(first)
	setcameraxy(math.random(-2,2), math.random(-2,2))
end

local lasttime = -0.001
local timetab = {
	{func = start, startat = 0, endat = 34.9},
	{func = mid1, startat = 34.9, endat = 46.4},
	{func = mid2, startat = 46.4, endat = 69.2},
	{func = drop, startat = 69.2, endat = 69.848},
	{func = main, startat = 69.848, endat = 116.08},
	{func = start, startat = 116.308, endat = 140.895},
	{func = mid3, startat = 140.895, endat = 163.863},
	{func = drop, startat = 163.863, endat = 164.395},
	{func = main, startat = 164.395, endat = 210.936},
	{func = stutter, startat = 167.306, endat = 168.714},
	{func = stutter, startat = 173.847, endat = 174.565},
	{func = stutter, startat = 178.943, endat = 180.356},
	{func = stutter, startat = 185.485, endat = 186.180},
	{func = start, startat = 210.936, endat = 224},
}

local function jf_update(curtime)
	for key,val in ipairs(timetab) do
		if curtime > val.startat and curtime < val.endat then
			val.func(lasttime < val.startat or lasttime > val.endat)
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