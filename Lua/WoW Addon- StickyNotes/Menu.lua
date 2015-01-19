InterfacePanelWidth = 550;
InterfacePanelHeight = 500;
InterfacePanelScrollHeight = 100;
ScrollBaseWidth = 500;
ScrollBaseHeight = 300;
NoteCellWidth = 350;
NoteCellHeight = 20;
NoteCellPadding = 10;
CellButtonWidth = 55;
CellButtonHeight = 26;

function CreateInterfacePanel()
	local MainPanel = CreateDraggableFrame("MainPanel" .. GenerateGUID(), UIParent);
	MainPanel.Texture = GenerateTexture(nil, "BACKGROUND", {R=0.0, G=0.0, B=0.0, A=0.5}, MainPanel);
	MainPanel:SetSize(InterfacePanelWidth, InterfacePanelHeight);
	MainPanel:SetPoint("CENTER");
	
	--the sole purpose of this frame is to make the texture stretch far enough to include the scrollbar
	MainPanel.ScrollBackdrop = CreateFrame("Frame", "ScrollBackdrop" .. GenerateGUID(), MainPanel);
	MainPanel.ScrollBackdrop:SetWidth(ScrollBaseWidth + ScrollbarWidth);
	MainPanel.ScrollBackdrop:SetHeight(ScrollBaseHeight);
	MainPanel.ScrollBackdrop.Texture = GenerateTexture(nil, "BACKGROUND", {R=0.0, G=0.0, B=0.0, A=0.5}, MainPanel.ScrollBackdrop);
	MainPanel.ScrollBackdrop:SetPoint("CENTER", MainPanel);
	
	---[[	ScrollingFrame: a scrolling list of all StickyNotes, the size of this frame dictates how much of the content can be seen
	MainPanel.ScrollingFrame = CreateFrame("ScrollFrame", "NotesListScrollFrame" .. GenerateGUID(), MainPanel, "UIPanelScrollFrameTemplate");
	MainPanel.ScrollingFrame:SetWidth(ScrollBaseWidth);
	MainPanel.ScrollingFrame:SetHeight(ScrollBaseHeight);
	MainPanel.ScrollingFrame:SetPoint("TOPLEFT", MainPanel.ScrollBackdrop);
	--]]

	---[[	ScrollContent: a frame to be scrolled through, attach list-style content here
	MainPanel.ScrollContent = CreateFrame("Frame", "ScrollContent" .. GenerateGUID(), MainPanel.ScrollingFrame); 
	MainPanel.ScrollContent:EnableMouse(true);
	MainPanel.ScrollContent:SetWidth(ScrollBaseWidth);
	MainPanel.ScrollContent:SetHeight(InterfacePanelScrollHeight);
	--]]
	
	MainPanel.Notes = {};
	MainPanel.Notes = CreateInterfaceNotesList(MainPanel); --dependent upon ScrollContent existing as a frame
	
	MainPanel.AddButton = CreateAddNoteButton(MainPanel);
	MainPanel.AddButton:SetPoint("TOP", MainPanel, "TOP", 0, -50);
	
	MainPanel.CloseWidget = CreateCloseWidget(MainPanel);
	MainPanel.CloseWidget:SetPoint("TOPRIGHT", MainPanel, "TOPRIGHT", -5, -5);
	
	MainPanel.ScrollingFrame:SetScrollChild(MainPanel.ScrollContent);
	InterfaceMainPanel = MainPanel;
	MainPanel:Hide();
end

function ShowInterfacePanel()
	InterfaceMainPanel:Show();
end

