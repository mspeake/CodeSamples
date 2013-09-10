--creates a new tracker from a configuration table, places it at the given index
function CreateTracker(config, ind)
	ind = ind or nil;
	local TrackerID;
	
	if ind == nil then
		local TrackerNum = TableSize(Trackers) + 1;
		TrackerID = "Tracker_" .. tostring(TrackerNum);
	else
		TrackerID = ind;
	end
	local Tracker = UI.CreateFrame("Frame", TrackerID, context);
	
	Tracker.ID = TrackerID;
	Tracker.Config = config;
	
	Tracker:SetWidth(config.TrackerSize);
	Tracker:SetHeight(config.TrackerSize);
	
	if Tracker.Config.ShowTimer == nil then
		Tracker.Config.ShowTimer = false;
	end
	
	if Tracker.Config.ShowStacks == nil then
		Tracker.Config.ShowStacks = false;
	end
	
	if Tracker.Config.ShowPercent == nil then
		Tracker.Config.ShowPercent = false;
	end
	
	if Tracker.Config.ThresholdPercent == nil then
		Tracker.Config.ThresholdPercent = false;
	end
	
	if Tracker.Config.ThresholdValue == nil then
		Tracker.Config.ThresholdValue = 0;
	end
	
	if Tracker.Config.ThresholdOperator == nil then
		Tracker.Config.ThresholdOperator = ">=";
	end
	
	if Tracker.Config.TrackerPos == nil then
		Tracker.Config.TrackerPos = {
			x = 0,
			y = 0
		}
	elseif type(Tracker.Config.TrackerPos) ~= "table" then --in case of errors
		Tracker.Config.TrackerPos = {                             --resulting in position
			x = 0,                                                  --not being stored
			y = 0                                                   --as a table
		}
	end
	
	if Tracker.Config.StacksColor == nil then
		Tracker.Config.StacksColor = {
			r = 1,
			g = 1,
			b = 1,
			a = 1
		};
	end
	
	if Tracker.Config.TimerColor == nil then
		Tracker.Config.TimerColor = {
			r = 1,
			g = 1,
			b = 1,
			a = 1
		};
	end
	
	Tracker:SetPoint("TOPLEFT", UIParent, "TOPLEFT", Tracker.Config.TrackerPos.x, Tracker.Config.TrackerPos.y);
	
	Tracker.Icon = UI.CreateFrame("Texture", TrackerID .. "_Icon", Tracker);
	Tracker.Icon:SetPoint("TOPLEFT", Tracker, "TOPLEFT");
	Tracker.Icon:SetWidth(config.TrackerSize);
	Tracker.Icon:SetHeight(config.TrackerSize);
	Tracker.Icon:SetLayer(Tracker:GetLayer());
	Tracker.Icon:SetVisible(true);
	
	Tracker.Icon:SetAlpha(Tracker.Config.TrackerAlpha);
	
	if Tracker.Config.TimerPos == nil then  --if no position is stored, give it the
		Tracker.Config.TimerPos = {           --position of TOPLEFT to Tracker BOTTOMLEFT
			x = Tracker:GetLeft(),
			y = Tracker:GetBottom()
		}
	elseif type(Tracker.Config.TimerPos) ~= "table" then --in case of errors
		Tracker.Config.TimerPos = {                             --resulting in position
			x = 0,                                                --not being stored
			y = 0                                                 --as a table
		}
	end
	
	local TimerColor = Tracker.Config.TimerColor;
	Tracker.Timer = UI.CreateFrame("Text", TrackerID .. "_Timer", Tracker);
	Tracker.Timer:SetPoint("TOPLEFT", UIParent, "TOPLEFT", Tracker.Config.TimerPos.x, Tracker.Config.TimerPos.y);
	Tracker.Timer:SetWidth(config.TimerSize * 4); --quadruple width supports under 10 hrs
	Tracker.Timer:SetHeight(config.TimerSize);
	Tracker.Timer:SetFontSize(config.TimerSize);
	Tracker.Timer:SetFontColor(TimerColor.r, TimerColor.g, TimerColor.b, TimerColor.a);
	Tracker.Timer:SetAlpha(TimerColor.a);
	Tracker.Timer:SetLayer(Tracker:GetLayer() + 1);
	Tracker.Timer:SetVisible(true);
	
	if Tracker.Config.StacksPos == nil then
		Tracker.Config.StacksPos = {
			x = Tracker:GetRight(),
			y = Tracker:GetTop()
		}
	elseif type(Tracker.Config.StacksPos) ~= "table" then --in case of errors
		Tracker.Config.StacksPos = {                             --resulting in position
			x = 0,                                                 --not being stored
			y = 0                                                  --as a table
		}
	end
	
	local StackColor = Tracker.Config.StacksColor;
	Tracker.Stacks = UI.CreateFrame("Text", TrackerID .. "_Text", Tracker);
	Tracker.Stacks:SetPoint("TOPLEFT", UIParent, "TOPLEFT", Tracker.Config.StacksPos.x, Tracker.Config.StacksPos.y);
	Tracker.Stacks:SetWidth(config.StacksSize);
	Tracker.Stacks:SetHeight(config.StacksSize);
	Tracker.Stacks:SetFontSize(config.StacksSize);
	Tracker.Stacks:SetFontColor(StackColor.r, StackColor.g, StackColor.b, StackColor.a);
	Tracker.Stacks:SetAlpha(StackColor.a);
	Tracker.Stacks:SetText("");
	Tracker.Stacks:SetLayer(Tracker:GetLayer() + 1);
	Tracker.Stacks:SetVisible(true);
	
	if Tracker.Config.ShowTimerBG == true then
			Tracker.Timer:SetBackgroundColor(0,0,0,TimerColor.a);
		else
			Tracker.Timer:SetBackgroundColor(0,0,0,0);
		end
		
		if Tracker.Config.ShowStacksBG == true then
			Tracker.Stacks:SetBackgroundColor(0,0,0,StackColor.a);
		else
			Tracker.Stacks:SetBackgroundColor(0,0,0,0);
		end
	
	--sets the visibility of the timer, compliant with customizable parameters
	function Tracker:SetTimerVisibility(show)
		if self.Config.ShowTimer == true then
			if show == true then
				self.Timer:SetVisible(true);
			else
				self.Timer:SetVisible(false);
			end
		else
			self.Timer:SetVisible(false);
		end
	end
	
	--sets the visibility of stacks, compliant with customizable parameters
	function Tracker:SetStacksVisibility(show)
		if self.Config.ShowStacks == true then
			local StackCount = self.Stacks:GetText();
			if show == true then
				if StackCount ~= "" then
					self.Stacks:SetVisible(true);
				else
					self.Stacks:SetVisible(false);
				end
			else
				self.Stacks:SetVisible(false);
			end
		else
			self.Stacks:SetVisible(false);
		end
	end
	
	--this tracker's individual update function
	function Tracker:Update(time)
		if self.TestMode == true then
			return; --no need to update trackers that are in test mode
		elseif self.Continuous then
			self:SetTimerVisibility(false);
			return; --no need to update trackers that aren't bound by time
		elseif IsResource(self.Config.SpellType) then
			--self:UpdateResource(); --special update function for resource trackers
			return;
		elseif self.Completion then
			local Remaining = self.Completion - time;
			
			if Remaining < 0 then
				self:SetTimerVisibility(false);
				self:SetStacksVisibility(false);
				self:SetVisible(false);
			else
				self:UpdateTimer(Remaining);
			end
			
		end
	end
	
	--A specialized update function for resource trackers
	function Tracker:UpdateResource()
		local Valid = self:ValidateResource(); --due to the nature of resource, they
		if Valid == true then                  --need to be frequently validated
			self:AddMeResource();
			local Value, Percentage = ResourceDetail(self.Config.TrackUnit, self.Config.SpellType);
			self:UpdateResourceValue(Value, Percentage);
		else
			self:SetVisible(false);
		end
	end
	
	--Updates the width of the stacks textfield
	function Tracker:UpdateStacks(StackCount)
		self.Stacks:SetVisible(true);
		local WidthMod = GetValueWidthModifier(StackCount);
		self.Stacks:SetWidth(self.Config.StacksSize * WidthMod);
		self.Stacks:SetText(tostring(StackCount));
	end
	
	--Updates the width of the timer textfield
	function Tracker:UpdateTimer(Remaining)
		local WidthMod = GetTimerWidthModifier(Remaining);
		local FormattedText = GetTimerFormattedString(Remaining);
		
		self.Timer:SetWidth(self.Config.TimerSize * WidthMod);
		self.Timer:SetText(FormattedText);
	end
	
	--Updates the format of the displayed resource value
	function Tracker:UpdateResourceValue(Value, Percentage)
		local WidthMod = 1;
		local FormattedText = "";
		if self.Config.ShowPercent == true then
			WidthMod = GetPercentWidthModifier(Percentage);
			FormattedText = GetPercentageFormattedString(Percentage);
		else
			WidthMod = GetValueWidthModifier(Value);
			FormattedText = GetValueFormattedString(Value);
		end
		self.Timer:SetWidth(self.Config.TimerSize * WidthMod);
		self.Timer:SetText(FormattedText);
	end
	
	--reset all parameters so the user can see changes as they make them
	function Tracker:RefreshData()
		self:SetWidth(self.Config.TrackerSize);
		self:SetHeight(self.Config.TrackerSize);
		self:SetPoint("TOPLEFT", UIParent, "TOPLEFT", Tracker.Config.TrackerPos.x, Tracker.Config.TrackerPos.y);
		
		RefreshIconData(self);
		RefreshTimerData(self);
		RefreshStacksData(self);
		RefreshTestModeSettings(self);
	end
	
	--updates the location of the texture to display
	function Tracker:UpdateDisplayedTexture(Spell) --works with buffs and abilities
		local Texture = self.Config.Texture;
		
		--decide which texture to use, try the given spell first
		if Spell ~= nil then
			self.Icon:SetTexture("Rift", Spell.icon);
			Texture = {
				Location = "Rift",
				Name = Spell.icon
				};
		elseif Texture ~= nil then --try the alternate image if that fails
			self.Icon:SetTexture(Texture.Location, Texture.Name);
			if Texture.Location == AddonID then --update displayed image if alternate
				Texture = {
					Location = AddonID,
					Name = AlternateTextures[self.Config.AltTex]
				};
			end
		end
		
		if Texture == nil then --use the alternate image if all else fails
			Texture = {
				Location = AddonID,
				Name = AlternateTextures[self.Config.AltTex]
			};
		end
		
		self.Config.Texture = Texture;
		
		if self.Config.UseSpellIcon == true then
			self.Icon:SetTexture(Texture.Location, Texture.Name);
		else
			self.Icon:SetTexture(AddonID, AlternateTextures[self.Config.AltTex]);
		end
	end
	
	--display this tracker
	function Tracker:AddMe(Buff)
		self:UpdateDisplayedTexture(Buff);
		
		if self.TestMode == true then --test mode settings
			if self.Config.ShowStacks then
				self.Stacks:SetText("9");
			end
			if self.Config.ShowTimer then
				local FormattedText = GetTestModeTimerFormattedString(self);
				self.Timer:SetText(FormattedText);
			end
			
		elseif self.Config.Invert == true then --inverted tracker settings
			self.Continuous = true;
			if Buff ~= nil then
				self.SpellID = Buff.id;
			end
			
		else --settings for everything else
			if Buff.stack then
				Tracker:UpdateStacks(Buff.stack);
			else
				self.Stacks:SetText("");
			end
			
			if Buff.begin ~= nil then
				self.Continuous = false;
				self.Completion = Buff.begin + Buff.duration;
				self.Duration = Buff.duration;
			else
				self.Continuous = true;
			end
			self.SpellID = Buff.id;
		end
		
		self:SetVisible(true);
		self.Icon:SetVisible(true);
		self:SetTimerVisibility(true);
		self:SetStacksVisibility(true);
	end
	
	--display this cooldown tracker
	function Tracker:AddMeCD(Abil)
		self:UpdateDisplayedTexture(Abil);
		local time = Inspect.Time.Frame();
		if self.Config.Invert == true then --only show when not on cooldown
			self.Continuous = true;
			self:SetTimerVisibility(false); --no need for timer when inverted
			self.SpellID = Abil.id;
			self.Config.SpellID = Abil.id;
		else --only show when on cooldown
			self.Continuous = false;
			if Abil.currentCooldownRemaining ~= nil then
				self.Completion = time + Abil.currentCooldownRemaining;
				self.Duration = Abil.currentCooldownRemaining;
			else
				self.Completion = time;
				self.Duration = 0;
			end
			self.SpellID = Abil.id;
			self:SetTimerVisibility(true);
		end
		
		self:SetVisible(true);
		self.Icon:SetVisible(true);
		self:SetStacksVisibility(false);
	end
	
	--display this resource tracker
	function Tracker:AddMeResource()
		self:UpdateDisplayedTexture();
		self:SetVisible(true);
		self.Icon:SetVisible(true);
		self:SetTimerVisibility(true);
		self:SetStacksVisibility(true);
	end
	
	--toggle test mode for this tracker
	function Tracker:SetTestMode(Test)
		if self.TestMode == Test then
			return;
		end
		
		if Test == true then
			self.Timer:SetWidth(self.Config.TimerSize * 4);
			self.TestMode = true;
			self:AddMe(nil);
		else
			self.TestMode = false;
			self:SetVisible(false);
			self.Stacks:SetText("");
			self.Timer:SetText("");
		end
	end
	
	--should this tracker be applied? if so, also return buff
	function Tracker:ValidateMe() 
		if self.Config.OnlyInCombat == true then
			if CharacterUnit.combat ~= true then --dont show in combat trackers while
				return false, nil;                 --out of combat
			end
		end
		
		--dont show trackers not marked to be shown in this role
		if self.Config.ShowInSpec[PlayerRole] ~= true then 
			return false, nil;
		end
		
		local Unit = FindUnit(self.Config.TrackUnit);
		
		if Unit ~= nil then
			if (self.Config.SpellType == "Buff") or (self.Config.SpellType == "Debuff") then
				local Buffs = Inspect.Buff.List(Unit);
				local Found = false;
				local FoundSpell = nil;
				
				if Buffs ~= nil then --search the unit's buffs for the tracked buff
					local BuffDetails = Inspect.Buff.Detail(Unit, Buffs);
					local BuffsInd = next(BuffDetails);
					
					while BuffsInd ~= nil do
						local Buff = BuffDetails[BuffsInd];
						--compare buff/debuff type of this buff to that of this tracker
						if (Buff.debuff ~= nil and self.Config.SpellType == "Debuff") or
						   (Buff.debuff == nil and self.Config.SpellType == "Buff") then
							if Buff.name == self.Config.SpellName then --buff/debuff found
								--if cast by player or MySpells not checked, apply this tracker
								if (self.Config.MySpells == false) or (Buff.caster == PlayerID) then
									Found = true;
									FoundSpell = Buff;
									break;
								end
							end
						end
						
						BuffsInd = next(BuffDetails, BuffsInd);
					end
				end
				
				if Found == true then --spell found, return true if not inverted
					if self.Config.Invert == false then
						return true, FoundSpell;
					else
						return false, FoundSpell; --false, nil
					end
				else --spell not found, return true if inverted
					if self.Config.Invert == true then
						return true, FoundSpell;
					else
						return false, FoundSpell; --false, nil
					end
				end
			end
		end
		
		return false, nil;
	end
	
	--same as ValidateMe, but for cooldown trackers
	function Tracker:ValidateCD() 
		if (self.Config.OnlyInCombat == true) and (CharacterUnit.combat ~= true) then
			return false, nil; --don't show combat only trackers if not in combat
		end
		
		if self.Config.ShowInSpec[PlayerRole] ~= true then 
			return false, nil; --dont show trackers not marked to be shown in this role
		end
		
		if self.Config.SpellID == nil then
			return false, nil; --invalid spell id, don't show
		end
		
		local Abil = Inspect.Ability.New.Detail(self.Config.SpellID);
		if Abil.name == self.Config.SpellName then
			if (Abil.currentCooldownRemaining ~= nil) and (Abil.currentCooldownRemaining > 1.5) then --this spell is on cooldown
				if self.Config.Invert == true then
					return false, nil; --dont show if the effect is inverted
				else
					return true, Abil;
				end
				
			else  --this spell is not on cooldown
				if self.Config.Invert == true then
					return true, Abil; --show if the effect is inverted
				else
					return false, nil;
				end
			end
		end
		
	end
	
	--same as ValidateMe, but for resource trackers
	function Tracker:ValidateResource()
		if (self.Config.OnlyInCombat == true) and (CharacterUnit.combat ~= true) then
			return false;
		end
		
		if FindUnit(self.Config.TrackUnit) == nil then
			return false;
		end
		
		local Value, Percentage = ResourceDetail(self.Config.TrackUnit, self.Config.SpellType);
		
		if self.Config.ThresholdPercent == true then
			Value = Percentage; --use the percentage instead of the value if the threshold uses percentage
		end
		
		if EvaluateThreshold(Value, self.Config.ThresholdValue, self.Config.ThresholdOperator) == true then
			if self.Config.Invert == true then
				return false; --threshold passed, but inverted. do not show
			end
		else
			if self.Config.Invert == false then
				return false; --threshold failed, but not inverted. do not show
			end
		end
		
		return true;
	end
	
	--------------------Draggable Tracker-----------------------------------------
	Tracker:EventAttach(Event.UI.Input.Mouse.Left.Down, function(self, h)
		if self.TestMode ~= true then
			return
		end
		self.MouseDown = true
		local mouseData = Inspect.Mouse()
		self.sx = mouseData.x - Tracker:GetLeft()
		self.sy = mouseData.y - Tracker:GetTop()
	end, "Event.UI.Input.Mouse.Left.Down")
	
	Tracker:EventAttach(Event.UI.Input.Mouse.Cursor.Move, function(self, h)
		if self.TestMode ~= true then
			return
		end
		if self.MouseDown then
			local nx, ny
			local mouseData = Inspect.Mouse()
			nx = mouseData.x - self.sx
			ny = mouseData.y - self.sy
			Tracker:SetPoint("TOPLEFT", UIParent, "TOPLEFT", nx,ny)
		end
	end, "Event.UI.Input.Mouse.Cursor.Move")
	
	Tracker:EventAttach(Event.UI.Input.Mouse.Left.Up, function(self, h)
		if self.TestMode ~= true then
			return
		end
		if self.MouseDown then
			self.MouseDown = false
		end
		Tracker.Config.TrackerPos.x = math.floor(Tracker:GetLeft());
		Tracker.Config.TrackerPos.y = math.floor(Tracker:GetTop());
		if FrameEditWindow ~= nil then
			FrameEditWindow:RefreshPanelSliders(1); --"TrackerPanel"
		end
	end, "Event.UI.Input.Mouse.Left.Up")
	------------------------------------------------------------------------------
		
	----------------------Draggable Timer-----------------------------------------
	Tracker.Timer:EventAttach(Event.UI.Input.Mouse.Left.Down, function(self, h)
		if self:GetParent().TestMode ~= true then
			return
		end
		self.MouseDown = true
		local mouseData = Inspect.Mouse()
		self.sx = mouseData.x - Tracker.Timer:GetLeft()
		self.sy = mouseData.y - Tracker.Timer:GetTop()
	end, "Event.UI.Input.Mouse.Left.Down")
	
	Tracker.Timer:EventAttach(Event.UI.Input.Mouse.Cursor.Move, function(self, h)
		if self:GetParent().TestMode ~= true then
			return
		end
		if self.MouseDown then
			local nx, ny
			local mouseData = Inspect.Mouse()
			nx = mouseData.x - self.sx
			ny = mouseData.y - self.sy
			Tracker.Timer:SetPoint("TOPLEFT", UIParent, "TOPLEFT", nx,ny)
		end
	end, "Event.UI.Input.Mouse.Cursor.Move")
	
	Tracker.Timer:EventAttach(Event.UI.Input.Mouse.Left.Up, function(self, h)
		if self:GetParent().TestMode ~= true then
			return
		end
		if self.MouseDown then
			self.MouseDown = false
		end
		Tracker.Config.TimerPos.x = math.floor(Tracker.Timer:GetLeft());
		Tracker.Config.TimerPos.y = math.floor(Tracker.Timer:GetTop());
		if FrameEditWindow ~= nil then
			FrameEditWindow:RefreshPanelSliders(2); --"TimerPanel"
		end
	end, "Event.UI.Input.Mouse.Left.Up")
	------------------------------------------------------------------------------
	
	----------------------Draggable Stacks----------------------------------------
	Tracker.Stacks:EventAttach(Event.UI.Input.Mouse.Left.Down, function(self, h)
		if self:GetParent().TestMode ~= true then
			return
		end
		self.MouseDown = true
		local mouseData = Inspect.Mouse()
		self.sx = mouseData.x - Tracker.Stacks:GetLeft()
		self.sy = mouseData.y - Tracker.Stacks:GetTop()
	end, "Event.UI.Input.Mouse.Left.Down")
	
	Tracker.Stacks:EventAttach(Event.UI.Input.Mouse.Cursor.Move, function(self, h)
		if self:GetParent().TestMode ~= true then
			return
		end
		if self.MouseDown then
			local nx, ny
			local mouseData = Inspect.Mouse()
			nx = mouseData.x - self.sx
			ny = mouseData.y - self.sy
			Tracker.Stacks:SetPoint("TOPLEFT", UIParent, "TOPLEFT", nx,ny)
		end
	end, "Event.UI.Input.Mouse.Cursor.Move")
	
	Tracker.Stacks:EventAttach(Event.UI.Input.Mouse.Left.Up, function(self, h)
		if self:GetParent().TestMode ~= true then
			return
		end
		if self.MouseDown then
			self.MouseDown = false
		end
		Tracker.Config.StacksPos.x = math.floor(Tracker.Stacks:GetLeft());
		Tracker.Config.StacksPos.y = math.floor(Tracker.Stacks:GetTop());
		if FrameEditWindow ~= nil then
			FrameEditWindow:RefreshPanelSliders(3); --"StacksPanel"
		end
	end, "Event.UI.Input.Mouse.Left.Up")
	------------------------------------------------------------------------------
	
	Tracker:SetVisible(false);
	return Tracker;
