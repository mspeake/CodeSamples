
--[[
How to add your own textures:
	1. Place your texture into the Textures folder
	2. Copy the last line in the AlternateTextures table below, and paste it into
	   the line below it
	3. Increase the number in the [ ] by 1
	4. Change the name of the texture on that line to the name of your texture
	
So if the last line is: [7] = "Textures/Guardian.png",
and the name of your texture is: MyTex.png
then the new last line should look like: [8] = "Textures/MyTex.png",
--]]

AlternateTextures = {
	[1] = "Textures/Rift.png",
	[2] = "Textures/Cleric.png",
	[3] = "Textures/Mage.png",
	[4] = "Textures/Rogue.png",
	[5] = "Textures/Warrior.png",
	[6] = "Textures/Defiant.png",
	[7] = "Textures/Guardian.png",
	
};