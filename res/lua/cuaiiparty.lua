--Lua fx file for song "Party in the Stars"
--Pony48 source - cuaiiparty.lua
--Copyright (c) 2014 Mark Hutcheson 

local lasttime_ss
local boardrot
local maxboardrot
local startcol_r
local startcol_g
local startcol_b
local startcol_a
local destcol_r
local destcol_g
local destcol_b
local destcol_a
local boardcol_r
local boardcol_g
local boardcol_b
local boardcol_a
local tilecol_r
local tilecol_g
local tilecol_b
local tilecol_a
local fadedalready

local function pits_init()
	fadedalready = false

	boardcol_r = 0.7
	boardcol_g = 0.7
	boardcol_b = 0.7
	boardcol_a = 0.5
	
	tilecol_r = 0.5
	tilecol_g = 0.5
	tilecol_b = 0.5
	tilecol_a = 0.5
	
	lasttime_ss = -0.001
	boardrot = 0
	maxboardrot = 0
	--setboardcol(0.93,0.27,0.54,0.5)
	--for i = 0,16 do
	--	settilebgcol(i,0.95,0.72,0.81,0.5)
	--end
end
setglobal("pits_init", pits_init)

local function setdestcol(r,g,b,a)
	destcol_r = r
	destcol_g = g
	destcol_b = b
	destcol_a = a
end

local function setstartcol(r,g,b,a)
	startcol_r = r
	startcol_g = g
	startcol_b = b
	startcol_a = a
end

local function fadecol(difftime, amt)
	--Red
	if startcol_r < destcol_r then
		startcol_r = startcol_r + difftime * amt
		if startcol_r > destcol_r then
			startcol_r = destcol_r
		end
	elseif startcol_r > destcol_r then
		startcol_r = startcol_r - difftime * amt
		if startcol_r < destcol_r then
			startcol_r = destcol_r
		end
	end
	--Green
	if startcol_g < destcol_g then
		startcol_g = startcol_g + difftime * amt
		if startcol_g > destcol_g then
			startcol_g = destcol_g
		end
	elseif startcol_g > destcol_g then
		startcol_g = startcol_g - difftime * amt
		if startcol_g < destcol_g then
			startcol_g = destcol_g
		end
	end
	--Blue
	if startcol_b < destcol_b then
		startcol_b = startcol_b + difftime * amt
		if startcol_b > destcol_b then
			startcol_b = destcol_b
		end
	elseif startcol_b > destcol_b then
		startcol_b = startcol_b - difftime * amt
		if startcol_b < destcol_b then
			startcol_b = destcol_b
		end
	end
	--Alpha
	if startcol_a < destcol_a then
		startcol_a = startcol_a + difftime * amt
		if startcol_a > destcol_a then
			startcol_a = destcol_a
		end
	elseif startcol_a > destcol_a then
		startcol_a = startcol_a - difftime * amt
		if startcol_a < destcol_a then
			startcol_a = destcol_a
		end
	end
end

local function start(first, curtime, starttime, endtime, difftime)
	maxboardrot = 0
end

local function starthit(first, curtime, starttime, endtime, difftime)
	maxboardrot = 0
end

local function secondhit(first, curtime, starttime, endtime, difftime)
	maxboardrot = 0
end

local function pianostart(first, curtime, starttime, endtime, difftime)
	if maxboardrot < 2.5 then
		maxboardrot = maxboardrot + difftime * 0.5
		if maxboardrot > 2.5 then
			maxboardrot = 2.5
		end
	end
end

local function kickstart1(first, curtime, starttime, endtime, difftime)

end

local function wubwubstart1(first, curtime, starttime, endtime, difftime)
	if maxboardrot < 5 then
		maxboardrot = maxboardrot + difftime * 0.5
		if maxboardrot > 5 then
			maxboardrot = 5
		end
	end
end

local function wubshortpause1(first, curtime, starttime, endtime, difftime)
	maxboardrot = 0
end

local function wubgrindy2(first, curtime, starttime, endtime, difftime)
	if maxboardrot < 5.5 then
		maxboardrot = maxboardrot + difftime * 0.5
		if maxboardrot > 5.5 then
			maxboardrot = 5.5
		end
	end
end

