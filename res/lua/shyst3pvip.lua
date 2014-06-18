--Lua fx file for song "ShySt3p VIP"
--Pony48 source - shyst3pvip.lua
--Copyright (c) 2014 Mark Hutcheson 

local starbgvel
local starbgaccel
local lasttime
local maxstarspeed

local function ss_init()
	starbgvel = 0
	starbgaccel = 0
	lasttime = -0.001
	maxstarspeed = 15
end
setglobal("ss_init", ss_init)

local function startstars(first, curtime, starttime, endtime)
	if first == true then
		starbgaccel = 20
	end
	if starbgvel > maxstarspeed then
		starbgvel = maxstarspeed
		starbgaccel = 0
	end
end

local function drumdrop1(first, curtime, starttime, endtime)
	if first == true then
		starbgaccel = -25
	end
	if starbgvel < 0 then
		starbgvel = 0
		starbgaccel = 0
	end
end

local function main1(first, curtime, starttime, endtime)
	
end

local function main2(first, curtime, starttime, endtime)
	if first == true then
		starbgvel = -maxstarspeed*2
	end
end

local function main3(first, curtime, starttime, endtime)
	if first == true then
		starbgaccel = 50
	end
	if starbgvel > maxstarspeed * 2 then
		starbgvel = maxstarspeed * 2
		starbgaccel = 0
	end
end

local timetab = {
	{func = startstars, startat = 4.606, endat = 7.9},
	{func = main1, startat = 7.9, endat = 32.383},
	{func = drumdrop1, startat = 32.383, endat = 33.427},
	{func = main2, startat = 33.427, endat = 65.903},
	{func = main3, startat = 65.903, endat = 90},
}

local function ss_update(curtime)
	starbgvel = starbgvel + (curtime - lasttime) * starbgaccel
	for key,val in ipairs(timetab) do
		if curtime > val.startat and curtime < val.endat then
			val.func(lasttime < val.startat or lasttime > val.endat, curtime, val.startat, val.endat)
		end
	end
	setstarbgvel(starbgvel);
	lasttime = curtime
end
setglobal("ss_update", ss_update)
























