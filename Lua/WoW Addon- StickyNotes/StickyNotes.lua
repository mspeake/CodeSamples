-------------------Globals---------------------------------------
AddonName = "StickyNotes";
CharacterStickyNote = nil;
MainBackFrame = nil;
SavedVarsLoaded = false;
TitleBarHeight = 24;
ResizerSize = 16;
WidgetSize = 16;
WidgetPadding = 6;
ScrollbarWidth = 23;
MinimumNoteWidth = ((WidgetSize + (2 * WidgetPadding)) * 5) + 50;
OptionsGui = nil;
AllStickyNotes = {};
InterfaceMainPanel = nil;

function OnLoadFunction(self)
	StickyNotes:SetScript("OnEvent",
	function(self, event, arg1)
		if arg1 ~= AddonName then 
			return;
		end
		
		if event == "ADDON_LOADED" then
			CreateAllNotes();
			OptionsGui = CreateOptionsGUI();
			OptionsGui:HideMe();
			CreateInterfacePanel();
		elseif event == "PLAYER_LOGOUT" then
			SaveMyData();
		end
	end);
		
	StickyNotes:RegisterEvent("ADDON_LOADED");
	StickyNotes:RegisterEvent("PLAYER_LOGOUT");
end

---[[
function ResizeNote(note, w, h)
	local TitleWidth, TitleHeight = note.TitleBar:GetSize();
	note.TitleBar:SetSize(w, TitleHeight);
	note.BaseFrame:SetSize(w, h);
end
--]]

function CreateAllNotes()
	local DataType = type(CharacterSavedData);
	
	if(DataType ~= "table") then
		return;
	end
	
	for k, v in pairs(CharacterSavedData) do
		if(next(CharacterSavedData[k]) ~= nil) then
			local ConfigCopy = CopyTable(CharacterSavedData[k]);
			AllStickyNotes[k] = CreateStickyNote(ConfigCopy);
			AllStickyNotes[k].Key = k;
		end
	end
end

---[[
function SaveMyData()
	ClearTable(CharacterSavedData);
	CharacterSavedData = {};
	
	for k, v in pairs(AllStickyNotes) do
			
		local TcR, TcG, TcB, TcA = AllStickyNotes[k].TextField:GetTextColor();
		local TTcR, TTcG, TTcB, TTcA = AllStickyNotes[k].TitleBar.Title:GetTextColor();
		local TbColor = AllStickyNotes[k].TitleBar.Color;
		local NtR = AllStickyNotes[k].BaseFrame.Color.R;
		local NtG = AllStickyNotes[k].BaseFrame.Color.G; 
		local NtB = AllStickyNotes[k].BaseFrame.Color.B;
		local NtA = AllStickyNotes[k].BaseFrame.Color.A;
		local MyFont, MyFontSize, MyFontFlags = AllStickyNotes[k].TextField:GetFont();
		local NoteWidth, NoteHeight = AllStickyNotes[k].BaseFrame:GetSize();
		ClearTable(CharacterSavedData[k]);
		CharacterSavedData[k] = {
			Title = tostring(AllStickyNotes[k].TitleBar.Title:GetText()),
			Text = tostring(AllStickyNotes[k].TextField:GetText()),
			Minimized = AllStickyNotes[k].Minimizer.Minimized,
			Visible = AllStickyNotes[k].Visible,
			Pos = {
				X = AllStickyNotes[k].TitleBar:GetLeft(),
				Y = AllStickyNotes[k].TitleBar:GetBottom(),
			},
			Size = {
				Width = NoteWidth, 
				Height = NoteHeight,
			},
			Font = MyFont, 
			FontSize = MyFontSize, 
			FontFlags = MyFontFlags,
			NoteColor = {
				R = NtR,
				G = NtG,
				B = NtB,
				A = NtA,
			},
			TextColor = {
				R = TcR,
				G = TcG,
				B = TcB,
				A = TcA,
			},
			TitleColor = {
				R = TTcR,
				G = TTcG,
				B = TTcB,
				A = TTcA,
			},
			TitleBarColor = {
				R = TbColor.R,
				G = TbColor.G,
				B = TbColor.B,
				A = TbColor.A,
			},
		};
	end
end
--]]