end

--sets all trackers to be hidden
function HideAllTrackers()
	local TrackerInd = next(Trackers);
	
	while TrackerInd ~= nil do
		local Tracker = Trackers[TrackerInd];
		Tracker:SetVisible(false);
		TrackerInd = next(Trackers, TrackerInd);
	end
end

--applies inverted trackers that should be visible
function ApplyInvertedTrackers()
	local TrackerInd = next(Trackers_Role);
	
	while TrackerInd ~= nil do
		local Tracker = Trackers[TrackerInd];
		
		if Tracker.Config.Invert == true then
			if Tracker.Config.SpellType == "Cooldown" then
				local Valid, Abil = Tracker:ValidateCD(); --special cooldown validate
				if Valid == true then
					Tracker:AddMeCD(Abil);
				end
				
			elseif IsResource(Tracker.Config.SpellType) then
				local Valid = Tracker:ValidateResource(); --special resource validate
				if Valid == true then
					Tracker:AddMeResource();
				end
			else
			
				local Valid, Buff = Tracker:ValidateMe(); --validate for everything else
				if Valid == true then
					Tracker:AddMe(Buff);
				end
			end
		end
		
		TrackerInd = next(Trackers_Role, TrackerInd);
	end
end

--applies all valid resource trackers
function ApplyResourceTrackers()
	local TrackerInd = next(Trackers_Role);
	
	while TrackerInd ~= nil do
		local Tracker = Trackers[TrackerInd];
		
		if IsResource(Tracker.Config.SpellType) then
			local Valid = Tracker:ValidateResource();
			if Valid == true then
				Tracker:AddMeResource();
			end
		end
		
		TrackerInd = next(Trackers_Role, TrackerInd);
	end
