--Lua fx file for song "ShySt3p VIP"
--Pony48 source - shyst3pvip.lua
--Copyright (c) 2014 Mark Hutcheson 

local starbgvel
local starbgaccel
local lasttime_ss
local maxstarspeed
local starszx
local starszy
local starszz
local wubfreq
local star_r
local star_g
local star_b
local star_a
local dest_star_r
local dest_star_g
local dest_star_b
local dest_star_a
local boardrot
local maxboardrot

local function ss_init()
	starbgvel = 0
	starbgaccel = 0
	lasttime_ss = -0.001
	maxstarspeed = 15
	starszx = 0.15
	starszy = 0.15
	starszz = 0
	wubfreq = 60
	star_r = 1
	star_g = 1
	star_b = 1
	star_a = 1
	dest_star_r = 1
	dest_star_g = 1
	dest_star_b = 1
	dest_star_a = 1
	boardrot = 0
	maxboardrot = 0
end
setglobal("ss_init", ss_init)

--Helper function for fading one color into another
local function fadecol(difftime, amt)
	--Red
	if star_r < dest_star_r then
		star_r = star_r + difftime * amt
		if star_r > dest_star_r then
			star_r = dest_star_r
		end
	elseif star_r > dest_star_r then
		star_r = star_r - difftime * amt
		if star_r < dest_star_r then
			star_r = dest_star_r
		end
	end
	--Green
	if star_g < dest_star_g then
		star_g = star_g + difftime * amt
		if star_g > dest_star_g then
			star_g = dest_star_g
		end
	elseif star_g > dest_star_g then
		star_g = star_g - difftime * amt
		if star_g < dest_star_g then
			star_g = dest_star_g
		end
	end
	--Blue
	if star_b < dest_star_b then
		star_b = star_b + difftime * amt
		if star_b > dest_star_b then
			star_b = dest_star_b
		end
	elseif star_b > dest_star_b then
		star_b = star_b - difftime * amt
		if star_b < dest_star_b then
			star_b = dest_star_b
		end
	end
	--Alpha
	if star_a < dest_star_a then
		star_a = star_a + difftime * amt
		if star_a > dest_star_a then
			star_a = dest_star_a
		end
	elseif star_a > dest_star_a then
		star_a = star_a - difftime * amt
		if star_a < dest_star_a then
			star_a = dest_star_a
		end
	end
end

--At beginning of song, stars are stopped. Start them moving
local function startstars(first, curtime, starttime, endtime, difftime)
	if first == true then
		starbgaccel = 20
		maxboardrot = 0
	end
	if starbgvel > maxstarspeed then
		starbgvel = maxstarspeed
		starbgaccel = 0
	end
end

local function mid1(first, curtime, starttime, endtime, difftime)
	if first == true then	--In case we've looped, start stars moving at normal speed again
		starbgvel = maxstarspeed
	end
end

--On first drop, stars slow to a stop
local function drumdrop1(first, curtime, starttime, endtime, difftime)
	if first == true then
		starbgaccel = -25
	end
	if starbgvel < 0 then
		starbgvel = 0
		starbgaccel = 0
	end
end

--After first drop is done, stars immediately start moving backwards fast
local function mid2(first, curtime, starttime, endtime, difftime)
	if first == true then
		starbgvel = -maxstarspeed*2
	end
	if maxboardrot < 2.5 then
		maxboardrot = maxboardrot + difftime * 5
		if maxboardrot > 2.5 then
			maxboardrot = 2.5
		end
	end
end

--On this change, stars slow down and accelerate forwards fast
local function mid3(first, curtime, starttime, endtime, difftime)
	if first == true then
		starbgaccel = 50
	end
	if starbgvel > maxstarspeed * 2 then
		starbgvel = maxstarspeed * 2
		starbgaccel = 0
	end
	if dest_star_r == star_r and 
	   dest_star_g == star_g and
	   dest_star_b == star_b then
		dest_star_r = math.random()
		dest_star_g = math.random()
		dest_star_b = math.random()
	end
	fadecol(difftime, 1)
	if maxboardrot < 4.5 then
		maxboardrot = maxboardrot + difftime * 5
		if maxboardrot > 4.5 then
			maxboardrot = 4.5
		end
	end
end

--On this change, stars slow down and move backwards slow
local function mid4(first, curtime, starttime, endtime, difftime)
	if first == true then
		starbgaccel = -40
		dest_star_r = 1	--Change back to white
		dest_star_g = 1
		dest_star_b = 1
		dest_star_a = 1
	end
	if starbgvel < -maxstarspeed then
		starbgaccel = 0
		starbgvel = -maxstarspeed
	end
	fadecol(difftime, 0.75)
end

--On second drum drop, stars slow down and rocket forwards for the main beat drop, gaining trails
local function drumdrop2(first, curtime, starttime, endtime, difftime)
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
	if maxboardrot < 5 then
		maxboardrot = maxboardrot + difftime * 5
		if maxboardrot > 5 then
			maxboardrot = 5
		end
	end
end

local function main1(first, curtime, starttime, endtime, difftime)
	
end

--Exiting this, stars accelerate forwards like mad and trail size increases
local function exitmain1(first, curtime, starttime, endtime, difftime)
	if first == true then
		starbgaccel = 25
	end
	starszz = starbgvel * 0.2
	if maxboardrot < 7.5 then
		maxboardrot = maxboardrot + difftime * 5
		if maxboardrot > 7.5 then
			maxboardrot = 7.5
		end
	end
end