function AttachNoteComponents(note)
	note.BaseFrame:SetPoint("TOPLEFT", note.TitleBar, "BOTTOMLEFT");
	note.BaseFrame:SetPoint("TOPRIGHT", note.TitleBar, "BOTTOMRIGHT");
	
	note.BaseFrame.ScrollingFrame:SetPoint("TOPLEFT", note.BaseFrame, "TOPLEFT", WidgetSize);
	note.BaseFrame.ScrollingFrame:SetPoint("BOTTOMRIGHT", note.BaseFrame, "BOTTOMRIGHT", -ScrollbarWidth, WidgetSize);
	note.TextField:SetAllPoints(note.BaseFrame.ScrollingFrame);
	note.TextField:SetCursorPosition(0);
	
	note.Minimizer:SetPoint("LEFT", note.TitleBar, "LEFT", WidgetPadding, 0);
	note.LockWidget:SetPoint("LEFT", note.TitleBar, "LEFT", (WidgetPadding * 2 + WidgetSize), 0);
	note.CloseWidget:SetPoint("RIGHT", note.TitleBar, "RIGHT", -(WidgetPadding), 0);
	note.ChatWidget:SetPoint("RIGHT", note.TitleBar, "RIGHT", -(WidgetPadding * 2 + WidgetSize), 0);
	note.OptionsWidget:SetPoint("RIGHT", note.TitleBar, "RIGHT", -(WidgetPadding * 3 + (WidgetSize * 2)), 0);
	note.TitleBar.Title:ClearAllPoints();
	note.TitleBar.Title:SetPoint("LEFT", note.LockWidget, "RIGHT", WidgetPadding, 0);
	note.TitleBar.Title:SetPoint("RIGHT", note.OptionsWidget, "LEFT");
	note.TitleBar.Title:SetCursorPosition(0);
	
	AttachResizers(note.BaseFrame);
	MinimizerApplyState(note.Minimizer);
	VisibilityApplyState(note);
	
	note.BaseFrame:SetFrameLevel(BaseFrameLevel);
	note.TextField:SetFrameLevel(TextfieldFrameLevel);
end

function CreateNoteTitleBar(config, parent)
	local TitleBar = CreateDraggableFrame("TitleBar", UIParent);
	TitleBar.Color = config.TitleBarColor;--{R = 0.0, G = 0.0, B = 0.0, A = 0.5};
	TitleBar.Position = {X = config.Pos.X, Y = config.Pos.Y};
	TitleBar.Parent = parent;
	TitleBar.LastPos = {};
	
	TitleBar:SetSize(config.Size.Width, TitleBarHeight);
	TitleBar.Texture = GenerateTexture(nil, "BACKGROUND", TitleBar.Color, TitleBar);
	TitleBar:SetFrameLevel(TitleBarFrameLevel);
	TitleBar:SetPoint("BOTTOMLEFT", UIParent, "BOTTOMLEFT", config.Pos.X, config.Pos.Y);
	TitleBar:SetClampedToScreen(true);
	
	TitleBar.Title = CreateUneditableTextField(config, TitleBar);
	TitleBar.Title:SetTextColor(config.TitleColor.R, config.TitleColor.G, config.TitleColor.B, config.TitleColor.A);
	TitleBar.Title:SetFrameLevel(TitleBarFrameLevel + 1);
	
	TitleBar:SetScript("OnDragStart", DragTitleBarStart);
	TitleBar:SetScript("OnDragStop", DragTitleBarStop);
	
	return TitleBar;
end

