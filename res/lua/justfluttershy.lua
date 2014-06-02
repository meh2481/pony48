--Lua fx file for song "Just... Fluttershy"
--Pony48 source - justfluttershy.lua
--Copyright (c) 2014 Mark Hutcheson

local function jf_init()
	print("init")
end
setglobal("jf_init", jf_init) 

local midstart = 46.530
local midend = 69.311

local function jf_update(curtime)
	if curtime > midstart and curtime < midend then
		fireparticles("test", true)
	elseif curtime > midend then
		showparticles("test", false)
	end
end
setglobal("jf_update", jf_update)

--[[
	<drop start="46.530" end="69.849" type="mid"/>
	<drop start="141.097" end="164.394" type="mid"/>
	<drop start="69.849" end="116.389" type="main"/>
	<drop start="164.394" end="210.923" type="main"/>
	<drop start="167.292" end="168.727" type="stutter"/>
	<drop start="173.846" end="174.533" type="stutter"/>
	<drop start="178.942" end="180.362" type="stutter"/>
	<drop start="185.476" end="186.195" type="stutter"/>
	<main>
		<halt particles="test"/>
	</main>
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
	<stutter />
--]]