end

--hides all "only show in combat" trackers when out of combat
function HideCombatTrackers()
	local TrackerInd = next(Trackers_Role);
	
	while TrackerInd ~= nil do
		local Tracker = Trackers[TrackerInd];
		
		if Tracker.TestMode ~= true then --ignore trackers in test mode
			if Tracker.Config.OnlyInCombat == true then
				Tracker:SetVisible(false);
			end
		end
		
		TrackerInd = next(Trackers_Role, TrackerInd);
	end
end

--shows all "only show in combat" trackers that should be visible
function ApplyCombatTrackers()
	local TrackerInd = next(Trackers_Role);
	
	while TrackerInd ~= nil do
		local Tracker = Trackers[TrackerInd];
		if Tracker.Config.OnlyInCombat == true then --skip non-combat trackers
		
			if Tracker.Config.SpellType == "Cooldown" then --special cooldown validate
				local Valid, Abil = Tracker:ValidateCD();
				if Valid == true then
					Tracker:AddMeCD(Abil);
				end
				
			elseif IsResource(Tracker.Config.SpellType) then --special resource vaidate
				local Valid = Tracker:ValidateResource();
				if Valid == true then
					Tracker:AddMeResource();
				end
				
			else --normal validate for everything else
				local Valid, Buff = Tracker:ValidateMe();
				if Valid == true then
					Tracker:AddMe(Buff);
				end
			end
			
		end
		TrackerInd = next(Trackers_Role, TrackerInd);
	end
	
