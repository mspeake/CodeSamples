local function AddBuffHelper(handle, unit, buffs, tracked)
	local BuffDetails = Inspect.Buff.Detail(unit, buffs);
	local BuffInd = next(BuffDetails);
	if BuffInd ~= nil then
		local Buff = BuffDetails[BuffInd];
		local TrackerInd = next(Trackers);
		local SpellType = "Buff";
		
		if (Buff ~= nil) and (Buff.debuff ~= nil) then
			SpellType = "Debuff";
		end
		
		while TrackerInd ~= nil do
			local Tracker = Trackers[TrackerInd];
			
			if Tracker.TestMode ~= true then --dont remove or add trackers in test mode
				if Tracker.Config.TrackUnit == tracked then
					if Buff.name == Tracker.Config.SpellName then
						local Valid, AddedBuff = Tracker:ValidateMe();
						
						if Tracker.Config.Invert == true then
							if Valid == true then
								Tracker:AddMe(AddedBuff);
							else
								Tracker:UpdateDisplayedTexture(Buff);
								Tracker:SetVisible(false);
							end
						else
							if Valid == true then
								Tracker:AddMe(AddedBuff);
							end
						end
					end
				end
			end
			TrackerInd = next(Trackers, TrackerInd);
		end
		
	end
end

--------------------------------------------------------------------------------

local function ChangeBuffHelper(handle, unit, buffs, tracked)
	local BuffDetails = Inspect.Buff.Detail(unit, buffs);
	local BuffInd = next(BuffDetails);
	if BuffInd ~= nil then
		local Buff = BuffDetails[BuffInd];
		local TrackerInd = next(Trackers);
		local SpellType = "Buff";
		
		if (Buff ~= nil) and (Buff.debuff) ~= nil then
			SpellType = "Debuff";
		end
		
		while TrackerInd ~= nil do
			local Tracker = Trackers[TrackerInd];
			
			if Tracker.Config.TrackUnit == tracked then
				if Buff.name == Tracker.Config.SpellName then
					--basicallly this is a fringe case for decreasing stacks
					if Buff.stack then
						Tracker.Stacks:SetVisible(true);
						
						local StackCount = Buff.stack;
						if StackCount >= 1000 then
							Tracker.Stacks:SetWidth(Tracker.Config.StacksSize * 2.7);
						elseif StackCount >= 100 then
							Tracker.Stacks:SetWidth(Tracker.Config.StacksSize * 2.0);
						elseif StackCount > 10 then
							Tracker.Stacks:SetWidth(Tracker.Config.StacksSize * 1.3);
						else
							Tracker.Stacks:SetWidth(Tracker.Config.StacksSize);
						end
						Tracker.Stacks:SetText(tostring(StackCount));
					else
						Tracker.Stacks:SetText("");
						Tracker.Stacks:SetVisible(false);
					end
				end
			end
			TrackerInd = next(Trackers, TrackerInd);
		end
		
	end
	
end

--------------------------------------------------------------------------------

local function RemoveBuffHelper(handle, unit, buffs, tracked)
	local RemoveID = PlayerID;
	
	if tracked == "Target" then
		RemoveID = Inspect.Unit.Lookup("player.target");
	elseif tracked == "Focus" then
		RemoveID = Inspect.Unit.Lookup("focus");
	end
	
	if unit == RemoveID then
		local BuffID = next(buffs);
		
		if BuffID ~= nil then
			local TrackerInd = next(Trackers);
			
			while TrackerInd ~= nil do
				local Tracker = Trackers[TrackerInd];
				if Tracker.TestMode ~= true then --dont remove trackers in test mode
					if BuffID == Tracker.SpellID then
						if Tracker.Config.Invert == true then
							Tracker:SetVisible(true);
						else
							Tracker.Stacks:SetText("");
							Tracker:SetVisible(false);
							local time = Inspect.Time.Frame();
							Tracker.Completion = time;
						end
					end
				end
				
				TrackerInd = next(Trackers, TrackerInd);
			end
			
		end
		
	end
	RefreshTrackers(tracked);
end