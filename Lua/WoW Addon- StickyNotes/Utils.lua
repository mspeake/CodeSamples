UpdateInterval = 1.0;

--Frame Levels-----------
TitleBarFrameLevel = 3;
BaseFrameLevel = TitleBarFrameLevel;
TextfieldFrameLevel = BaseFrameLevel + 1;
WidgetFrameLevel = TitleBarFrameLevel + 2;
-------------------------
function CopyTable(orig)
	local orig_type = type(orig);
	local copy;
	
	if orig_type == 'table' then
		copy = {};
		
		for orig_key, orig_value in next, orig, nil do
			copy[CopyTable(orig_key)] = CopyTable(orig_value);
		end
		
		setmetatable(copy, CopyTable(getmetatable(orig)));
	else -- number, string, boolean, etc
		copy = orig;
	end
	
	return copy;
end

function TableDeepCopy(obj, seen)
  -- Handle non-tables and previously-seen tables.
  if type(obj) ~= 'table' then return obj end
  if seen and seen[obj] then return seen[obj] end
 
  -- New table; mark it as seen an copy recursively.
  local s = seen or {}
  local res = setmetatable({}, getmetatable(obj))
  s[obj] = res
  for k, v in pairs(obj) do res[TableDeepCopy(k, s)] = TableDeepCopy(v, s) end
  return res
end

function ClearTable(mytable)
	local table_type = type(mytable);
	
	if table_type == 'table' then
		for k, v in pairs(mytable) do
			mytable[k] = nil
		end
	else
		mytable = nil;
	end
end

function TableSize(t)
	local count = 0;
	for _ in pairs(t) do 
		count = count + 1;
	end
	return count
end

function UpdateNoteKeys(t)
	for k, v in pairs(t) do
		t[k].Key = k;
	end
end

function PrintTable(mytable)
	for k, v in pairs(mytable) do
		local v_type = type(v);
		
		if(v_type == 'table') then
			PrintTable(v);
		else
			print(tostring(k) .. " = " .. tostring(v));
		end
	end
end

function DragTitleBarStart(self, button)
	if(self.Parent.LockWidget.Locked == false) then
		self:StartMoving();
		self.Moving = true;
		self.LastPos.X = (self:GetLeft());
		self.LastPos.Y = (self:GetBottom());
	end
end

function DragTitleBarStop(self, button)
	if(self.Parent.LockWidget.Locked == false) then
		self:StopMovingOrSizing();
		self.Moving = false;
		self:ClearAllPoints();
		local x, y = self:GetLeft(), self:GetBottom();
		self:SetPoint("BOTTOMLEFT", UIParent, "BOTTOMLEFT", x, y);
		self.Position = {X = x, Y = y};
		CharacterSavedData[self.Parent.Key].Pos = {X = x, Y = y};
		AttachNoteComponents(self.Parent);
	end
end

function TextFieldOnUpdate(self, elapsed)
	self.TimeSinceLastUpdate = self.TimeSinceLastUpdate + elapsed;
	
	local InCombat = UnitAffectingCombat("player");
	local IsLocked = self.Parent.LockWidget.Locked;
	if((InCombat == true) or (IsLocked == true)) then
		self:ClearFocus();
		ToggleNoteInput(self.Parent, false);
	else
		ToggleNoteInput(self.Parent, true);
	end
	
	if(self.TimeSinceLastUpdate > UpdateInterval) then
		self.TimeSinceLastUpdate = 0;
	end
end

function MinimizerOnClick(self, button, down)
	self.Minimized = not self.Minimized;
	MinimizerApplyState(self);
	CharacterSavedData[self.Parent.Key].Minimized = self.Minimized;
end

function MinimizerApplyState(self)
	if(self.Minimized == false) then
		self.Parent.BaseFrame:Show();
		self:SetTextureExpanded();
	else
		self.Parent.BaseFrame:Hide();
		self:SetTextureCollapsed();
	end
	ResizersApplyVisibility(self.Parent);