local function calm1(first, curtime, starttime, endtime, difftime)
	if maxboardrot > .5 then
		maxboardrot = maxboardrot - difftime * 0.5
		if maxboardrot < .5 then
			maxboardrot = .5
		end
	end
end

local function pianowub2(first, curtime, starttime, endtime, difftime)
	if maxboardrot < 2.5 then
		maxboardrot = maxboardrot + difftime * 0.5
		if maxboardrot > 2.5 then
			maxboardrot = 2.5
		end
	end
end

local function drawnoutpause2(first, curtime, starttime, endtime, difftime)
	maxboardrot = 0
end

local function buildup1(first, curtime, starttime, endtime, difftime)
	if maxboardrot < 2.5 then
		maxboardrot = maxboardrot + difftime * 0.5
		if maxboardrot > 2.5 then
			maxboardrot = 2.5
		end
	end
end

local function fastpiano3(first, curtime, starttime, endtime, difftime)
	if maxboardrot < 3.5 then
		maxboardrot = maxboardrot + difftime * 0.5
		if maxboardrot > 3.5 then
			maxboardrot = 3.5
		end
	end
end

local function wubwub3(first, curtime, starttime, endtime, difftime)
	if maxboardrot < 5.5 then
		maxboardrot = maxboardrot + difftime * 0.5
		if maxboardrot > 5.5 then
			maxboardrot = 5.5
		end
	end
end

local function wubstopstillintense(first, curtime, starttime, endtime, difftime)
	if maxboardrot > 5 then
		maxboardrot = maxboardrot - difftime * 0.5
		if maxboardrot < 5 then
			maxboardrot = 5
		end
	end
	fadedalready = true
end

local timetab = {
	{func = start, startat = 0, endat = 2.898},
	{func = starthit, startat = 2.898, endat = 14.167},
	{func = secondhit, startat = 14.167, endat = 36.781},
	{func = pianostart, startat = 36.781, endat = 53.715},
	{func = kickstart1, startat = 53.715, endat = 59.369},
	{func = wubwubstart1, startat = 59.369, endat = 104.525},
	{func = wubshortpause1, startat = 104.525, endat = 107.369},
	{func = wubgrindy2, startat = 107.369, endat = 127.134},
	{func = calm1, startat = 127.134, endat = 149.722},
	{func = pianowub2, startat = 149.722, endat = 172.310},
	{func = drawnoutpause2, startat = 172.310, endat = 177.956},
	{func = buildup1, startat = 177.956, endat = 194.898},
	{func = fastpiano3, startat = 194.898, endat = 217.483},
	{func = wubwub3, startat = 217.483, endat = 285.252},
	{func = wubstopstillintense, startat = 285.252, endat = 307.839}
}

local function pits_update(curtime)
	for key,val in ipairs(timetab) do
		if curtime > val.startat and curtime < val.endat then
			val.func(lasttime_ss < val.startat or lasttime_ss > val.endat, curtime, val.startat, val.endat, curtime-lasttime_ss)
		end
	end
	boardrot = math.sin(curtime) * maxboardrot
	setboardrot(boardrot)
	
	if fadedalready == false then
	
		--Update colors
		setstartcol(boardcol_r,boardcol_g,boardcol_b,boardcol_a)
		setdestcol(0.93,0.27,0.54,0.5)	--We want to gradually fade into pink
		fadecol(curtime-lasttime_ss, 0.001)
		boardcol_r = startcol_r
		boardcol_g = startcol_g
		boardcol_b = startcol_b
		boardcol_a = startcol_a
		setboardcol(boardcol_r,boardcol_g,boardcol_b,boardcol_a)
		
		setstartcol(tilecol_r, tilecol_g, tilecol_b, tilecol_a)
		setdestcol(0.95,0.72,0.81,0.5)
		fadecol(curtime-lasttime_ss, 0.001)
		tilecol_r = startcol_r
		tilecol_g = startcol_g
		tilecol_b = startcol_b
		tilecol_a = startcol_a
		for i = 0,16 do
			settilebgcol(i,tilecol_r,tilecol_g,tilecol_b,tilecol_a)
		end
	
	end
	
	lasttime_ss = curtime
end
setglobal("pits_update", pits_update)
























