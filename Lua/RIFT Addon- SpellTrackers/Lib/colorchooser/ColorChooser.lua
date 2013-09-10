
Library = Library or {}
Library.ColorChooser = {}

local DEFAULT_PIXEL = 6

-- Convert a 32bit or 16bit color value to its individual RGB values
-- @usage Color2RGBA(color, thirtyTwoBit)
-- @param color The color value, i.e. 0xffffff
-- @param thirtyTwoBit True if color is in 32 bit format
-- @return Returns 4 values: red, green, blue, and alpha
local function Color2RGBA(color, thirtyTwoBit)
	
	local r, g, b, a = 1, 1, 1, 1
		
	if (color == nil) then
		return r, g, b, a;
	end

	local l = color
		
	if thirtyTwoBit then
		--/* RGBA */
		a = bit.band(bit.rshift(l, 24), 0xff) / 256
		r = bit.band(bit.rshift(l, 16), 0xff) / 256
		g = bit.band(bit.rshift(l, 8), 0xff) / 256
		b = bit.band(bit.rshift(l, 0), 0xff) / 256
	else
		--/* RGB */
		r = bit.band(bit.rshift(l, 16), 0xff) / 256
		g = bit.band(bit.rshift(l, 8), 0xff) / 256
		b = bit.band(bit.rshift(l, 0), 0xff) / 256
		a = 0xff / 256
	end
	return r, g, b, a
end


-- Convert RGBA values into its 32bit or 16bit value
-- @usage RGBA2Color(red, green, blue, [alpha])
-- @param red The color's red value
-- @param green The color's green value
-- @param blue The color's blue value
-- @param alpha The color's alpha value. This parameter is optional.
-- @return A color value, i.e. 0xffffff
local function RGBA2Color(red, green, blue, alpha)
	red = bit.band((red or 1) * 256, 0xff)
	green = bit.band((green or 1) * 256, 0xff)
	blue = bit.band((blue or 1) * 256, 0xff)
	alpha = alpha and bit.band(alpha * 256, 0xff)
	if alpha then
		local r = bit.lshift(red, 16)
		local g = bit.lshift(green, 8)
		local b = blue
		local a = bit.lshift(alpha, 24)
		return bit.bor(r, bit.bor(g, bit.bor(b, a)))	
	else
		local r = bit.lshift(red, 16)
		local g = bit.lshift(green, 8)
		local b = blue
		local a = 0xff000000
		return bit.bor(r, bit.bor(g, bit.bor(b, a)))
	end
end

--- Return RGB values from HSV colorspace values.
-- @usage HSV2RGB(h, s, v)
-- @param h Hue value, ranging from 0 to 1
-- @param s Satration value, ranging from 0 to 1
-- @param v Value value, ranging from 0 to 1
-- @return Red, green, and blue values from the HSV values provided.
local floor = math.floor
local function HSV2RGB(h, s, v)
	local i
	local f, w, q, t
	local hue
	
	if s == 0.0 then
		r = v
		g = v
		b = v
	else
		hue = h
		if hue == 1.0 then
			hue = 0.0
		end
		hue = hue * 6.0
		
		i = floor(hue)
		f = hue - i
		w = v * (1.0 - s)
		q = v * (1.0 - (s * f))
		t = v * (1.0 - (s * (1.0 - f)))
		if i == 0 then
			r = v
			g = t
			b = w
		elseif i == 1 then
			r = q
			g = v
			b = w
		elseif i == 2 then
			r = w
			g = v
			b = t
		elseif i == 3 then
			r = w
			g = q
			b = v
		elseif i == 4 then
			r = t
			g = w
			b = v
		elseif i == 5 then
			r = v
			g = w
			b = q
		end
	end
	
	return r, g, b
end

local function rgb_max(r, g, b)
	if r > g then 
		return r > b and r or b
	else
		return g > b and g or b
	end
end
local function rgb_min(r, g, b)
	if r < g then
		return r < b and r or b
	else
		return g < b and g or b
	end
end