function CreateMinimizeWidget(config, parent)
	local Minimizer = CreateFrame("Button", "Minimizer" .. GenerateGUID(), UIParent);
	
	Minimizer:SetSize(WidgetSize, WidgetSize);
	Minimizer.Minimized = config.Minimized;
	Minimizer:SetScript("OnClick", MinimizerOnClick);
	Minimizer.Texture = GenerateTexture(nil, "OVERLAY", {R=0.8, G=0.8, B=0.0, A=0.5}, Minimizer);
	Minimizer:SetFrameLevel(WidgetFrameLevel);
	
	function Minimizer:SetTextureExpanded()
		--self.Texture:SetTexture(0.8,0.8,0.0,0.5);
		self.Texture:SetTexture("Interface\\AddOns\\StickyNotes\\textures\\UI-Panel-CollapseButton-Up");
		self.Texture:SetTexCoord(0.2, 0.8, 0.2, 0.8);
	end
	
	function Minimizer:SetTextureCollapsed()
		--self.Texture:SetTexture(0.0,0.8,0.8,0.5);
		self.Texture:SetTexture("Interface\\AddOns\\StickyNotes\\textures\\UI-Panel-ExpandButton-Up");
		self.Texture:SetTexCoord(0.2, 0.8, 0.2, 0.8);
	end
	
	Minimizer.Parent = parent;
	return Minimizer;
end

function CreateLockWidget(config, parent)
	local LockWidget = CreateFrame("Button", "LockWidget" .. GenerateGUID(), UIParent);
	
	LockWidget:SetSize(WidgetSize, WidgetSize);
	LockWidget:SetScript("OnClick", LockOnClick);
	LockWidget.Texture = GenerateTexture(nil, "OVERLAY", {R=0.0, G=1.0, B=0.1, A=0.5}, LockWidget);
	LockWidget.Texture:SetTexture("Interface\\AddOns\\StickyNotes\\textures\\UI-LFG-ICON-LOCK");
	LockWidget.Texture:SetTexCoord(0, 0.85, 0, 0.85);
	LockWidget:SetFrameLevel(WidgetFrameLevel);
	
	function LockWidget:SetTextureUnlocked()
		--self.Texture:SetTexture(0.0, 1.0, 0.1, 0.5);
	end
	
	function LockWidget:SetTextureLocked()
		--self.Texture:SetTexture(1.0, 0.0, 0.3, 0.5);
	end
	
	LockWidget.Locked = false;
	LockWidget.Parent = parent;
	return LockWidget;
end

function CreateChatDumpWidget(parent)
	local ChatButton = CreateFrame("Button", "ChatButton" .. GenerateGUID(), UIParent);
	ChatButton:SetSize(WidgetSize, WidgetSize);
	ChatButton.Texture = GenerateTexture(nil, "BACKGROUND", {R=0.2, G=0.4, B=0.6, A=0.5}, ChatButton);
	ChatButton.Texture:SetTexture("Interface\\AddOns\\StickyNotes\\textures\\ReportLagIcon-Chat");
	ChatButton.Texture:SetTexCoord(0.2, 0.8, 0.2, 0.8);
	ChatButton:SetFrameLevel(WidgetFrameLevel);
	ChatButton:SetScript("OnClick", ChatDumperOnClick);
	
	local ChatDumper = CreateFrame("Frame", "ChatDumper" .. GenerateGUID(), UIParent, "UIDropDownMenuTemplate");
	UIDropDownMenu_SetWidth(ChatDumper, WidgetSize);
	UIDropDownMenu_SetText(ChatDumper, "Post");
	ChatDumper:SetFrameLevel(2);
	
	-- Create and bind the initialization function to the dropdown menu
	UIDropDownMenu_Initialize(ChatDumper, 
		function(self, level, menuList)
			local info = UIDropDownMenu_CreateInfo();
			if (level or 1) == 1 then
				CreateChatDumpCategories(info);
			else
				local PublicNames = GetPublicChannelNames();
				local PublicChannels, CustomChannels = GetAllChannelNames(PublicNames);
				
				info.func = self.ChannelSelect;
				
				if(menuList == 0) then
					info.text, info.arg1, info.arg2, info.checked = "Guild", "GUILD", nil, false;
					UIDropDownMenu_AddButton(info, level);
					
					info.text, info.arg1, info.checked = "Raid", "RAID", nil, false;
					UIDropDownMenu_AddButton(info, level);
					
					info.text, info.arg1, info.checked = "Party", "PARTY", nil, false;
					UIDropDownMenu_AddButton(info, level);
					
					info.text, info.arg1, info.checked = "Instance", "INSTANCE_CHAT", nil, false;
					UIDropDownMenu_AddButton(info, level);
				elseif(menuList == 1) then
					for k, v in pairs(CustomChannels) do
						info.text, info.arg1, info.arg2, info.checked = tostring(v), "CHANNEL", k, false
						UIDropDownMenu_AddButton(info, level);
					end
				elseif(menuList == 2) then
					for k, v in pairs(PublicChannels) do
						info.text, info.arg1, info.arg2, info.checked = tostring(v), "CHANNEL", k, false
						UIDropDownMenu_AddButton(info, level);
					end
				end
		end
	end, "MENU");
	
	function ChatDumper:ChannelSelect(arg1, arg2, checked)
		local NoteText = ChatDumper.Parent.TextField:GetText();
		PrintNoteFormattedText(NoteText, arg1, arg2);
		CloseDropDownMenus();
	end
	
	ChatButton.ChatDumper = ChatDumper;
	ChatButton.Parent = parent;
	ChatButton.ChatDumper.Parent = parent;
	return ChatButton;