end

--called when the player's target or focus changes, shows all trackers for target
--or focus that should be visible
function ShowNonPlayerTrackers(Show, Tracked)
	local UnitID = CurrentTarget;
	local TrackerList = Trackers_Target;
	if Tracked == "Focus" then
		UnitID = CurrentFocus;
		TrackerList = Trackers_Focus;
	elseif Tracked == "Pet" then
		UnitID = CurrentPet;
		TrackerList = Trackers_Pet;
	end
	
	local TrackerInd = next(TrackerList);
	while TrackerInd ~= ni do
		local Tracker = Trackers[TrackerInd];
		
		if Tracker.TestMode ~= true then --ignore trackers in test mode
			if Tracker.Config.SpellType ~= "Cooldown" then
				Tracker:SetVisible(Show);
			end
		end
		
		TrackerInd = next(TrackerList, TrackerInd);
	end
end

--refreshes all trackers that are tracking the provided unit
function RefreshTrackers(Tracked)
	local UnitID = PlayerID;
	local TrackerList = Trackers_Player;
	if Tracked == "Target" then
		UnitID = CurrentTarget;
		TrackerList = Trackers_Target;
	elseif Tracked == "Focus" then
		UnitID = CurrentFocus;
		TrackerList = Trackers_Focus;
	elseif Tracked == "Pet" then
		UnitID = CurrentPet;
		TrackerList = Trackers_Pet;
	end
	
	if CharacterUnit.combat ~= true then
		HideCombatTrackers();
	end
	
	local TrackerInd = next(TrackerList);
	while TrackerInd ~= nil do
		local Tracker = Trackers[TrackerInd];
		
		if Tracker.TestMode ~= true then --no need to refresh trackers in test mode
			if IsResource(Tracker.Config.SpellType) then --special resource validate
				local Valid = Tracker:ValidateResource();
				if Valid == true then
					Tracker:AddMeResource();
				end
				
			else --validate for everything else
				local Valid, Buff = Tracker:ValidateMe();
				if Valid == true then
					Tracker:AddMe(Buff);
				end
			end
		end
		TrackerInd = next(TrackerList, TrackerInd);
	end