--- Returns HSV colorspace values from RGB values
-- @usage RGB2HSV(r, g, b)
-- @param r Red value, ranging from 0 to 1
-- @param g Green value, ranging from 0 to 1
-- @param b Blue value, ranging from 0 to 1
-- @return Hue, Saturation, and Value values from RGB values
local function RGB2HSV(r, g, b, a)
	local min, max, delta
	local h, s, v
	
	max = rgb_max(r, g, b)
	min = rgb_min(r, g, b)
	
	v = max
	delta = max - min
	
	if delta > 0.0001 then
		s = delta / max
		if r == max then
			h = (g - b) / delta
			if h < 0.0 then
				h = h + 6.0
			end
		elseif g == max then
			h = 2.0 + (b - r) / delta
		elseif b == max then
			h = 4.0 + (r - g) / delta
		end
		h = h / 6.0
	else
		s = 0.0
		h = 0.0
	end
	
	return h, s, v, a
end

local copy = function(tbl)
	local new = {}
	for k, v in pairs(tbl) do
		new[k] = v
	end
	return new
end

--- Create a color chooser. 
-- Provide a handler function, pixel size, and an indication whether to use websafe colors or not.
-- @param handler - This function will be called when a image point is clicked.
-- @param pixel - How large each pixel should be.
-- @param websafe - Whether to use websafe colors or not.
Library.ColorChooser.CreateWidget = function(frame, handler, pixel, websafe)
	local r, g, b, a = 0, 0, 0, 1
	local pixel = pixel or DEFAULT_PIXEL
	frame.textures = frame.textures or {}

	local x, y = 0, 0
	local count = 0
	local draw = function(pixel)
		for h = 1, 360, 10 do
			local h = h % 360
			local count2 = 0
			for v = 100, 1, -(100/8) do
				for s = 100, 100, 1 do
					local h, s, v = h/360, s/100, v/100
					y = count * pixel
					count = count + 1
						
					local r, g, b = HSV2RGB(h, s, v)
					r, g, b = r * 255, g * 255, b * 255
					if websafe then
						local max_color_component_value = 255
						local quantum = max_color_component_value / 5
						r = quantum * math.floor((r + (quantum / 2)) / quantum)
						g = quantum * math.floor((g + (quantum / 2)) / quantum)
						b = quantum * math.floor((b + (quantum / 2)) / quantum)
					end
					r, g, b = r / 255, g / 255, b / 255
	
					local texture = select(2, table.remove(frame.textures)) or UI.CreateFrame("Frame", "Pixel", frame)
					texture:ClearAll()
					texture:SetMouseMasking("full")	
					texture:SetPoint("TOPLEFT", frame, "TOPLEFT", x, y)
					texture:SetBackgroundColor(r, g, b)
					texture:SetWidth(pixel)
					texture:SetHeight(pixel)
			
					texture.Event.LeftClick = function()
						handler(r, g, b)
					end
					table.insert(frame.textures, texture)
				end
			end
			x = x + pixel
			count = 0
		end
		count = 0
		for v = 99, 0, -(100/8)  do
			local h, s, v = 0, 0, v
	
			y = count * pixel
			count = count + 1
	
			local texture = select(2, table.remove(frame.textures)) or UI.CreateFrame("Frame", "Pixel", frame)
			texture:ClearAll()
			texture:SetMouseMasking("full")
			texture:SetPoint("TOPLEFT", frame, "TOPLEFT", x, y)
			
			local r, g, b = HSV2RGB(h/360, s/100, v/100)
			texture:SetBackgroundColor(r, g, b)
			texture:SetWidth(pixel)
			texture:SetHeight(pixel)
	
			texture.Event.LeftClick = function()
				handler(r, g, b)
			end
			table.insert(frame.textures, texture)
		end
	end
	frame.ResizePixel = function(self, pixel)
		pixel = pixel or DEFAULT_PIXEL
		count, x, y = 0, 0, 0
		draw(pixel)
	end
	frame:ResizePixel(pixel)
	return frame
end

local ctx = UI.CreateContext("ColorChooser")
local frame
table.insert(Command.Slash.Register("colorchooser"), {function (commands)	

	local show = commands:match("^show")
	local resize = commands:match("^resize (%d+)")
	if show then
		frame = frame or UI.CreateFrame("Frame", "ColorChooser", ctx)
		frame:SetPoint("CENTER", UIParent, "CENTER")
		frame:SetVisible(true)
		local pixel = 5
		local handler = function(r, g, b)
			print(string.format("%02x%02x%02x", r*256, g*256, b*256))
		end
		local cc = Library.ColorChooser.CreateWidget(frame, handler, pixel, true)
	elseif resize then
		if frame then 
			frame:SetVisible(false)
			frame:ResizePixel(tonumber(resize))
		end
	end

end, "ColorChooser", "Slash"})