end

function CreateOptionsWidget(parent)
	local OptionsWidget = CreateFrame("Button", "OptionsWidget" .. GenerateGUID(), UIParent);
	OptionsWidget:SetSize(WidgetSize, WidgetSize);
	OptionsWidget.Texture = GenerateTexture(nil, "OVERLAY", {R=1.0, G=1.0, B=1.0, A=1.0}, OptionsWidget); --{R=0.8, G=0.0, B=0.8, A=0.5}
	OptionsWidget.Texture:SetTexture("Interface\\AddOns\\StickyNotes\\textures\\HelpIcon-CharacterStuck");
	OptionsWidget.Texture:SetTexCoord(0.15, 0.85, 0.15, 0.85);
	OptionsWidget:SetFrameLevel(WidgetFrameLevel);
	OptionsWidget:SetScript("OnClick", OptionsOnClick);
	
	OptionsWidget.Parent = parent;
	return OptionsWidget;
end

function CreateNoteCloseWidget(parent)
	local NoteCloseWidget = CreateFrame("Button", "NoteCloseWidget" .. GenerateGUID(), UIParent);
	NoteCloseWidget:SetSize(WidgetSize, WidgetSize);
	NoteCloseWidget.Texture = GenerateTexture(nil, "OVERLAY", {R=0.8, G=0.0, B=0.0, A=0.5}, NoteCloseWidget);
	NoteCloseWidget.Texture:SetTexture("Interface\\AddOns\\StickyNotes\\textures\\UI-Panel-MinimizeButton-Up");
	NoteCloseWidget.Texture:SetTexCoord(0.2, 0.8, 0.2, 0.8);
	NoteCloseWidget:SetFrameLevel(WidgetFrameLevel);
	NoteCloseWidget:SetScript("OnClick", NoteCloseOnClick);
	
	NoteCloseWidget.Parent = parent;
	return NoteCloseWidget;
end

function CreateBaseFrame(config, parent)
	local BaseFrame = CreateFrame("Frame", "BaseFrame" .. GenerateGUID(), UIParent);
	BaseFrame:SetClampedToScreen(true);
	BaseFrame:EnableMouse(true);
	BaseFrame:SetResizable(true);
	BaseFrame:SetSize(config.Size.Width, config.Size.Height);
	BaseFrame.Color = {R = config.NoteColor.R, G = config.NoteColor.G, B = config.NoteColor.B, A = config.NoteColor.A};
	BaseFrame.Texture = GenerateTexture(nil, "BACKGROUND", BaseFrame.Color, BaseFrame);
	BaseFrame:SetFrameLevel(BaseFrameLevel);
	BaseFrame.Parent = parent
	
	parent.TextField = CreateTextField(config, parent, BaseFrame);
	
	local ScrollingFrame = CreateFrame("ScrollFrame", "ScrollingFrame" .. GenerateGUID(), BaseFrame, "UIPanelScrollFrameTemplate");
	ScrollingFrame:SetFrameLevel(BaseFrameLevel);
	
	ScrollingFrame:SetScrollChild(parent.TextField);
	ScrollingFrame.Parent = parent;
	BaseFrame.ScrollingFrame = ScrollingFrame;
	return BaseFrame;