end

--refreshes all trackers for the 4 units of interest
function RefreshAllTrackers()
	RefreshTrackers("Player");
	RefreshTrackers("Target");
	RefreshTrackers("Focus");
	RefreshTrackers("Pet");
end

--a list of cooldowns is stored to optimize cooldown tracker validation, this
--repopulates that list with all cooldown trackers
function RefreshCooldownList()
	CooldownList = {};
	local TrackerInd = next(Trackers_Role);
	
	while TrackerInd ~= nil do
		local Tracker = Trackers[TrackerInd];
		
		if Tracker.Config.SpellType == "Cooldown" then
			CooldownList[TrackerInd] = Tracker.Config.SpellName;
			--if we already have an ID, make sure its the right one
			if Tracker.Config.SpellID ~= nil then 
				local Ability = Inspect.Ability.New.Detail(Tracker.Config.SpellID);
				if Ability ~= nil then
					if (Ability.name == nil) or (Ability.name ~= Tracker.Config.SpellName) then
						--name of the tracked cooldown has changed, update the spell ID
						Tracker.Config.SpellID = FindSpellID(Tracker.Config.SpellName);
					end
				end
			else
				--no spellID, attempt to find it
				Tracker.Config.SpellID = FindSpellID(Tracker.Config.SpellName);
			end
		end
		
		TrackerInd = next(Trackers_Role, TrackerInd);
	end
