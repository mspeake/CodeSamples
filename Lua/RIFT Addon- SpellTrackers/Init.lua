context = UI.CreateContext("HUD");

function RefreshTrackersGlobal(Tracked)
	if RefreshTrackers ~= nil then
		RefreshTrackers(Tracked);
	end
end

--turns off test mode on all trackers
function StopAllTestModes()
	local TrackerInd = next(Trackers);
	
	while TrackerInd ~= nil do
		local Tracker = Trackers[TrackerInd];
		Tracker:SetTestMode(false);
		TrackerInd = next(Trackers, TrackerInd);
	end
end

local function LoadSavedVariables(handle)
	if handle == AddonID then
		if MyTrackers then
			local DataInd = next(MyTrackers);
			
			while DataInd ~= nil do
				local frame = CreateTracker(MyTrackers[DataInd], DataInd);
				Trackers[DataInd] = frame; 
				DataInd = next(MyTrackers, DataInd);
			end
			
			MainGui = nil;
			FrameEditWindow = nil;
			SelectedTrackFrame = nil;
			HelpWindow = nil;
			UpdateTrackerLists();
			ApplyInvertedTrackers();
			ApplyResourceTrackers();
			RefreshCooldownList();
		end
	end
end

table.insert(Event.Addon.SavedVariables.Load.End, {LoadSavedVariables, AddonID, "LoadSavedVariables"});