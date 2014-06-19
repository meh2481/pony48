--Lua fx file for song "ShySt3p VIP"
--Pony48 source - shyst3pvip.lua
--Copyright (c) 2014 Mark Hutcheson 

local starbgvel
local starbgaccel
local lasttime
local maxstarspeed
local starszx
local starszy
local starszz

local function ss_init()
	starbgvel = 0
	starbgaccel = 0
	lasttime = -0.001
	maxstarspeed = 15
	starszx = 0.15
	starszy = 0.15
	starszz = 0
end
setglobal("ss_init", ss_init)

--At beginning of song, stars are stopped. Start them moving
local function startstars(first, curtime, starttime, endtime)
	if first == true then
		starbgaccel = 20
	end
	if starbgvel > maxstarspeed then
		starbgvel = maxstarspeed
		starbgaccel = 0
	end
end

local function mid1(first, curtime, starttime, endtime)
	
end

--On first drop, stars slow to a stop
local function drumdrop1(first, curtime, starttime, endtime)
	if first == true then
		starbgaccel = -25
	end
	if starbgvel < 0 then
		starbgvel = 0
		starbgaccel = 0
	end
end

--After first drop is done, stars immediately start moving backwards fast
local function mid2(first, curtime, starttime, endtime)
	if first == true then
		starbgvel = -maxstarspeed*2
	end
end

--On this change, stars slow down and accelerate forwards fast
local function mid3(first, curtime, starttime, endtime)
	if first == true then
		starbgaccel = 50
	end
	if starbgvel > maxstarspeed * 2 then
		starbgvel = maxstarspeed * 2
		starbgaccel = 0
	end
end

--On this change, stars slow down and move backwards slow
local function mid4(first, curtime, starttime, endtime)
	if first == true then
		starbgaccel = -40
	end
	if starbgvel < -maxstarspeed then
		starbgaccel = 0
		starbgvel = -maxstarspeed
	end
end

--On second drum drop, stars slow down and rocket forwards for the main beat drop
local function drumdrop2(first, curtime, starttime, endtime)
	if first == true then
		starbgaccel = 30
	end
	if starbgvel > maxstarspeed * 3 then
		starbgaccel = 0
		starbgvel = maxstarspeed * 3
	end
	--If stars are going forwards, draw trail
	if starbgvel > 0 then
		starszz = starbgvel * 0.2
	end
end

local function main1(first, curtime, starttime, endtime)
end

local timetab = {
	{func = startstars, startat = 4.606, endat = 7.9},
	{func = mid1, startat = 7.9, endat = 32.383},
	{func = drumdrop1, startat = 32.383, endat = 33.427},
	{func = mid2, startat = 33.427, endat = 65.903},
	{func = mid3, startat = 65.903, endat = 100.207},
	{func = mid4, startat = 100.207, endat = 131.5},
	{func = drumdrop2, startat = 131.5, endat = 133.6},
	{func = main1, startat = 133.6, endat = 200},
}

local function ss_update(curtime)
	starbgvel = starbgvel + (curtime - lasttime) * starbgaccel
	for key,val in ipairs(timetab) do
		if curtime > val.startat and curtime < val.endat then
			val.func(lasttime < val.startat or lasttime > val.endat, curtime, val.startat, val.endat)
		end
	end
	setstarbgvel(starbgvel)
	setstarbgsize(starszx, starszy, starszz)
	
	lasttime = curtime
end
setglobal("ss_update", ss_update)
