function CreateInterfaceNotesList(mainpanel)
	local Counter = 0;
	local Notes = {};
	for k,v in pairs(AllStickyNotes) do
		Notes[k] = CreateDraggableFrame("NoteK" .. GenerateGUID(), mainpanel.ScrollContent);
		Notes[k]:SetClampedToScreen(true);
		Notes[k]:SetSize(NoteCellWidth, NoteCellHeight);
		Notes[k].Texture = GenerateTexture(nil, "BACKGROUND", {R=0.0, G=1.0, B=1.0, A=0.1}, Notes[k]);
		Notes[k]:SetPoint("TOPLEFT", mainpanel.ScrollContent, "TOPLEFT", 0, -((NoteCellHeight+NoteCellPadding) * Counter));
		
		Notes[k].TextField = CreateFrame("EditBox", "TextField" .. GenerateGUID(), Notes[k]);
		DisableInput(Notes[k].TextField);
		Notes[k].TextField:SetAutoFocus(false);
		Notes[k].TextField:SetText(AllStickyNotes[k].TitleBar.Title:GetText());
		Notes[k].TextField:SetTextColor(1, 1, 1, 1);
		Notes[k].TextField:SetSize(NoteCellWidth, NoteCellHeight + NoteCellPadding);
		Notes[k].TextField:SetFont("Fonts\\FRIZQT__.TTF", 14, "OUTLINE, MONOCHROME");
		Notes[k].TextField:SetAllPoints(Notes[k]);
		
		Notes[k].VisButton = CreateVisibilityButton(Notes[k]);
		Notes[k].VisButton:SetPoint("TOPLEFT", Notes[k], "TOPRIGHT", NoteCellPadding, 0);
		Notes[k].VisButton:SetPoint("BOTTOMLEFT", Notes[k], "BOTTOMRIGHT", NoteCellPadding, 0);
		
		Notes[k].DeleteButton = CreateDeleteButton(Notes[k]);
		Notes[k].DeleteButton:SetPoint("TOPLEFT", Notes[k].VisButton, "TOPRIGHT", NoteCellPadding, 0);
		Notes[k].DeleteButton:SetPoint("BOTTOMLEFT", Notes[k].VisButton, "BOTTOMRIGHT", NoteCellPadding, 0);
		
		Notes[k]:SetScript("OnDragStart", ReorderDragStart);
		Notes[k]:SetScript("OnDragStop", ReorderDragStop);
		
		Notes[k].Key = k;
		Notes[k].List = Notes;
		Counter = Counter + 1;
	end
	
	return Notes;
end

function ReorderDragStart(self, button)
	self.LastPos = {};
	self.LastPos.X = (self:GetLeft());
	self.LastPos.Y = (self:GetBottom());
	
	self.OldParent = self:GetParent();
	self:SetParent(UIParent);
	
	self:ClearAllPoints();
	self:SetPoint("BOTTOMLEFT", UIParent, "BOTTOMLEFT", self.LastPos.X, self.LastPos.Y);
	self:SetFrameLevel(5);
	
	self:SetScript("OnUpdate", UpdateReorder);
	self:StartMoving();
end

function ReorderDragStop(self)
	self:StopMovingOrSizing();
	self:SetScript("OnUpdate", nil);
	self.LastPos = {};
	UpdateNoteListOrder(self);
	UpdateInterfaceNotesList();
	SaveMyData();
end