end

function VisibilityApplyState(note)
	if(note.Visible == true) then
		ShowNote(note);
	else
		HideNote(note);
	end
end

function ChatDumperOnClick(self, button, down)
	ToggleDropDownMenu(1, nil, self.ChatDumper, UIParent, self:GetLeft(), self:GetBottom());
end

function OptionsOnClick(self, button, down)
	if(OptionsGui.Visible == true) then
		OptionsGui:HideMe();
	else
		OptionsGui:SetNote(self.Parent);
		OptionsGui:ShowMe();
	end
end

function CloseOnClick(self, button, down)
	self.Parent:Hide();
	self.Parent.Visible = false;
end

function NoteCloseOnClick(self, button, down)
	HideNote(self.Parent);
end

function HideNote(note)
	note.TitleBar:Hide();
	note.TitleBar.Title:Hide();
	note.BaseFrame:Hide();
	note.Minimizer:Hide();
	note.ChatWidget:Hide();
	note.OptionsWidget:Hide();
	note.CloseWidget:Hide();
	note.LockWidget:Hide();
	note.BaseFrame.LeftResizer:Hide();
	note.BaseFrame.RightResizer:Hide();
	note.Visible = false;
	if(note.Key ~= nil) then
		local Key = note.Key;
		CharacterSavedData[Key].Visible = false;
	end
end

function ShowNote(note)
	note.TitleBar:Show();
	note.TitleBar.Title:Show();
	note.BaseFrame:Show();
	note.Minimizer:Show();
	note.ChatWidget:Show();
	note.OptionsWidget:Show();
	note.CloseWidget:Show();
	note.LockWidget:Show();
	note.BaseFrame.LeftResizer:Show();
	note.BaseFrame.RightResizer:Show();
	MinimizerApplyState(note.Minimizer);
	note.Visible = true;
	if(note.Key ~= nil) then
		local Key = note.Key;
		CharacterSavedData[Key].Visible = true;
	end
end

function CreateOptionsGUI()
	local GuiFrame = CreateDraggableFrame("GuiFrame", UIParent);
	GuiFrame:SetClampedToScreen(true);
	
	GuiFrame:SetSize(300, 400);
	GuiFrame.Texture = GenerateTexture(nil, "BACKGROUND", {R=0, G=0, B=0, A=0.8}, GuiFrame);
	GuiFrame:SetFrameLevel(1);
	GuiFrame.CurrentNote = nil;
	GuiFrame.Visible = true;
	GuiFrame:SetPoint("CENTER");
	
	GuiFrame.TitleEditor = CreateTitleEditor(GuiFrame);
	GuiFrame.TitleColorEditor = CreateTitleColorEditor(GuiFrame);
	GuiFrame.FontSizeEditor = CreateFontSizeEditor(GuiFrame);
	GuiFrame.TextColorEditor = CreateTextColorEditor(GuiFrame);
	GuiFrame.NoteColorEditor = CreateNoteColorEditor(GuiFrame);
	
	GuiFrame.CloseWidget = CreateCloseWidget(GuiFrame);
	
	function GuiFrame:HideMe()
		self.Visible = false;
		self:Hide();
	end
	
	function GuiFrame:ShowMe()
		self.Visible = true;
		self:Show();
	end
	
	function GuiFrame:SetVisible(state)
		if(state == true) then
			self:ShowMe();
		else
			self:HideMe();
		end
	end
	
	function GuiFrame:SetNote(note)
		self.CurrentNote = note;
		PopulateGuiFields(self);
	end
	
	return GuiFrame;
end