end

--places all trackers into test mode
function TestAllTrackers()
	local TrackerInd = next(Trackers);
	
	while TrackerInd ~= nil do
		local Tracker = Trackers[TrackerInd];
		Tracker:SetTestMode(true);
		TrackerInd = next(Trackers, TrackerInd);
	end
end

--helper function for the add buff event, filters based on tracked unit so that
--any tracker not tracking the provided unit is skipped over
local function AddBuffHelper(handle, unit, buffs, tracked)
	local TrackerList = GetTrackerList(tracked);
	--[
	Trackers_Player;
	if tracked == "Target" then
		TrackerList = Trackers_Target; --these lists contain indices to the master tracker list
	elseif tracked == "Focus" then
		TrackerList = Trackers_Focus;
	elseif tracked == "Pet" then
		TrackerList = Trackers_Pet;
	end
	--]]
	
	local BuffDetails = Inspect.Buff.Detail(unit, buffs);
	local BuffInd = next(BuffDetails);
	
	while BuffInd ~= nil do
		local Buff = BuffDetails[BuffInd];
		
		local TrackerInd = next(TrackerList);
		
		while TrackerInd ~= nil do
			local Tracker = Trackers[TrackerInd];
			
			if Tracker.TestMode ~= true then --dont remove or add trackers in test mode
				if Buff.name == Tracker.Config.SpellName then --only validate if the names match
					local Valid, AddedBuff = Tracker:ValidateMe();
					
					if Valid == true then --add this tracker if valid, hide it if not
						Tracker:AddMe(AddedBuff);
					else
						Tracker:UpdateDisplayedTexture(Buff); --save the tex image if needed
						Tracker:SetVisible(false);
						if (Tracker.Config.Invert == true) and (AddedBuff ~= nil) then
							Tracker.SpellID = AddedBuff.id; --need to save the ID for inverted effects
						end
					end
				end
			end
			TrackerInd = next(TrackerList, TrackerInd);
		end
		
		BuffInd = next(BuffDetails, BuffInd);
	end