function UpdateNoteListOrder(noteframe)
	local MovingBottom, MovingTop = floor(noteframe:GetBottom()), floor(noteframe:GetTop());
	local OldKey, NewKey = noteframe.Key, nil;
	local NoteframeList = noteframe.List;
	
	--find where this note belongs in the list after drag+drop
	for k,v in pairs(NoteframeList) do
		if(k ~= OldKey) then
			local CurrentBottom, CurrentTop = floor(v:GetBottom()), floor(v:GetTop());
			
			if((MovingBottom > CurrentBottom - (NoteCellPadding / 2)) and (MovingTop <= CurrentTop + (NoteCellPadding / 2))) then
				NewKey = k;
				break;
			elseif((MovingTop > CurrentBottom - (NoteCellPadding / 2)) and (MovingBottom <= CurrentTop + (NoteCellPadding / 2))) then
				NewKey = k;
				break;
			end
		end
	end
	
	--do nothing if no new position was found
	if(NewKey == nil) then
		return;
	end
	
	AllStickyNotes[OldKey].Key = NewKey;
	local SmallKey, BigKey = math.min(OldKey, NewKey), math.max(OldKey, NewKey);
	local Step = 1;
	
	if(OldKey > NewKey) then
		BigKey = BigKey - 1;
	elseif(NewKey > OldKey) then
		SmallKey = SmallKey + 1;
		Step = -1;
	end
	
	local NewNoteList = {};
	
	--update all keys that will change with this move
	for k,v in pairs(AllStickyNotes) do
		if((k >= SmallKey) and (k <= BigKey)) then
			AllStickyNotes[k].Key = AllStickyNotes[k].Key + Step;
		end
		
		local Index = AllStickyNotes[k].Key;
		NewNoteList[Index] = TableDeepCopy(AllStickyNotes[k]);
	end
	
	--update the master note table
	ClearTable(AllStickyNotes);
	AllStickyNotes = {};
	AllStickyNotes = TableDeepCopy(NewNoteList);
	ClearTable(NewNoteList);
end

function UpdateReorder(self, arg1)
	local ParentFrame = InterfaceMainPanel.ScrollingFrame;
	self.LastPos.X, self.LastPos.Y = self:GetLeft(), self:GetBottom();
	local Top, Bottom, Left, Right = floor(ParentFrame:GetTop()), floor(ParentFrame:GetBottom()), floor(ParentFrame:GetLeft()), floor(ParentFrame:GetRight());
	
	if(Top < self:GetTop()) then
		self.LastPos.Y = Top - NoteCellHeight;
	elseif(Bottom > self.LastPos.Y) then
		self.LastPos.Y = Bottom;
	end
	
	if(Left > self.LastPos.X) then
		self.LastPos.X = Left;
	elseif((Right - ScrollBaseWidth) < self.LastPos.X) then
		self.LastPos.X = Right - ScrollBaseWidth;
	end
	
	self:StopMovingOrSizing();
	self:SetPoint("BOTTOMLEFT", UIParent, "BOTTOMLEFT", self.LastPos.X, self.LastPos.Y);
	self:SetSize(NoteCellWidth, NoteCellHeight);
	self:StartMoving();
end

function UpdateInterfaceNotesList()
	for k, v in pairs(InterfaceMainPanel.Notes) do
		InterfaceMainPanel.Notes[k]:Hide();
	end
	ClearTable(InterfaceMainPanel.Notes);
	InterfaceMainPanel.Notes = CreateInterfaceNotesList(InterfaceMainPanel);
end

function CreateVisibilityButton(parent)
	local VisButton = CreateFrame("Button", "VisButton" .. GenerateGUID(), parent);
	VisButton:SetSize(CellButtonWidth, CellButtonHeight);
	VisButton.Texture = GenerateTexture(nil, "BACKGROUND", {R=0.0, G=0.6, B=0.2, A=1.0}, VisButton);
	VisButton:SetScript("OnClick", VisButtonOnClick);
	VisButton.Parent = parent;
	
	VisButton.TextField = CreateFrame("EditBox", "TextField" .. GenerateGUID(), VisButton);
	VisButton.TextField:SetAutoFocus(false);
	VisButton.TextField:SetText("Show");
	VisButton.TextField:SetTextColor(1, 1, 1, 1);
	VisButton.TextField:SetSize(CellButtonWidth, CellButtonHeight);
	VisButton.TextField:SetFont("Fonts\\FRIZQT__.TTF", 14, "OUTLINE, MONOCHROME");
	VisButton.TextField:SetAllPoints(VisButton);
	DisableInput(VisButton.TextField);
	
	return VisButton;
end