--Stars then rocket backwards really insanely fast
local function main2(first, curtime, starttime, endtime, difftime)
	if first == true then
		starbgaccel = 0
		starbgvel = -maxstarspeed * 5
	end
	if starszz > 0.1 then
		starszz = starszz / 1.4
	else
		starszz = 0
	end
end

--Stars then go forwards pretty slow
local function main3(first, curtime, starttime, endtime, difftime)
	if first == true then
		starbgvel = maxstarspeed / 2
	end
	if starszx < 0.25 then
		starszx = starszx + difftime * 0.1
		starszy = starszx
	else
		starszx = 0.25
		starszy = starszx
	end
	--Change stars to a random color
	if dest_star_r == star_r and 
	   dest_star_g == star_g and
	   dest_star_b == star_b then
		dest_star_r = math.random()
		dest_star_g = math.random()
		dest_star_b = math.random()
	end
	fadecol(difftime, 7.5)
	if maxboardrot > 3.5 then
		maxboardrot = maxboardrot - difftime * 5
		if maxboardrot < 3.5 then
			maxboardrot = 3.5
		end
	end
end

--Stars then accelerate backwards, forming trails
local function drumdrop3(first, curtime, starttime, endtime, difftime)
	if first == true then
		starbgvel = 0
		starbgaccel = -75
		dest_star_r = 0.3490	--Stars change to blue
		dest_star_g = 0.3882
		dest_star_b = 1
		dest_star_a = 1
	end
	if starbgvel < -maxstarspeed * 3 then
		starbgvel = -maxstarspeed * 3
		starbgaccel = 0
	end
	if starszx > 0.15 then
		starszx = starszx - difftime * 0.1
		starszy = starszx
	else
		starszx = 0.15
		starszy = starszx
	end
	fadecol(difftime, 0.5)
	starszz = -starbgvel * 0.3	--Trails face forwards, cause otherwise they look funny when appearing
	if maxboardrot < 7.5 then
		maxboardrot = maxboardrot + difftime * 5
		if maxboardrot > 7.5 then
			maxboardrot = 7.5
		end
	end
	--TODO beatbounce size
end

local function main4(first, curtime, starttime, endtime, difftime)
	--TODO
end

--Exiting this, stars accelerate backwards and trail size increases
local function exitmain4(first, curtime, starttime, endtime, difftime)
	if first == true then
		starbgaccel = -25
	end
	starszz = -starbgvel * 0.3
end

--Then, stars move forwards at a normal speed and change back to white
local function midend(first, curtime, starttime, endtime, difftime)
	if first == true then
		starbgaccel = 0
		starbgvel = maxstarspeed
		dest_star_r = 1
		dest_star_g = 1
		dest_star_b = 1
		dest_star_a = 1
	end
	if starszz > 0.1 then
		starszz = starszz / 1.05
	else
		starszz = 0
	end
	fadecol(difftime, 0.75)
	if maxboardrot > 2.5 then
		maxboardrot = maxboardrot - difftime * 5
		if maxboardrot < 2.5 then
			maxboardrot = 2.5
		end
	end
end

--At end-of-song wubby effect, stars step forwards in a sinusoidal pattern
local function exitmidend(first, curtime, starttime, endtime, difftime)
	if first == true then
		starbgaccel = 0
		starbgvel = maxstarspeed
	end
	starbgvel = maxstarspeed + math.sin(wubfreq * curtime) * (curtime - starttime) * 15
end

--At end of song, stars slow down to a crawl, to pick speed back up again on loop
local function ending(first, curtime, starttime, endtime, difftime)
	starbgaccel = 0
	starbgvel = maxstarspeed / 10
	if maxboardrot > 0 then
		maxboardrot = maxboardrot - difftime * 5
		if maxboardrot < 0 then
			maxboardrot = 0
		end
	end
end

local timetab = {
	{func = startstars, startat = 4.606, endat = 7.9},
	{func = mid1, startat = 7.9, endat = 32.383},
	{func = drumdrop1, startat = 32.383, endat = 33.427},
	{func = mid2, startat = 33.427, endat = 65.903},
	{func = mid3, startat = 65.903, endat = 100.207},
	{func = mid4, startat = 100.207, endat = 131.5},
	{func = drumdrop2, startat = 131.5, endat = 133.6},
	{func = main1, startat = 133.6, endat = 160.8},
	{func = exitmain1, startat = 160.8, endat = 166.989},
	{func = main2, startat = 166.989, endat = 200.381},
	{func = main3, startat = 200.381, endat = 231.689},
	{func = drumdrop3, startat = 231.689, endat = 233.772},
	{func = main4, startat = 233.772, endat = 260.779},
	{func = exitmain4, startat = 260.779, endat = 267.165},
	{func = midend, startat = 267.165, endat = 294.849},
	{func = exitmidend, startat = 296.806, endat = 300.557},
	{func = ending, startat = 300.557, endat = 305},
}

local function ss_update(curtime)
	starbgvel = starbgvel + (curtime - lasttime_ss) * starbgaccel
	for key,val in ipairs(timetab) do
		if curtime > val.startat and curtime < val.endat then
			val.func(lasttime_ss < val.startat or lasttime_ss > val.endat, curtime, val.startat, val.endat, curtime-lasttime_ss)
		end
	end
	boardrot = math.sin(curtime) * maxboardrot
	setstarbgvel(starbgvel)
	setstarbgsize(starszx, starszy, starszz)
	setstarbgcol(star_r, star_g, star_b, star_a)
	setboardrot(boardrot)
	lasttime_ss = curtime
end
setglobal("ss_update", ss_update)
