end

--helper function for the change buff event, filters based on tracked unit so
--that any tracker not tracking the provided unit is skipped over
local function ChangeBuffHelper(handle, unit, buffs, tracked)
	local TrackerList = Trackers_Player; --determine which list to search
	if tracked == "Target" then
		TrackerList = Trackers_Target; --these lists contain indices to the master tracker list
	elseif tracked == "Focus" then
		TrackerList = Trackers_Focus;
	elseif tracked == "Pet" then
		TrackerList = Trackers_Pet;
	end
	
	local BuffDetails = Inspect.Buff.Detail(unit, buffs);
	local BuffInd = next(BuffDetails);
	
	while BuffInd ~= nil do
		local Buff = BuffDetails[BuffInd];
		local TrackerInd = next(TrackerList);
		
		while TrackerInd ~= nil do
			local Tracker = Trackers[TrackerInd];
			--ignore trackers in test mode
			if Tracker.TestMode ~= true then
				if Buff.name == Tracker.Config.SpellName then
					--basicallly this is a fringe case for decreasing stacks
					if Buff.stack then
						Tracker:UpdateStacks(Buff.stack);
					else
						Tracker.Stacks:SetText("");
						Tracker.Stacks:SetVisible(false);
					end
				end
			end
			
			TrackerInd = next(TrackerList, TrackerInd);
		end
		
		BuffInd = next(BuffDetails, BuffInd);
	end
	
end

--helper function for the remove buff event, filters based on tracked unit so
--that any tracker not tracking the provided unit is skipped over
local function RemoveBuffHelper(handle, unit, buffs, tracked)
	local TrackerList = Trackers_Player; --determine which list to search
	if tracked == "Target" then
		TrackerList = Trackers_Target; --these lists contain indices to the master tracker list
	elseif tracked == "Focus" then
		TrackerList = Trackers_Focus;
	elseif tracked == "Pet" then
		TrackerList = Trackers_Pet;
	end
	
	local BuffID = next(buffs);
	
	while BuffID ~= nil do --usually does 1 call per buff removed, but just in case
		local TrackerInd = next(TrackerList);
		
		while TrackerInd ~= nil do
			local Tracker = Trackers[TrackerInd];
			if Tracker.TestMode ~= true then --dont remove trackers in test mode
				if BuffID == Tracker.SpellID then --SpellID matches, show if inverted
					if Tracker.Config.Invert == true then
						Tracker:SetVisible(true);
					else
						Tracker.Stacks:SetText(""); --hide if not inverted
						Tracker:SetVisible(false);
						local time = Inspect.Time.Frame();
						Tracker.Completion = time;
					end
				end
			end
			
			TrackerInd = next(TrackerList, TrackerInd);
		end
		
		BuffID = next(buffs, BuffID);
	end
end

--Helper function for the Health Change event handler
local function HealthChangeHelper(Tracked, NewVal)
	MajorUnits[Tracked].Health = NewVal;
	--Now need to send a message to all trackers tracking health on the given unit
	--saying that they need to re-validate
	RefreshResourceTrackers(Tracked, "Health");
	--print(MajorUnits[Tracked].Health);
end

--Helper function for the Max Health Change event handler
local function MaxHealthChangeHelper(Tracked, NewVal)
	MajorUnits[Tracked].MaxHealth = NewVal;
	RefreshResourceTrackers(Tracked, "Health");
	--print(MajorUnits[Tracked].Health);
end

--Refreshes resource trackers of the given type on the given unit
function RefreshResourceTrackers(Tracked, Type)
	local TrackerInd = next(Trackers_Resource);
	
	while TrackerInd ~= nil do
		local Tracker = Trackers[TrackerInd];
		if (Tracker.Config.TrackUnit == Tracked) and (Tracker.Config.SpellType == Type) then
			Tracker:UpdateResource();
		end
		TrackerInd = next(Trackers_Resource, TrackerInd);
	end
end


--Handler function for the add buff event
local function AddBuff(handle, unit, buffs)
	if unit == PlayerID then --DO NOT use elseif for these, player could be targetting themselves or their focus
		AddBuffHelper(handle, unit, buffs, "Self"); --buffs/debuffs added to the player
	end
	if unit == CurrentTarget then
		AddBuffHelper(handle, unit, buffs, "Target"); --added to the players target
	end
	if unit == CurrentFocus then
		AddBuffHelper(handle, unit, buffs, "Focus"); --added to the players focus target
	end
	if unit == CurrentPet then
		AddBuffHelper(handle, unit, buffs, "Pet"); --added to the players pet
	end
end