function CreateDeleteButton(parent)
	local DeleteButton = CreateFrame("Button", "DeleteButton" .. GenerateGUID(), parent);
	DeleteButton:SetSize(CellButtonWidth, CellButtonHeight);
	DeleteButton.Texture = GenerateTexture(nil, "BACKGROUND", {R=0.5, G=0.0, B=0.0, A=1.0}, DeleteButton);
	DeleteButton:SetScript("OnClick", DeleteButtonOnClick);
	DeleteButton.Parent = parent;
	
	DeleteButton.TextField = CreateFrame("EditBox", "TextField" .. GenerateGUID(), DeleteButton);
	DeleteButton.TextField:SetClampedToScreen(true);
	DeleteButton.TextField:SetAutoFocus(false);
	DeleteButton.TextField:SetText("Delete");
	DeleteButton.TextField:SetTextColor(1, 1, 1, 1);
	DeleteButton.TextField:SetSize(CellButtonWidth, CellButtonHeight);
	DeleteButton.TextField:SetFont("Fonts\\FRIZQT__.TTF", 14, "OUTLINE, MONOCHROME");
	DeleteButton.TextField:SetAllPoints(DeleteButton);
	DisableInput(DeleteButton.TextField);
	
	return DeleteButton;
end

function CreateAddNoteButton(parent)
	local AddButton = CreateFrame("Button", "AddButton" .. GenerateGUID(), parent);
	local Width, Height = 140, 26;
	AddButton:SetSize(Width, Height);
	AddButton.Texture = GenerateTexture(nil, "BACKGROUND", {R=0.0, G=0.6, B=0.2, A=1.0}, AddButton);
	AddButton:SetScript("OnClick", AddButtonOnClick);
	AddButton.Parent = parent;
	
	AddButton.TextField = CreateFrame("EditBox", "TextField" .. GenerateGUID(), AddButton);
	AddButton.TextField:SetClampedToScreen(true);
	AddButton.TextField:SetAutoFocus(false);
	AddButton.TextField:SetText("Create Sticky Note");
	AddButton.TextField:SetTextColor(1, 1, 1, 1);
	AddButton.TextField:SetSize(Width, Height);
	AddButton.TextField:SetFont("Fonts\\FRIZQT__.TTF", 14, "OUTLINE, MONOCHROME");
	AddButton.TextField:SetAllPoints(AddButton);
	DisableInput(AddButton.TextField);
	
	return AddButton;
end

function VisButtonOnClick(self, button, down)
	local MyNote = AllStickyNotes[self.Parent.Key];
	if(MyNote.Visible == false) then
		ShowNote(MyNote);
	else
		HideNote(MyNote);
	end
end

function DeleteButtonOnClick(self, button, down)
	local MyNote = AllStickyNotes[self.Parent.Key];
	HideNote(MyNote);
	if(MyNote == OptionsGui.CurrentNote) then
		OptionsGui:HideMe();
	end
	table.remove(AllStickyNotes, self.Parent.Key);
	UpdateNoteKeys(AllStickyNotes);
	SaveMyData();
	UpdateInterfaceNotesList(InterfaceMainPanel);
end

function AddButtonOnClick(self, button, down)
	AddNewNote();
	InterfaceOptionsFrame:Hide();
	UpdateInterfaceNotesList(InterfaceMainPanel);
end