function PopulateGuiFields(guiframe)
	if(guiframe.CurrentNote ~= nil) then
		local File, Height, Flags = guiframe.CurrentNote.TextField:GetFont();
		guiframe.FontSizeEditor:SetText(floor(Height + 0.5));
		
		local Title = guiframe.CurrentNote.TitleBar.Title:GetText();
		guiframe.TitleEditor:SetText(tostring(Title));
		
		local Color = guiframe.CurrentNote.TextField.Color;
		guiframe.TextColorEditor.Texture:SetTexture(Color.R, Color.G, Color.B, Color.A);
		
		Color = guiframe.CurrentNote.BaseFrame.Color;
		guiframe.NoteColorEditor.Texture:SetTexture(Color.R, Color.G, Color.B, Color.A);
		
		local R, G, B, A = guiframe.CurrentNote.TitleBar.Title:GetTextColor();
		guiframe.TitleColorEditor.Texture:SetTexture(R, G, B, A);
		
		Color = guiframe.CurrentNote.TitleBar.Color;
		guiframe.BarEditor.Texture:SetTexture(Color.R, Color.G, Color.B, Color.A);
	end
end

function CreateChatDumpCategories(info)
	info.text, info.checked = "Guild/Group Channels", false
	info.menuList, info.hasArrow = 0, true
	UIDropDownMenu_AddButton(info);
	
	info.text, info.checked = "Custom Channels", false
	info.menuList, info.hasArrow = 1, true
	UIDropDownMenu_AddButton(info);
	
	info.text, info.checked = "Public Channels", false
	info.menuList, info.hasArrow = 2, true
	UIDropDownMenu_AddButton(info);
end

function GetPublicChannelNames()
	local PublicNames = {};
	table.insert(PublicNames, "General - ");
	table.insert(PublicNames, "Trade - ");
	table.insert(PublicNames, "LocalDefense - ");
	table.insert(PublicNames, "LookingForGroup");
	return PublicNames;
end

function GetAllChannelNames(publicnames)
	local PublicChannels = {};
	local CustomChannels = {};
	
	for i = 1, 20 do
		id, name = GetChannelName(i);
		local Found = false;
		
		for k, v in pairs(publicnames) do
			local S, E = nil, nil;
			if(name ~= nil) then
				S, E = string.find(name, v);
			end
			
			if((S and E) ~= nil) then
				PublicChannels[id] = name;
				Found = true;
				break;
			end
		end
		
		if(Found == false) then
			CustomChannels[id] = name;
		end
	end
	
	return PublicChannels, CustomChannels;
end

function PrintNoteFormattedText(text, chattype, channel)
	local Lines = {}
    for w in string.gmatch(text, "[^\n]*[\n]*") do
		if(w ~= "") then
			table.insert(Lines, w);
		end
    end
	
	for k, v in pairs(Lines) do
		local Line = tostring(Lines[k]);
		SendChatMessage(Line, chattype, nil, channel);
	end
end

function GenerateTexture(name, layer, color, parent)
	local tex = parent:CreateTexture(name, layer);
	tex:SetTexture(color.R, color.G, color.B, color.A);
	tex:SetAllPoints(parent);
	return tex;
end

function UpdateFont(note, file, size, flags)
	note.TextField:SetFont(file, size, flags);
	local Key = note.Key;
	CharacterSavedData[Key].Font = tostring(file);
	CharacterSavedData[Key].FontSize = size;
	CharacterSavedData[Key].FontFlags = tostring(flags);
end

function UpdateTitle(note, title)
	local Key = note.Key;
	note.TitleBar.Title:SetText(tostring(title));
	CharacterSavedData[Key].Title = tostring(title);
	InterfaceMainPanel.Notes[Key].TextField:SetText(tostring(title));
end

function AddNewNote()
	local DefaultConfig = CopyTable(DefaultNoteConfig);
	local NewNote = CreateStickyNote(DefaultConfig);
	table.insert(AllStickyNotes, NewNote);
	UpdateNoteKeys(AllStickyNotes);
	SaveMyData();
	OptionsGui:SetNote(NewNote);
	OptionsGui:ShowMe();
	OptionsGui.TitleEditor:SetFocus();
	OptionsGui.TitleEditor:HighlightText();