--Handler function for the change buff event
local function ChangeBuff(handle, unit, buffs)
	if unit == PlayerID then
		ChangeBuffHelper(handle, unit, buffs, "Self"); --buffs/debuffs changed for the player
	end
	if unit == CurrentTarget then
		ChangeBuffHelper(handle, unit, buffs, "Target"); --changed for the players target
	end
	if unit == CurrentFocus then
		ChangeBuffHelper(handle, unit, buffs, "Focus"); --changed for the players focus target
	end
	if unit == CurrentPet then
		ChangeBuffHelper(handle, unit, buffs, "Pet"); --changed for the players pet
	end
end

--Handler function for the remove buff event
local function RemoveBuff(handle, unit, buffs)
	if unit == PlayerID then
		RemoveBuffHelper(handle, unit, buffs, "Self"); --buffs/debuffs removed from the player
	end
	if unit == CurrentTarget then
		RemoveBuffHelper(handle, unit, buffs, "Target"); --removed from the players target
	end
	if unit == CurrentFocus then
		RemoveBuffHelper(handle, unit, buffs, "Focus"); --removed from the players focus target
	end
	if unit == CurrentPet then
		RemoveBuffHelper(handle, unit, buffs, "Pet"); --removed from the players pet
	end
end

--Handler function for the cooldown begin event
local function CooldownBegin(handle, cooldowns)
	local CooldownInd = next(CooldownList);
	
	while CooldownInd ~= nil do
		local Tracker = Trackers[CooldownInd];
			
		if (Tracker.Config.SpellID ~= nil) and (cooldowns[Tracker.Config.SpellID] ~= nil) then
			if Tracker.TestMode ~= true then --dont remove or add trackers in test mode
				local Valid, Abil = Tracker:ValidateCD();
				if Valid == true then
					Tracker:AddMeCD(Abil);
				else
					Tracker:SetVisible(false);
				end
			end
		end
			
		CooldownInd = next(CooldownList, CooldownInd);
	end
	
end

--Handler function for the cooldown event
local function CooldownEnd(handle, cooldowns)
	local CooldownInd = next(CooldownList);
	
	while CooldownInd ~= nil do
		local Tracker = Trackers[CooldownInd];
		
		if (Tracker.Config.SpellID ~= nil) and (cooldowns[Tracker.Config.SpellID] ~= nil) then
			if Tracker.TestMode ~= true then --dont remove or add trackers in test mode
				local Valid, Abil = Tracker:ValidateCD();
				if Valid == true then
					if Tracker.Config.Invert == true then
						Tracker:AddMeCD(Abil);
					else
						Tracker:SetVisible(false);
					end
				else
					Tracker:SetVisible(false);
				end
			end
		end
		
		CooldownInd = next(CooldownList, CooldownInd);
	end
	
end

--Handler function for the Health Change event
local function HealthChange(handle, units)
	local Unit = next(units);
	
	while Unit ~= nil do
		if Unit == PlayerID then
			HealthChangeHelper("Self", units[Unit])
		end
		
		Unit = next(units, Unit);
	end
end

--Handler function for the Max Health Change event
local function MaxHealthChange(handle, units)
	local Unit = next(units);
	while Unit ~= nil do
		if Unit == PlayerID then
			MaxHealthChangeHelper("Self", units[Unit])
		end
		
		Unit = next(units, Unit);
	end
end

----------------------global update function------------------------------------
local function Update(handle)
	local time = Inspect.Time.Frame();
	
	UpdateRole();  --update the status of the 4 units of interest
	UpdateCombat();
	UpdateTarget();
	UpdateFocus();
	UpdatePet();
	
	local TrackerInd = next(Trackers_Role);
	while TrackerInd ~= nil do
		local Tracker = Trackers[TrackerInd];
		if Tracker ~= nil then  --failsafe incase of attempts to access nil trackers
			Tracker:Update(time); --update all trackers for the current role
		end
		TrackerInd = next(Trackers_Role, TrackerInd);
	end
	
	RefreshMainGui();
	
	if FrameEditWindow ~= nil then
		FrameEditWindow:Update();
	end
	
end
--------------------------------------------------------------------------------

--saves the Config data from each tracker between sessions
local function SaveVariables(handle)
	if handle == AddonID then
		local SaveData = {};
		local TrackerInd = next(Trackers);
		
		while TrackerInd ~= nil do
			local Tracker = Trackers[TrackerInd];
			SaveData[TrackerInd] = Tracker.Config;
			TrackerInd = next(Trackers, TrackerInd);
		end
		MyTrackers = SaveData;
	end
end

Command.Event.Attach(Event.Buff.Add, AddBuff, "add");
Command.Event.Attach(Event.Buff.Change, ChangeBuff, "remove");
Command.Event.Attach(Event.Buff.Remove, RemoveBuff, "remove");
Command.Event.Attach(Event.System.Update.Begin, Update, "refresh");
Command.Event.Attach(Event.Ability.New.Cooldown.Begin, CooldownBegin, "begin cooldown");
Command.Event.Attach(Event.Ability.New.Cooldown.End, CooldownEnd, "end cooldown");
Command.Event.Attach(Event.Unit.Detail.Health, HealthChange, "health change");
Command.Event.Attach(Event.Unit.Detail.HealthMax, MaxHealthChange, "max health change");
table.insert(Event.Addon.SavedVariables.Save.Begin, {SaveVariables, AddonID, "Save Variables"});