function CreateTitleEditor(guiframe)
	local Title = CreateFrame("EditBox", "Title" .. GenerateGUID(), guiframe);
	Title:SetClampedToScreen(true);
	Title:SetAutoFocus(false);
	
	Title:SetText("Title");
	Title:SetTextColor(1, 1, 1, 1);
	Title:SetSize(70, 20);
	Title:SetFont("Fonts\\FRIZQT__.TTF", 14, "OUTLINE, MONOCHROME");
	local Position = 0;
	Title:SetPoint("TOPLEFT", guiframe, "TOPLEFT", 20, -(20 + (Position * 60)));
	Title:SetFrameLevel(1);
	
	local TitleEditor = CreateFrame("EditBox", "TitleEditor" .. GenerateGUID(), guiframe);
	TitleEditor:SetMaxLetters(30);
	TitleEditor.Texture = GenerateTexture(nil, "BACKGROUND", {R=0.3, G=0.3, B=0.3, A=0.3}, TitleEditor);
	TitleEditor:EnableMouse(true)
	TitleEditor:SetAutoFocus(false);
	TitleEditor.Parent = guiframe;
	
	local NoteTitle = "Sticky Note";
	if(guiframe.CurrentNote ~= nil) then
		NoteTitle = guiframe.CurrentNote.TitleBar.Title:GetFont();
	end
	
	TitleEditor:SetText("");
	TitleEditor:SetTextColor(1, 1, 1, 1);
	TitleEditor:SetSize(240, 30);
	TitleEditor:SetFont("Fonts\\FRIZQT__.TTF", 14, "OUTLINE, MONOCHROME");
	TitleEditor:SetPoint("TOPLEFT", Title, "BOTTOMLEFT", 0, 0);
	TitleEditor:SetFrameLevel(2);
	
	TitleEditor:SetScript("OnEnterPressed", 
		function(self)
			self:ClearFocus();
			if(self.Parent.CurrentNote ~= nil) then
				local NewTitle = self:GetText();
				UpdateTitle(self.Parent.CurrentNote, NewTitle);
			end
		end);
		
	TitleEditor:SetScript("OnEscapePressed", 
		function(self)
			self:ClearFocus();
		end);
	
	return TitleEditor;
end

function CreateTitleColorEditor(guiframe)
	local Title = CreateFrame("EditBox", "TitleTextColor" .. GenerateGUID(), guiframe);
	Title:SetClampedToScreen(true);
	Title:SetAutoFocus(false);
	
	Title:SetText("Title Colors");
	Title:SetTextColor(1, 1, 1, 1);
	Title:SetSize(100, 20);
	Title:SetFont("Fonts\\FRIZQT__.TTF", 14, "OUTLINE, MONOCHROME");
	local Position = 1;
	Title:SetPoint("TOPLEFT", guiframe, "TOPLEFT", 20, -(20 + (Position * 60)));
	Title:SetFrameLevel(1);
	
	local TitleColorEditor = CreateFrame("Button", "TitleColorEditor" .. GenerateGUID(), guiframe);
	TitleColorEditor:SetSize(20, 20);
	TitleColorEditor:SetFrameLevel(2);
	TitleColorEditor.Parent = guiframe;
	
	local Color = {R=1.0, G=1.0, B=1.0, A=1.0};
	if(guiframe.CurrentNote ~= nil) then
		Color = guiframe.CurrentNote.TextField.Color;
	end
	
	TitleColorEditor.Texture = GenerateTexture(nil, "BACKGROUND", Color, TitleColorEditor);
	TitleColorEditor:SetPoint("TOPLEFT", Title, "BOTTOMLEFT", 0, 0);
	TitleColorEditor:SetFrameLevel(2);
	
	TitleColorEditor:SetScript("OnClick", 
		function(self, button, down)
			local R, G, B, A = self.Parent.CurrentNote.TitleBar.Title:GetTextColor();
			ColorAssignerCallback = TitleTextColorCallback;
			ColorEditingFrame = self.Parent;
			ShowColorPicker(R, G, B, A, ColorCallback);
		end);
		
	local BarEditor = CreateFrame("Button", "TitleBarColorEditor" .. GenerateGUID(), guiframe);
	BarEditor:SetSize(20, 20);
	BarEditor:SetFrameLevel(2);
	BarEditor.Parent = guiframe;
	
	Color = {R=0.0, G=1.0, B=0.0, A=1.0};
	if(guiframe.CurrentNote ~= nil) then
		Color = guiframe.CurrentNote.TitleBar.Color;
	end
	
	BarEditor.Texture = GenerateTexture(nil, "BACKGROUND", Color, BarEditor);
	BarEditor:SetPoint("LEFT", TitleColorEditor, "RIGHT", 4, 0);
	BarEditor:SetFrameLevel(2);
	
	BarEditor:SetScript("OnClick", 
		function(self, button, down)
			local Color = self.Parent.CurrentNote.TitleBar.Color;
			ColorAssignerCallback = TitleBarColorCallback;
			ColorEditingFrame = self.Parent;
			ShowColorPicker(Color.R, Color.G, Color.B, Color.A, ColorCallback);
		end);
	
	guiframe.BarEditor = BarEditor;
	return TitleColorEditor;