end

function CreateTextField(config, parent, baseframe)
	local TextField = CreateFrame("EditBox", "TextField" .. GenerateGUID(), baseframe);
	TextField:EnableMouse(true)
	TextField:SetMultiLine(true);
	TextField:SetAutoFocus(false);
	
	TextField:SetSize(config.Size.Width, config.Size.Height);
	TextField:SetFont(config.Font, config.FontSize, config.FontFlags);--"Fonts\\FRIZQT__.TTF", 11, "OUTLINE, MONOCHROME");
	TextField:SetTextColor(config.TextColor.R, config.TextColor.G, config.TextColor.B, config.TextColor.A);
	TextField:SetText(tostring(config.Text));
	TextField:SetFrameLevel(TextfieldFrameLevel);
	
	TextField.Color = {R = config.TextColor.R, G = config.TextColor.G, B = config.TextColor.B, A = config.TextColor.A};
	TextField.Parent = parent;
	TextField.TimeSinceLastUpdate = 0;
	
	TextField:SetScript("OnUpdate", TextFieldOnUpdate);
	TextField:SetScript("OnEscapePressed", 
		function(self)
			self:ClearFocus();
		end);
	TextField:SetScript("OnEditFocusLost", 
		function(self)
			local Text = tostring(self:GetText());
			CharacterSavedData[self.Parent.Key].Text = Text;
		end);
		
	return TextField;
end

function CreateUneditableTextField(config, parent)
	local TextField = CreateFrame("EditBox", "TextField" .. GenerateGUID(), UIParent);
	TextField:SetClampedToScreen(true);
	TextField:SetAutoFocus(false);
	
	TextField:SetText(tostring(config.Title));
	TextField:SetTextColor(0, 0, 1, 1);
	TextField:SetSize(config.Size.Width, TitleBarHeight)
	TextField:SetFont(config.Font, config.FontSize, config.FontFlags);
	DisableInput(TextField);
	
	return TextField;
end

function CreateDraggableFrame(id, parent)
	local DragFrame = CreateFrame("Frame", id .. GenerateGUID(), parent);
	DragFrame:SetMovable(true);
	DragFrame:EnableMouse(true);
	DragFrame:SetResizable(true);
	
	DragFrame:RegisterForDrag("LeftButton");
	DragFrame:SetScript("OnDragStart", DragFrame.StartMoving);
	DragFrame:SetScript("OnDragStop", DragFrame.StopMovingOrSizing);
	
	return DragFrame;
end


function CreateStickyNote(config)
	local NewNote = {};
	if(next(config) == nil) then
		config = CopyTable(DefaultNoteConfig);
	end
	
	NewNote.TitleBar = CreateNoteTitleBar(config, NewNote);
	NewNote.BaseFrame = CreateBaseFrame(config, NewNote);
	NewNote.Minimizer = CreateMinimizeWidget(config, NewNote);
	NewNote.ChatWidget = CreateChatDumpWidget(NewNote);
	NewNote.OptionsWidget = CreateOptionsWidget(NewNote);
	NewNote.CloseWidget = CreateNoteCloseWidget(NewNote);
	NewNote.LockWidget = CreateLockWidget(config, NewNote);
	NewNote.Visible = config.Visible;
	CreateResizeWidgets(NewNote.BaseFrame);
	AttachNoteComponents(NewNote);
	return NewNote;
end

---[[
SLASH_STICKYNOTES1 = '/stn';
function SlashCmdList.STICKYNOTES(msg, editBox)
	if(msg == 's') then
		SaveMyData();
	elseif(msg == '') then
		--InterfaceMainPanel.PanelContent:Show();
		--InterfaceMainPanel.NotesList:Show();
		print("sticky notes commands:");
		print("stn c: create new sticky note");
		print("stn all: show all sticky notes");
		print("stn p (or stn panel): open the note management panel");
	elseif(msg == 'all') then
		ShowAllNotes();
	elseif(msg == 'c') then
		AddNewNote();
		UpdateInterfaceNotesList(InterfaceMainPanel);
	elseif((msg == 'panel') or (msg == 'p')) then
		ShowInterfacePanel();
	end
end
--]]