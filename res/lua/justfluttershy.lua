--Lua fx file for song "Just... Fluttershy"
--Pony48 source - justfluttershy.lua
--Copyright (c) 2014 Mark Hutcheson

local lasttime

local function jf_init()
	lasttime = -0.001
end
setglobal("jf_init", jf_init)

local function start(first, curtime)
	if first == true then
		showparticles("bgflash", false)
		pinwheelspeed(40)
	end
end

local function mid1(first, curtime)
	if first == true then
		resetparticles("bgponies")
		showparticles("bgponies", true)
		fireparticles("bgponies", true)
		pinwheelspeed(120)
	end
end

local mid2startcounter
local mid2currow
local mid2humtime = 0.7258
local function mid2(first, curtime)
	if first == true then
		mid2startcounter = curtime - mid2humtime
		mid2currow = -4
		pinwheelspeed(-120)
	end
	if curtime - mid2startcounter >= mid2humtime then
		mid2startcounter = mid2startcounter + mid2humtime
		mid2currow = mid2currow + 4
		if mid2currow > 12 then
			mid2currow = 0
		end
		
		--Reset color of all tiles
		for i = 0, 15 do
			settilecol(i, 1, 1, 1, 1)
		end
		--Change color of one row at a time
		for i = 0, 3 do
			settilecol(i + mid2currow, math.random(), math.random(), math.random(), 1)
		end
	end
end

local function mid3(first, curtime)
	if first == true then
		resetparticles("bgponies")
		showparticles("bgponies", true)
		fireparticles("bgponies", true)
		pinwheelspeed(-120)
	end
end

local function drop(first, curtime)
	if first == true then
		showparticles("bgponies", false)
		pinwheelspeed(0)
		--Reset color of all tiles
		for i = 0, 15 do
			settilecol(i, 1, 1, 1, 1)
		end
	end
end

local stuttering = false
local function main(first, curtime)
	if first == true then
		pinwheelspeed(240)
		showparticles("bgflash", true)
		fireparticles("bgflash", true)
	end
	if stuttering == true then
		setcameraxy(0,0)	--Cause this is called before stutter, it all works out
	end
	stuttering = false
end

local function stutter(first, curtime)
	setcameraxy(math.random()*3.0 - 1.5, math.random()*3.0 - 1.5)
	stuttering = true
end

local timetab = {
	{func = start, startat = 0, endat = 34.9},
	{func = mid1, startat = 34.9, endat = 46.4},
	{func = mid2, startat = 46.4, endat = 69.2},	--22.772/32
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
			val.func(lasttime < val.startat or lasttime > val.endat, curtime)
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