end

function CreateFontSizeEditor(guiframe)
	local Title = CreateFrame("EditBox", "Title" .. GenerateGUID(), guiframe);
	Title:SetClampedToScreen(true);
	Title:SetAutoFocus(false);
	
	Title:SetText("Font Size");
	Title:SetTextColor(1, 1, 1, 1);
	Title:SetSize(100, 20);
	Title:SetFont("Fonts\\FRIZQT__.TTF", 14, "OUTLINE, MONOCHROME");
	local Position = 2;
	Title:SetPoint("TOPLEFT", guiframe, "TOPLEFT", 20, -(20 + (Position * 60)));
	Title:SetFrameLevel(1);
	
	local FontSizeEditor = CreateFrame("EditBox", "HeightEditor" .. GenerateGUID(), guiframe);
	FontSizeEditor:SetNumeric();
	FontSizeEditor:SetMaxLetters(3);
	FontSizeEditor.Texture = GenerateTexture(nil, "BACKGROUND", {R=0.3, G=0.3, B=0.3, A=0.3}, FontSizeEditor);
	FontSizeEditor:EnableMouse(true)
	FontSizeEditor:SetAutoFocus(false);
	FontSizeEditor.Parent = guiframe;
	
	local File, Height, Flags = "Fonts\\FRIZQT__.TTF", 10, "OUTLINE, MONOCHROME";
	if(guiframe.CurrentNote ~= nil) then
		File, Height, Flags = guiframe.CurrentNote.TextField:GetFont();
	end
	
	FontSizeEditor:SetText(floor(Height + 0.5));
	FontSizeEditor:SetTextColor(1, 1, 1, 1);
	FontSizeEditor:SetSize(40, 30);
	FontSizeEditor:SetFont("Fonts\\FRIZQT__.TTF", 14, "OUTLINE, MONOCHROME");
	FontSizeEditor:SetFrameLevel(2);
	FontSizeEditor:SetPoint("TOPLEFT", Title, "BOTTOMLEFT", 0, 0);
	
	FontSizeEditor:SetScript("OnEnterPressed", 
		function(self)
			self:ClearFocus();
			if(self.Parent.CurrentNote ~= nil) then
				local File, Height, Flags = self.Parent.CurrentNote.TextField:GetFont();
				Height = self:GetNumber();
				UpdateFont(self.Parent.CurrentNote, File, Height, Flags);
			end
		end);
		
	FontSizeEditor:SetScript("OnEscapePressed", 
		function(self)
			self:ClearFocus();
		end);
	
	return FontSizeEditor;
end