end

function LockOnClick(self, button, down)
	if(self.Locked == true) then
		self.Locked = false;
		self.Parent.TitleBar:SetMovable(true);
		ShowWidgets(self.Parent);
		ToggleNoteInput(self.Parent, true);
		ShowScrollBar(self.Parent.BaseFrame.ScrollingFrame);
		self:SetTextureUnlocked();
	else
		self.Locked = true;
		self.Parent.TitleBar:SetMovable(false);
		HideWidgets(self.Parent);
		ToggleNoteInput(self.Parent, false);
		HideScrollBar(self.Parent.BaseFrame.ScrollingFrame);
		self:SetTextureLocked();
	end
	ResizersApplyVisibility(self.Parent);
end

function HideWidgets(note)
	note.BaseFrame.LeftResizer:Hide();
	note.BaseFrame.RightResizer:Hide();
	note.ChatWidget:Hide();
	note.OptionsWidget:Hide();
	note.CloseWidget:Hide();
end

function ShowWidgets(note)
	note.BaseFrame.LeftResizer:Show();
	note.BaseFrame.RightResizer:Show();
	note.ChatWidget:Show();
	note.OptionsWidget:Show();
	note.CloseWidget:Show();
end

function ShowAllNotes()
	for k,v in pairs(AllStickyNotes) do
		ShowNote(AllStickyNotes[k]);
	end
end

function EnableInput(frame)
	frame:EnableKeyboard(true);
	frame:EnableMouse(true);
end

function DisableInput(frame)
	frame:EnableKeyboard(false);
	frame:EnableMouse(false);
end

function ToggleNoteInput(note, flag)
	if(flag == true) then
		EnableInput(note.TextField);
		EnableInput(note.BaseFrame);
		EnableInput(note.TitleBar);
	else
		DisableInput(note.TextField);
		DisableInput(note.BaseFrame);
		DisableInput(note.TitleBar);
	end
end

function HideScrollBar(frame)
	local FrameName = frame:GetName();
	local ScrollBar = _G[FrameName .. "ScrollBar"];
	ScrollBar:Hide();
end

function ShowScrollBar(frame)
	local FrameName = frame:GetName();
	local ScrollBar = _G[FrameName .. "ScrollBar"];
	ScrollBar:Show();
end

function SetScrollPos(val)
	InterfaceMainPanel.ScrollingFrame:SetVerticalScroll(val);
end

function GetScrollPos()
	local ScrollPos = InterfaceMainPanel.ScrollingFrame:GetVerticalScroll();
	--print("scroll pos = " .. tostring(ScrollPos));
end

function GetScrollRange()
	local MaxScroll = InterfaceMainPanel.ScrollingFrame:GetVerticalScrollRange();
	--print("Max scroll range = " .. tostring(MaxScroll));
end

function GenerateGUID()
	return tostring(time()) .. tostring(math.random(1, 10000));
end

DefaultNoteConfig = {
	Title = "Sticky Note",
	Pos = {
		X = 500,
		Y = 500,
	},
	Size = {
		Width = 300,
		Height = 300,
	},
	Font = "Fonts\\FRIZQT__.TTF",
	FontSize = 12,
	FontFlags = "OUTLINE, MONOCHROME",
	NoteColor = {
		R = 0.3,
		G = 0.3,
		B = 0.3,
		A = 0.5,
	},
	TextColor = {
		R = 1.0,
		G = 1.0,
		B = 1.0,
		A = 1.0,
	},
	TitleColor = {
		R = 1.0,
		G = 1.0,
		B = 1.0,
		A = 1.0,
	},
	TitleBarColor = {
		R = 0.1,
		G = 0.1,
		B = 0.1,
		A = 0.5,
	},
	Text = "Write stuff here",
	Minimized = false,
	Visible = true,
};