function CreateTextColorEditor(guiframe)
	local Title = CreateFrame("EditBox", "Title" .. GenerateGUID(), guiframe);
	Title:SetClampedToScreen(true);
	Title:SetAutoFocus(false);
	
	Title:SetText("Text Color");
	Title:SetTextColor(1, 1, 1, 1);
	Title:SetSize(100, 20);
	Title:SetFont("Fonts\\FRIZQT__.TTF", 14, "OUTLINE, MONOCHROME");
	local Position = 3;
	Title:SetPoint("TOPLEFT", guiframe, "TOPLEFT", 20, -(20 + (Position * 60)));
	Title:SetFrameLevel(1);
	
	local TextColorEditor = CreateFrame("Button", "TextColorEditor" .. GenerateGUID(), guiframe);
	
	TextColorEditor:SetSize(20, 20);
	TextColorEditor:SetFrameLevel(2);
	
	TextColorEditor.Parent = guiframe;
	
	local Color = {R=1.0, G=1.0, B=1.0, A=1.0};
	if(guiframe.CurrentNote ~= nil) then
		Color = guiframe.CurrentNote.TextField.Color;
	end
	
	TextColorEditor.Texture = GenerateTexture(nil, "BACKGROUND", Color, TextColorEditor);
	
	TextColorEditor:SetPoint("TOPLEFT", Title, "BOTTOMLEFT", 0, 0);
	TextColorEditor:SetFrameLevel(2);
	
	TextColorEditor:SetScript("OnClick", 
		function(self, button, down)
			local Color = self.Parent.CurrentNote.TextField.Color;
			ColorAssignerCallback = TextColorCallback;
			ColorEditingFrame = self.Parent;
			ShowColorPicker(Color.R, Color.G, Color.B, Color.A, ColorCallback);
		end);
	
	return TextColorEditor;
end

function CreateNoteColorEditor(guiframe)
	local Title = CreateFrame("EditBox", "Title" .. GenerateGUID(), guiframe);
	Title:SetClampedToScreen(true);
	Title:SetAutoFocus(false);
	
	Title:SetText("Background Color");
	Title:SetTextColor(1, 1, 1, 1);
	Title:SetSize(150, 20);
	Title:SetFont("Fonts\\FRIZQT__.TTF", 14, "OUTLINE, MONOCHROME");
	local Position = 4;
	Title:SetPoint("TOPLEFT", guiframe, "TOPLEFT", 20, -(20 + (Position * 60)));
	Title:SetFrameLevel(1);
	
	local NoteColorEditor = CreateFrame("Button", "NoteColorEditor" .. GenerateGUID(), guiframe);
	NoteColorEditor:SetSize(20, 20);
	NoteColorEditor:SetFrameLevel(2);
	NoteColorEditor.Parent = guiframe;
	
	local Color = {R=1.0, G=1.0, B=1.0, A=1.0};
	if(guiframe.CurrentNote ~= nil) then
		Color = guiframe.CurrentNote.BaseFrame.Color;
	end
	
	NoteColorEditor.Texture = GenerateTexture(nil, "BACKGROUND", Color, NoteColorEditor);
	
	NoteColorEditor:SetPoint("TOPLEFT", Title, "BOTTOMLEFT", 0, 0);
	NoteColorEditor:SetFrameLevel(2);
	
	NoteColorEditor:SetScript("OnClick", 
		function(self, button, down)
			local Color = self.Parent.CurrentNote.BaseFrame.Color;
			ColorAssignerCallback = NoteColorCallback;
			ColorEditingFrame = self.Parent;
			ShowColorPicker(Color.R, Color.G, Color.B, Color.A, ColorCallback);
		end);
	
	return NoteColorEditor;
end

function CreateCloseWidget(parent)
	local CloseWidget = CreateFrame("Button", "CloseWidget" .. GenerateGUID(), parent);
	CloseWidget:SetSize(WidgetSize, WidgetSize);
	CloseWidget.Texture = GenerateTexture(nil, "BACKGROUND", {R=0.8, G=0.0, B=0.0, A=0.5}, CloseWidget);
	CloseWidget.Texture:SetTexture("Interface\\AddOns\\StickyNotes\\textures\\UI-Panel-MinimizeButton-Up");
	CloseWidget.Texture:SetTexCoord(0.2, 0.8, 0.2, 0.8);
	CloseWidget:SetFrameLevel(2);
	CloseWidget:SetPoint("TOPRIGHT", parent, "TOPRIGHT", -5, -5);
	CloseWidget:SetScript("OnClick", CloseOnClick);
	
	CloseWidget.Parent = parent;
	return CloseWidget;
end