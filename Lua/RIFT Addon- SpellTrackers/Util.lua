--Count the number of elements in a table
function TableSize(Tab)
	local Count = 0;
	local Ind = next(Tab);
	
	while Ind ~= nil do
		Count = Count + 1;
		Ind = next(Tab, Ind);
	end
	
	return Count;
end

--Returns the total number of trackers
function GetTrackerCount()
	return TableSize(Trackers);
end

--Determines whether or not a number lies in a given range
function InRange(Value, Min, Max)
	if (Min == nil) and (Max == nil) then
		return true; --fringe cases in case of nil bounds
	elseif (Min == nil) and (Value <= Max) then
		return true;
	elseif (Value >= Min) and (Max == nil) then
		return true;
	elseif (Value >= Min) and (Value <= Max) then
		return true;
	else
		return false;
	end
end

--Only really need 1 color picker frame
ColorPicker = UI.CreateFrame("Frame", "colorpicker_frame", context);
ColorPicker:SetLayer(999);
ColorPicker:SetVisible(false);
ColorPicker.InvokingFrame = nil;

--Closes the color picking window
function CloseColorPicker()
	ColorPicker.InvokingFrame = nil;
	ColorPicker:ClearAll();
	ColorPicker:SetVisible(false);
end

--The color picker is picky about handlers, can't have member functions
function ColorHandler(r, g, b)
	ColorPicker.InvokingFrame:ProcessColor(r, g, b);
	CloseColorPicker();
end

Library.ColorChooser.CreateWidget(ColorPicker, ColorHandler, 8, false);

--Creates a frame to give the user access to the color picker and display
--what color they have chosen
function CreateColorFrame(ID, Parent, MyVar)
	local ColorFrame = UI.CreateFrame("Frame", ID .. "_colorframe", Parent);
	ColorFrame:SetWidth(GUIVals.ColorFrameSize);
	ColorFrame:SetHeight(GUIVals.ColorFrameSize);
	local MyColor = Trackers[Parent.MyTracker].Config[MyVar];
	ColorFrame:SetBackgroundColor(MyColor.r,MyColor.g,MyColor.b,1);
	
	function ColorFrame.Event:LeftClick()
		ColorPicker.InvokingFrame = self;
		ColorPicker:SetPoint("BOTTOMLEFT", self, "TOPRIGHT");
		ColorPicker:SetVisible(true);
	end
	
	function ColorFrame:ProcessColor(r, g, b)
		self:SetBackgroundColor(r,g,b,1);
		local Tracker = Trackers[Parent.MyTracker];
		Tracker.Config[MyVar].r = r;
		Tracker.Config[MyVar].g = g;
		Tracker.Config[MyVar].b = b;
		Tracker:RefreshData();
	end
	
	return ColorFrame;
end

--Creates a RiftTextField, attaches it to the provided parent window,
--and associates it with the provided variable
function CreateEditText(TextID, Parent, MyVar)
	local TextField = UI.CreateFrame("RiftTextfield", tostring(TextID), Parent);
	TextField:SetBackgroundColor(0, 0, 0, 1);
	Library.LibSimpleWidgets.SetBorder(TextField, GUIVals.BorderWidth, 0.5, 0.5, 0.5, 1);
	
	local VarStr = tostring(Trackers[Parent.MyTracker].Config[MyVar]);
	TextField:SetText(VarStr);
	TextField.MyVar = MyVar;
	
	function TextField.Event:KeyDown(button)
		if button == "Return" then
			local Ind = self:GetParent().MyTracker;
			Trackers[Ind].Config[MyVar] = self:GetText();
			self:SetKeyFocus(false);
			RefreshTrackersGlobal(Trackers[Ind].Config.TrackUnit);
		end
	end
	
	return TextField;
end

--Creates an editable text field that only supports numbers in the provided range
function CreateNumberEditText(TextID, Parent, MyVar, MinVal, MaxVal, SubVar)
	MinVal = MinVal or nil; --won't always have min or max values
	MaxVal = MaxVal or nil;
	SubVar = SubVar or nil;
	local TextField = UI.CreateFrame("RiftTextfield", tostring(TextID), Parent);
	TextField:SetBackgroundColor(0, 0, 0, 1);
	Library.LibSimpleWidgets.SetBorder(TextField, GUIVals.BorderWidth, 0.5, 0.5, 0.5, 1);
	
	local VarStr = tostring(Trackers[Parent.MyTracker].Config[MyVar]);
	if SubVar ~= nil then
		VarStr = tostring(Trackers[Parent.MyTracker].Config[MyVar][SubVar]);
	end
	TextField:SetText(VarStr);
	TextField.MyVar = MyVar;
	
	function TextField.Event:KeyDown(button)
		if button == "Return" then
			local Ind = self:GetParent().MyTracker;
			local Tracker = Trackers[Ind];
			local Content = tonumber(self:GetText());
			local Value = Tracker.Config[MyVar];
			if SubVar ~= nil then
				Value = Tracker.Config[MyVar][SubVar];
			end
			
			if (Content == nil) or (InRange(Content, MinVal, MaxVal) == false) then
				self:SetText(tostring(Value));
			else
				if SubVar ~= nil then
					Tracker.Config[MyVar][SubVar] = Content;
				else
					Tracker.Config[MyVar] = Content;
				end
				self:SetKeyFocus(false);
				RefreshTrackersGlobal(Tracker.Config.TrackUnit);
			end
			
			Tracker:RefreshData();
		end
	end
	
	return TextField;
end

--Wrapper around RiftSlider creation, binds it to a tracker variable and gives
--it an editable text field to display the current value and give the user an
--alternative means of editing it
function CreateSlideBar(SliderID, Parent, MyVar, MinVal, MaxVal, SubVar)
	SubVar = SubVar or nil;
	local SlideBar = UI.CreateFrame("RiftSlider", SliderID, Parent);
	SlideBar:SetRange(MinVal, MaxVal);
	SlideBar:SetEnabled(true);
	
	local BarWidth = GUIVals.SlideBarWidth;
	local BarHeight = GUIVals.SlideBarHeight;
	
	SlideBar:SetWidth(BarWidth);
	SlideBar:SetHeight(BarHeight);
	SlideBar.MyVar = MyVar;
	
	SlideBar.TextBox = UI.CreateFrame("RiftTextfield", SliderID .. "_TextBox", SlideBar);
	SlideBar.TextBox:SetWidth(GUIVals.SliderBoxWidth);
	SlideBar.TextBox:SetHeight(GUIVals.SliderBoxHeight);
	SlideBar.TextBox:SetBackgroundColor(0, 0, 0, 1);
	Library.LibSimpleWidgets.SetBorder(SlideBar.TextBox, GUIVals.BorderWidth, 0.5, 0.5, 0.5, 1);
	SlideBar.TextBox:SetPoint("LEFTCENTER", SlideBar, "TOPRIGHT", 5, 0);
	
	local VarStr = tostring(Trackers[Parent.MyTracker].Config[MyVar]);
	if SubVar ~= nil then
		VarStr = tostring(Trackers[Parent.MyTracker].Config[MyVar][SubVar]);
	end
	SlideBar.TextBox:SetText(VarStr);
	SlideBar:SetPosition(tonumber(VarStr));
	
	
	function SlideBar.Event:SliderChange()
		local CurrentVal = tonumber(self:GetPosition());
		self:SetPosition(CurrentVal);
		self.TextBox:SetText(tostring(CurrentVal));
		local Tracker = Trackers[Parent.MyTracker];
		if SubVar ~= nil then
			Tracker.Config[MyVar][SubVar] = CurrentVal;
		else
			Tracker.Config[MyVar] = CurrentVal;
		end
		
		Tracker:RefreshData();
		
		if MyVar == "AltTex" then --fringe case for updating the alternative texture
			RefreshEditWindowIcon();
			Tracker:UpdateDisplayedTexture(Buff);
		end
	end
	
	function SlideBar.TextBox.Event:KeyDown(button)
		if button == "Return" then
			local CurrentVal = tonumber(self:GetText());
			local Slider = self:GetParent();
			local Tracker = Trackers[Slider:GetParent().MyTracker];
			
			Slider:SetPosition(CurrentVal);
			Slider.TextBox:SetKeyFocus(false);
			
			if SubVar ~= nil then
				Tracker.Config[MyVar][SubVar] = CurrentVal;
			else
				Tracker.Config[MyVar] = CurrentVal;
			end
			
			Tracker:RefreshData();
		end
	end
	
	function SlideBar:UpdateMe()
		local CurrentVal = nil;
		local Tracker = Trackers[Parent.MyTracker];
		
		if SubVar ~= nil then
			CurrentVal = Tracker.Config[MyVar][SubVar];
		else
			CurrentVal = Tracker.Config[MyVar];
		end
			
		self:SetPosition(CurrentVal);
		self.TextBox:SetKeyFocus(false);
		self.TextBox:SetText(tostring(CurrentVal));
	end
	
	return SlideBar;
end

--Wrapper around checkbox creation, binds a tracker parameter to it
function CreateCheckbox(BoxID, Parent, MyVar, SubVar)
	SubVar = SubVar or nil;
	local CheckBox = UI.CreateFrame("RiftCheckbox", BoxID, Parent);
	CheckBox:SetWidth(GUIVals.CheckboxSize);
	CheckBox:SetHeight(GUIVals.CheckboxSize);
	
	local TrackerInd = Parent.MyTracker;
	if TrackerInd == nil then --some checkboxes are nested, need their parent's parent
		TrackerInd = Parent:GetParent().MyTracker;
	end
	
	local Tracker = Trackers[TrackerInd];
	if SubVar ~= nil then
		CheckBox:SetChecked(Tracker.Config[MyVar][SubVar]);
	else
		CheckBox:SetChecked(Tracker.Config[MyVar]);
	end
	
	function CheckBox.Event:CheckboxChange()
		local CurrentVal = self:GetChecked();
		local TrackerInd = Parent.MyTracker;
		
		if TrackerInd == nil then
			TrackerInd = Parent:GetParent().MyTracker;
		end
		
		local Tracker = Trackers[TrackerInd];
		
		if SubVar ~= nil then
			Tracker.Config[MyVar][SubVar] = CurrentVal;
		else
			Tracker.Config[MyVar] = CurrentVal;
		end
		
		Tracker:RefreshData();
	end
	return CheckBox;
end

--Wrapper around Radio Group creation, binds a tracker to it, gives it a handler function
function CreateRadioGroup(ID, Parent, MyVar)
	local RadioGroup = Library.LibSimpleWidgets.RadioButtonGroup(ID);
	local TrackerInd = Parent.MyTracker;
	--RadioGroup.MyVar = MyVar;
	
	--Need to find and select the button that should currently be selected
	
	RadioGroup.Event.RadioButtonChange = 
	function(self)
		local TrackerInd = Parent.MyTracker;
		local Tracker = Trackers[TrackerInd];
		Tracker.Config[MyVar] = self:GetSelectedRadioButton():GetText();
	end
	
	return RadioGroup;
end

--Wrapper around radio button creation
function CreateRadioButton(ID, Parent, Text)
	local RadioButton = UI.CreateFrame("SimpleRadioButton", ID, Parent);
	RadioButton:SetText(Text);
	RadioButton:SetWidth(GUIVals.CheckboxSize);
	RadioButton:SetHeight(GUIVals.CheckboxSize);
	return RadioButton;
end

--Creates a radio group and all its buttons from a layout table
function RadioGroupFromLayout(ID, Parent, Layout)
	if Layout == nil then
		return nil;
	end
	
	local RadioGroup = CreateRadioGroup(ID, Parent, Layout.MyVar);
	local Buttons = Layout.Buttons;
	RadioGroup.Buttons = {};
	
	local TrackerInd = Parent.MyTracker;
	local Tracker = Trackers[TrackerInd];
	
	local ButtonInd = next(Buttons);
	
	while ButtonInd ~= nil do
		local Button = Buttons[ButtonInd];
		RadioGroup.Buttons[ButtonInd] = CreateRadioButton(ID .. tostring(ButtonInd), Parent, Button.Text);
		RadioGroup.Buttons[ButtonInd]:SetPoint("TOPLEFT", Parent, "TOPLEFT", Button.Pos.x, Button.Pos.y);
		RadioGroup:AddRadioButton(RadioGroup.Buttons[ButtonInd]);
		
		if Tracker.Config[Layout.MyVar] == Button.Text then
			RadioGroup.Buttons[ButtonInd]:SetSelected(true);
		end
		
		ButtonInd = next(Buttons, ButtonInd);
	end
	
	return RadioGroup;
end

function NumberEditTextsFromLayout(ID, Layout, Parent)
	if Layout == nil then
		return nil; --this function may be called in creating panels without edit texts
	end
	
	local NumberEditTexts = {};
	--local TextEdits = Layout.EditTextLayout;
	local TextInd = next(Layout);
	
	while TextInd ~= nil do
		local Editor = Layout[TextInd];
		local MinVal = nil;
		local MaxVal = nil;
		
		if Editor.MinVal ~= nil then
			MinVal = Editor.MinVal;
		elseif Editor.MaxVal ~= nil then
			MaxVal = Editor.MaxVal;
		end
		
		NumberEditTexts[TextInd] = CreateNumberEditText(ID .. tostring(TextInd), Parent, Editor.MyVar, MinVal, MaxVal);
		NumberEditTexts[TextInd]:SetPoint("TOPLEFT", Parent, "TOPLEFT", Editor.Pos.x, Editor.Pos.y);
		NumberEditTexts[TextInd]:SetWidth(GUIVals.SliderBoxWidth + 40);
		NumberEditTexts[TextInd]:SetHeight(GUIVals.SliderBoxHeight);
		TextInd = next(Layout, TextInd);
	end
	--CreateNumberEditText(TextID, Parent, MyVar, MinVal, MaxVal, SubVar)
	return NumberEditTexts;
end

--Creates all the text titles for an edit window from a layout table
function TitleFramesFromLayout(ID, Layout, Parent)
	if Layout == nil then
		return nil; --this function may be called in creating panels without titles
	end
	
	local TitlesTable = {};
	local TitleInd = next(Layout);
	
	while TitleInd ~= nil do
		local Title = Layout[TitleInd];
		local XVal = 0;
		if (Title ~= nil) and (Title.x ~= nil) then
			XVal = Title.x
		end
		
		TitlesTable[TitleInd] = CreateTitleFrame(ID, Parent, Title.Text, GUIVals.TitleSize);
		TitlesTable[TitleInd]:SetPoint("TOPLEFT", Parent, "TOPLEFT", XVal, Title.y);
		
		TitleInd = next(Layout, TitleInd);
	end
	
	return TitlesTable;
end

--Creates all the slider bars for an edit window from a layout table
function SliderBarsFromLayout(ID, Layout, Parent, Titles)
	if Layout == nil then
		return nil; --this function may be called in creating panels without slider bars
	end
	
	local SlidersTable = {};
	local SliderInd = next(Layout);
	
	while SliderInd ~= nil do
		local Slider = Layout[SliderInd];
		local SliderID = ID .. "_Slider_" .. tostring(SliderInd);
		
		SlidersTable[SliderInd] = CreateSlideBar(SliderID, Parent, Slider.MyVar, Slider.MinVal, Slider.MaxVal, Slider.SubVar);
		SlidersTable[SliderInd]:SetPoint("TOPLEFT", Titles[SliderInd], "BOTTOMLEFT", GUIVals.XPadLeft, 0);
		
		SliderInd = next(Layout, SliderInd);
	end
	
	return SlidersTable;
end

--Creates all the checkboxes for an edit window from a layout table
function CheckboxesFromLayout(ID, Layout, Parent, Side)
	if Layout == nil then
		return nil; --this function may be called in creating panels without checkboxes
	end
	
	local CheckboxTable = {};
	local CheckboxInd = next(Layout);
	
	while CheckboxInd ~= nil do
		local Box = Layout[CheckboxInd];
		local TextID = ID .. "_Text_" .. tostring(CheckboxInd);
		
		CheckboxTable[CheckboxInd] = CreateTitleFrame(TextID, Parent, Box.Text, GUIVals.TitleSize);
		
		local XVal = 0; --error checking for optional layout parameters
		if (Box ~= nil) and (Box.x ~= nil) then
			XVal = Box.x;
		end
		
		CheckboxTable[CheckboxInd].CheckBox = CreateCheckbox(TextID .. "_Checkbox", Parent, Box.MyVar, Box.SubVar);
		AttachCheckboxToSide(CheckboxTable[CheckboxInd], CheckboxTable[CheckboxInd].CheckBox, XVal, Box.y, Side);
		
		if (Box ~= nil) and (Box.Tooltip ~= nil) then
			AddTooltipToFrame(CheckboxTable[CheckboxInd], ID, Box.Tooltip);
		end
		
		CheckboxInd = next(Layout, CheckboxInd);
	end
	
	return CheckboxTable;
end

--Attaches the Checkbox to the Title and positions them based on the side:
--LEFT attaches the left side of the title to the left side of the parent,
--RIGHT attaches the right side of the checkbox to the right side of the parent
function AttachCheckboxToSide(Title, CheckBox, X, Y, Side)
	if Side == "LEFT" then
		CheckBox:SetPoint("LEFTCENTER", Title, "RIGHTCENTER");
		Title:SetPoint("TOPLEFT", Title:GetParent(), "TOPLEFT", X, Y);
	elseif Side == "RIGHT" then
		CheckBox:SetPoint("TOPRIGHT", Title:GetParent(), "TOPRIGHT", X, Y);
		Title:SetPoint("RIGHTCENTER", CheckBox, "LEFTCENTER");
	end
end

--Creates a Tabview frame
function CreateTabview(Parent, ID)
	local Tabview = UI.CreateFrame("SimpleTabView", ID .. "_TabView", Parent);
	Tabview:SetTabPosition("top");
	Tabview:SetPoint("TOPLEFT", Parent, "TOPLEFT", GUIVals.XPadLeft, GUIVals.FirstPanelY);
	Tabview:SetWidth(GUIVals.PanelWidth);
	Tabview:SetHeight(GUIVals.PanelHeight + GUIVals.TabviewPadding); --add extra height to account for tabs
	Tabview:SetFontSize(GUIVals.TitleSize);
	return Tabview;
end

--Creates an edit panel for the tabview of an edit window
function CreatePanel(PanelID, Parent, Panel)
	local EditPanel = UI.CreateFrame("Frame", PanelID, Parent);
	EditPanel:SetLayer(Parent:GetLayer() + 1);
	EditPanel.MyTracker = Parent.MyTracker;
	EditPanel:SetWidth(Panel.Width);
	EditPanel:SetHeight(Panel.Height);
	EditPanel:SetPoint("TOPLEFT", Parent, "TOPLEFT", Panel.Pos.x, Panel.Pos.y);
	Library.LibSimpleWidgets.SetBorder(EditPanel, GUIVals.BorderWidth, 0.5, 0.5, 0.5, 1);
	return EditPanel;
end

--Adds a title to a tabview's edit panel
function CreatePanelTitle(Panel, PanelID, Title)
	local Title = CreateBorderedTitleFrame(PanelID, Panel, Title, GUIVals.PanelTitleSize);
	Title:SetWidth(GUIVals.PanelTitleWidth);
	Title:SetPoint("TOPLEFT", Panel,"TOPLEFT");
	return Title;
end

--Adds a tooltip to the given title
function AddTooltipToFrame(Frame, ID, Tooltip)
	Frame.Tooltip = UI.CreateFrame("SimpleTooltip", ID .. "_Tooltip", context)
	Frame.Tooltip:SetFontSize(GUIVals.TitleSize);
	Frame.Tooltip:InjectEvents(Frame, function() return Tooltip end);
end

--Adds an alpha editor to a tabview's edit panel
function AddAlphaEditorToPanel(Panel, PanelID, AlphaData)
	Panel.AlphaEditor = CreateNumberEditText(PanelID .. "AlphaEditor", Panel, AlphaData.MyVar, 0, 1, AlphaData.SubVar);
	Panel.AlphaEditor:SetWidth(GUIVals.AlphaEditBoxWidth);
	Panel.AlphaEditor:SetHeight(GUIVals.AlphaEditBoxHeight);
	Panel.AlphaEditor:SetPoint("TOPLEFT", Panel,"TOPLEFT", AlphaData.x, AlphaData.y);
	
	Panel.AlphaTitle = CreateTitleFrame(PanelID, Panel, AlphaData.Title, GUIVals.TitleSize);
	Panel.AlphaTitle:SetPoint("RIGHTCENTER", Panel.AlphaEditor, "LEFTCENTER");
	
	AddTooltipToFrame(Panel.AlphaTitle, PanelID, AlphaData.Tooltip);
end

--Adds a color editor to a tabview's edit panel
function AddColorEditorToPanel(Panel, PanelID, ColorData)
	Panel.ColorEditor = CreateColorFrame(PanelID .. "ColorEditor", Panel, ColorData.MyVar);
	Panel.ColorEditor:SetPoint("TOPLEFT", Panel,"TOPLEFT", ColorData.x, ColorData.y);
	
	Panel.ColorTitle = CreateTitleFrame(PanelID, Panel, ColorData.Title, GUIVals.TitleSize);
	Panel.ColorTitle:SetPoint("RIGHTCENTER", Panel.ColorEditor, "LEFTCENTER");
	
	AddTooltipToFrame(Panel.ColorTitle, PanelID, ColorData.Tooltip);
end

--Creates all the edit panels for the tabview of an edit window from a layout table
function EditPanelsFromLayout(ID, Layout, Parent, Tabview)
	local PanelsTable = {};
	local PanelInd = next(Layout);
	
	Parent[Tabview] = CreateTabview(Parent, ID);
	
	while PanelInd ~= nil do
		local Panel = Layout[PanelInd];
		local PanelID = ID .. "_" .. tostring(PanelInd);
		PanelsTable[PanelInd] = CreatePanel(PanelID, Parent, Panel);
		PanelsTable[PanelInd].Title = CreatePanelTitle(PanelsTable[PanelInd], PanelID, Panel.Title);
		PanelsTable[PanelInd].TextTitles = TitleFramesFromLayout(ID, Panel.TitleLayout, PanelsTable[PanelInd]);
		PanelsTable[PanelInd].SliderBars = SliderBarsFromLayout(ID, Panel.SliderLayout, PanelsTable[PanelInd], PanelsTable[PanelInd].TextTitles);
		PanelsTable[PanelInd].Checkboxes = CheckboxesFromLayout(ID, Panel.CheckboxLayout, PanelsTable[PanelInd], "RIGHT");
		PanelsTable[PanelInd].RadioGroup = RadioGroupFromLayout(ID, PanelsTable[PanelInd], Panel.RadioGroupLayout);
		PanelsTable[PanelInd].EditTexts = NumberEditTextsFromLayout(ID, Panel.EditTextLayout, PanelsTable[PanelInd]);
		
		if Panel.ColorSection ~= nil then --optional color selection
			local AlphaData = Panel.ColorSection.AlphaEdit;
			AddAlphaEditorToPanel(PanelsTable[PanelInd], PanelID, AlphaData);
			
			local ColorData = Panel.ColorSection.ColorEdit;
			if ColorData ~= nil then
				AddColorEditorToPanel(PanelsTable[PanelInd], PanelID, ColorData);
			end
		end
		
		PanelsTable[PanelInd]:SetVisible(false);
		Parent[Tabview]:AddTab(Panel.Title, PanelsTable[PanelInd]);
		Parent[Tabview]:SetLayer(0);
		PanelInd = next(Layout, PanelInd);
	end
	
	return PanelsTable;
end

--Wrapper around dropdown menu selector creation, binds a variable to it and
--attaches lists of items and values to it
function CreateSelectList(Title, Parent, MyVar, Items, Values)
	local SelectList  = Library.LibSimpleWidgets.Select(Title, Parent);
	SelectList.MyVar = MyVar;
	SelectList:SetItems(Items, Values);
	SelectList:SetFontSize(GUIVals.TitleSize);
	
	local CurrentVal = Trackers[Parent.MyTracker].Config[MyVar];
	SelectList:SetSelectedValue(CurrentVal);
	SelectList:SetLayer(Parent:GetLayer() + 1);
	
	function SelectList.Event:ItemSelect(item, value, index)
		Trackers[self:GetParent().MyTracker].Config[MyVar] = value;
		local CurrentVal = Trackers[Parent.MyTracker].Config[MyVar];
		self:SetSelectedValue(CurrentVal);
	end
	
	return SelectList;
end

--Creates all the dropdown menus for an edit window from a layout table
function SelectListsFromLayout(ID, Layout, Parent)
	local ListsTable = {};
	local ListInd = next(Layout);
	
	while ListInd ~= nil do
		local List = Layout[ListInd];
		
		ListsTable[ListInd] = CreateSelectList(List.Title, Parent, List.MyVar, List.Items, List.Values);
		ListsTable[ListInd].Title = CreateBorderedTitleFrame(ID, Parent, List.Title, GUIVals.PanelTitleSize);
		ListsTable[ListInd].Title:SetPoint("TOPLEFT", Parent, "TOPLEFT", List.Pos.x, List.Pos.y);
		ListsTable[ListInd]:SetPoint("TOPLEFT", ListsTable[ListInd].Title, "BOTTOMLEFT");
		
		ListInd = next(Layout, ListInd);
	end
	
	return ListsTable;
end

--Creates a title frame
function CreateTitleFrame(ID, Parent, Title, Size)
	local TitleFrame = UI.CreateFrame("Text", ID .. Title, Parent);
	TitleFrame:SetText(Title);
	TitleFrame:SetFontSize(Size);
	return TitleFrame;
end

--Creates a frame intended to show a large chunk of text
function CreateTextFrame(ID, Parent, Text, Size, Height, Width)
	Width = Width or Height; --not all text frames will need a width
	local TextFrame = CreateTitleFrame(ID, Parent, Text, Size);
	TextFrame:SetWordwrap(true);
	TextFrame:SetHeight(Height);
	TextFrame:SetWidth(Width);
	return TextFrame;
end

--Creates a title frame with a border
function CreateBorderedTitleFrame(ID, Parent, Title, Size)
	local TitleFrame = CreateTitleFrame(ID, Parent, Title, Size);
	Library.LibSimpleWidgets.SetBorder(TitleFrame, GUIVals.BorderWidth, 0.5, 0.5, 0.5, 1);
	return TitleFrame;
end

--Creates a title frame and gives it a tooltip
function CreateTitleWithTooltip(ID, Parent, Title, X, Y, Tooltip)
	local TitleFrame = CreateBorderedTitleFrame(ID, Parent, Title, GUIVals.PanelTitleSize);
	TitleFrame:SetPoint("TOPLEFT", Parent, "TOPLEFT", X, Y);
	AddTooltipToFrame(TitleFrame, ID, Tooltip);
	return TitleFrame;
end

--Creates all the title framess with tooltips for an edit window from a layout table
function TitlesWithTooltipsFromLayout(ID, Parent, Layout)
	local TitleFrames = {};
	local TitleInd = next(Layout);
	
	while TitleInd ~= nil do
		local TitleFrame = Layout[TitleInd];
		TitleFrames[TitleInd] = CreateTitleWithTooltip(ID, Parent, TitleFrame.Title, TitleFrame.x, TitleFrame.y, TitleFrame.Tooltip);
		TitleInd = next(Layout, TitleInd);
	end
	
	return TitleFrames;
end

--------------------Button Creation Functions-----------------------------------

--Creates a Rift Button
function CreateButton(ID, Parent, Title)
	local Button = UI.CreateFrame("RiftButton", ID, Parent);
	Button:SetWidth(GUIVals.ButtonWidth);
	Button:SetHeight(GUIVals.ButtonHeight);
	Button:SetText(Title);
	return Button;
end

--Creates a button that opens up the edit window for its parent tracker
function CreateEditButton(ID, Parent)
	local EditButton = CreateButton(ID, Parent, "Edit");
	AddTooltipToFrame(EditButton, ID, MyTooltips.EditbuttonTooltip);
	
	function EditButton.Event:LeftClick()
		if FrameEditWindow ~= nil then
			FrameEditWindow:CloseMe();
			FrameEditWindow = nil;
		end
		
		local TrackerInd = self:GetParent().TrackerInd;
		FrameEditWindow = CreateEditWindow(TrackerInd);
		self.Tooltip:SetVisible(false);
	end
	
	return EditButton;
end

--Creates a button that places its parent tracker into test mode
function CreateTestButton(ID, Parent)
	local TestButton = CreateButton(ID, Parent, "Test");
	AddTooltipToFrame(TestButton, ID, MyTooltips.TestButtonTooltip);
	
	function TestButton.Event:LeftClick()
		local TrackerInd = self:GetParent().TrackerInd;
		if TrackerInd == nil then
			TrackerInd = self:GetParent().MyTracker;
		end
		local Test = Trackers[TrackerInd].TestMode;
		Trackers[TrackerInd]:SetTestMode(not Test);
		self.Tooltip:SetVisible(false);
	end
	
	return TestButton;
end

--Creates a button that places all trackers into test mode
function CreateTestAllButton(ID, Parent)
	local TestButton = CreateButton(ID, Parent, "Test All");
	AddTooltipToFrame(TestButton, ID, MyTooltips.TestAllTooltip);
	
	function TestButton.Event:LeftClick()
		TestAllTrackers();
	end
	
	return TestButton;
end

--Creates a button that creates a new tracker
function CreateTrackerButton(ID, Parent)
	local TrackButton = CreateButton(ID, Parent, "New Tracker");
	AddTooltipToFrame(TrackButton, ID, MyTooltips.NewButtonTooltip);
	
	function TrackButton.Event:LeftClick()
		local TrackerNum = GetTrackerCount() + 1;
		local TrackerInd = "Tracker_" .. tostring(TrackerNum);
		
		while Trackers[TrackerInd] ~= nil do  --find an untaken slot for this tracker
			TrackerNum = TrackerNum + 1;
			TrackerInd = "Tracker_" .. tostring(TrackerNum);
		end
		local DefaultTable = GetDefaultTracker();
		Trackers[TrackerInd] = CreateTracker(DefaultTable, TrackerInd);
		MainGui:DeselectFrame();
		MainGui:RefreshList(TrackerInd);
		self.Tooltip:SetVisible(false);
	end
	
	return TrackButton;
end

--Creates a button that deletes its parent tracker
function CreateDeleteButton(ID, Parent)
	local DeleteButton = CreateButton(ID, Parent, "Delete");
	
	function DeleteButton.Event:LeftClick()
		local TrackerInd = self:GetParent().MyTracker;
		
		Trackers[TrackerInd]:SetVisible(false);
		if SelectedTrackFrame ~= nil then
			SelectedTrackFrame:SetBackgroundColor(0, 0, 0);
			SelectedTrackFrame.EditButton:SetVisible(false);
			SelectedTrackFrame.TestButton:SetVisible(false);
		end
		if FrameEditWindow ~= nil then
			FrameEditWindow:CloseMe();
			FrameEditWindow = nil;
		end
		Trackers[TrackerInd] = nil;
		UpdateTrackerLists();
		RefreshCooldownList();
		MainGui:RefreshList();
	end
	
	return DeleteButton;
end

--Creates a button that opens the help window
function CreateHelpButton(ID, Parent)
	local HelpButton = CreateButton(ID, Parent, "Help");
	AddTooltipToFrame(HelpButton, ID, MyTooltips.HelpButtonTooltip);
	
	function HelpButton.Event:LeftClick()
		if HelpWindow == nil then
			CreateHelpWindow();
		end
	end
	
	return HelpButton;
end

--------------------------------------------------------------------------------

-------------------Help Window Functions----------------------------------------

--Initializes the size, position, and title of the Help Window
function InitializeHelpWindow(HelpWindow)
	HelpWindow:SetTitle("Help");
	HelpWindow:SetWidth(GUIVals.EditorWidth);
	HelpWindow:SetHeight(GUIVals.EditorHeight);
	HelpWindow:SetPoint("TOPCENTER", UIParent, "TOPCENTER", 0, 50);
	HelpWindow:SetVisible(true);
	HelpWindow:SetCloseButtonVisible(true);
end

--Sets up the scrolling text field for the help window
function SetupHelpWindowScrollView(HelpWindow, WindowID)
	HelpWindow.ScrollView = CreateScrollView(WindowID, HelpWindow, GUIVals.HelpWidth, GUIVals.HelpHeight);
	HelpWindow.ScrollView:SetPoint("TOPCENTER", HelpWindow, "TOPCENTER", 0, 60);
	HelpWindow.HelpText = CreateTextFrame(WindowID .. "_HelpText", HelpWindow.ScrollView, 
	                                      HelpWindowContent, GUIVals.TitleSize + 2, 
	                                      GUIVals.EditorHeight * 3.3);
	HelpWindow.HelpText:SetBackgroundColor(0, 0, 0, 1);
	HelpWindow.ScrollView:SetContent(HelpWindow.HelpText);
end

--Opens the help window
function CreateHelpWindow()
	if HelpWindow ~= nil then
		return;
	end
	
	local WindowID = "HelpWindow";
	HelpWindow = UI.CreateFrame("SimpleWindow", WindowID, context);
	InitializeHelpWindow(HelpWindow);
	SetupHelpWindowScrollView(HelpWindow, WindowID);
	
	function HelpWindow.Event:Close()
		self:SetVisible(false);
		HelpWindow = nil;
	end
end

--------------------------------------------------------------------------------

-----------------------Edit Window Functions------------------------------------

--Initialized the Edit Windows position, size, title, layer, and other default values
function InitializeEditWindow(EditWindow, TrackerInd)
	EditWindow:SetTitle("Edit Tracker");
	EditWindow:SetWidth(GUIVals.EditorWidth);
	EditWindow:SetHeight(GUIVals.EditorHeight);
	EditWindow:SetLayer(MainGui:GetLayer() + 1);
	EditWindow:SetPoint("BOTTOMLEFT", UIParent, "BOTTOMLEFT", 0, -100);
	
	EditWindow:SetVisible(true);
	EditWindow:SetCloseButtonVisible(true);
	
	EditWindow.MyTracker = TrackerInd;
	EditWindow.TextFields = {};
end

--Adds the buttons to the editor window
function AddButtonsToEditWindow(EditWindow, WindowID)
	EditWindow.TestButton = CreateTestButton(WindowID .. "TestButton", EditWindow);
	EditWindow.TestButton:SetPoint("TOPCENTER", EditWindow, "TOPCENTER", 0, 40);
	
	EditWindow.DeleteButton = CreateDeleteButton(WindowID .. "DeleteButton", EditWindow);
	EditWindow.DeleteButton:SetPoint("BOTTOMRIGHT", EditWindow, "BOTTOMRIGHT", -10, -10);
end

--Sets up the tabview and edit panels for the edit window
function SetupEditWindowTabview(EditWindow, WindowID)
	local RegularPanels = "Tabview";
	local ResourcePanels = "ResourceTabview";
	EditWindow.Panels = EditPanelsFromLayout(WindowID, EditWindowPanelsConfig, EditWindow, RegularPanels);
	EditWindow.ResourcePanels = EditPanelsFromLayout(WindowID, EditWindowResourcePanelsConfig, EditWindow, ResourcePanels);
	EditWindow[RegularPanels]:SetPoint("TOPLEFT", EditWindow, "TOPLEFT", GUIVals.XPadLeft, GUIVals.FirstPanelY);
	EditWindow[ResourcePanels]:SetPoint("TOPLEFT", EditWindow, "TOPLEFT", GUIVals.XPadLeft, GUIVals.FirstPanelY);
end

--Adds the various editing interfaces to the edit window
function AddEditorsToEditWindow(EditWindow, WindowID)
	EditWindow.SelectLists = SelectListsFromLayout(WindowID .. "SelectLists", EditWindowListsLayout, EditWindow);
	EditWindow.MiscBoxes = CheckboxesFromLayout(WindowID .. "_Boxes", EditWindowCheckboxes, EditWindow, "LEFT");
	EditWindow.MiscTitles = TitlesWithTooltipsFromLayout(WindowID, EditWindow, EditWindowTitlesLayout);
	
	EditWindow.SpellField = CreateEditText(WindowID .. "_SpellField", EditWindow, "SpellName");
	EditWindow.SpellField:SetPoint("TOPLEFT", EditWindow.MiscTitles.SpellName, "BOTTOMLEFT");
	
	EditWindow.ImgSlider = CreateSlideBar(WindowID.. "_ImgSlider", EditWindow, "AltTex", 1, TableSize(AlternateTextures));
	EditWindow.ImgSlider:SetPoint("TOPLEFT", EditWindow.MiscTitles.Image, "BOTTOMLEFT", GUIVals.XPadLeft, 10);
end

--Sets up the alternate image display on the edit window
function SetupEditWindowAltImg(EditWindow, WindowID, TrackerInd)
	local TexIndex = Trackers[TrackerInd].Config.AltTex;
	EditWindow.AltImg = UI.CreateFrame("Texture", WindowID .. "_AltImg", EditWindow);
	EditWindow.AltImg:SetWidth(GUIVals.PreviewIconSize);
	EditWindow.AltImg:SetHeight(GUIVals.PreviewIconSize);
	EditWindow.AltImg:SetTexture(AddonID, AlternateTextures[TexIndex]);
	EditWindow.AltImg:SetPoint("LEFTCENTER", EditWindow.MiscTitles.Image, "RIGHTCENTER", 220, 0);
end

--Creates a RiftWindow-style GUI to edit the specified Tracker
function CreateEditWindow(TrackerInd)
	if Trackers[TrackerInd] == nil then
		return nil;
	end
	
	local WindowID = tostring(TrackerInd) .. "_Options";
	local EditWindow = UI.CreateFrame("SimpleWindow", WindowID, context);
	InitializeEditWindow(EditWindow, TrackerInd);
	AddButtonsToEditWindow(EditWindow, WindowID);
	SetupEditWindowTabview(EditWindow, WindowID);
	AddEditorsToEditWindow(EditWindow, WindowID);
	SetupEditWindowAltImg(EditWindow, WindowID, TrackerInd);
	
	Trackers[TrackerInd]:SetTestMode(true);
	
	function EditWindow.Event:Close()
		self:SetVisible(false);
		FrameEditWindow = nil;
	end
	
	function EditWindow:CloseMe()
		self:SetVisible(false);
		FrameEditWindow = nil;
	end
	
	function EditWindow:Update()
		if IsResource(Trackers[self.MyTracker].Config.SpellType) then
			self.Tabview:SetVisible(false);
			self.ResourceTabview:SetVisible(true);
		else
			self.Tabview:SetVisible(true);
			self.ResourceTabview:SetVisible(false);
		end
	end
	
	function EditWindow:RefreshPanelSliders(PanelInd)
		local Sliders = self.Panels[PanelInd].SliderBars;
		local SliderInd = next(Sliders);
		
		while SliderInd ~= nil do
			Sliders[SliderInd]:UpdateMe();
			SliderInd = next(Sliders, SliderInd);
		end
	end

	return EditWindow;
end

--------------------------------------------------------------------------------

------------Tracker Icon Frame Functions----------------------------------------

--Initializes the Track Frames size, position, and tracker index values
function InitializeTrackFrame(TrackFrame, TrackerInd, PosX, PosY)
	TrackFrame:SetWidth(GUIVals.TrackFrameSize);
	TrackFrame:SetHeight(GUIVals.TrackFrameSize);
	TrackFrame:SetPoint("TOPLEFT", TrackFrame:GetParent(), "TOPLEFT", PosX, PosY);
	TrackFrame.TrackerInd = TrackerInd;
end

--Adds the icon to a tracker icon frame
function AddIconToTrackFrame(TrackFrame, FrameID, Parent, TrackerInd)
	local Tracker = Trackers[TrackerInd];
	local Texture = Tracker.Config.Texture;
	local TrackerIconSize = GUIVals.TrackFrameSize - (GUIVals.IconPadding * 2);
	TrackFrame.Icon = UI.CreateFrame("Texture", FrameID .. "_icon", Parent);
	TrackFrame.Icon:SetWidth(TrackerIconSize);
	TrackFrame.Icon:SetHeight(TrackerIconSize);
	TrackFrame.Icon:SetLayer(TrackFrame:GetLayer() + 1);
	TrackFrame.Icon:SetPoint("CENTER", TrackFrame, "CENTER");
	
	if Texture ~= nil then
		TrackFrame.Icon:SetTexture(Texture.Location, Texture.Name);
	else
		TrackFrame.Icon:SetTexture(AddonID, AlternateTextures[Tracker.Config.AltTex]);
	end
end

--Adds the buttons to the tracker icon frame
function AddButtonsToTrackFrame(TrackFrame, FrameID)
	TrackFrame.EditButton = CreateEditButton(FrameID .. "_EditButton", TrackFrame);
	TrackFrame.EditButton:SetPoint("TOPLEFT", TrackFrame, "TOPRIGHT");
	TrackFrame.EditButton:SetVisible(false);
	
	TrackFrame.TestButton = CreateTestButton(FrameID .. "_TestButton", TrackFrame);
	TrackFrame.TestButton:SetPoint("BOTTOMLEFT", TrackFrame, "BOTTOMRIGHT");
	TrackFrame.TestButton:SetVisible(false);
end

--Creates a frame to represent a tracker in the tracker list of the main gui
function CreateTrackerIconFrame(TrackerInd, ID, Parent, PosX, PosY)
	local FrameID = ID .. "_frame_" .. tostring(TrackerInd);
	local TrackFrame = UI.CreateFrame("Frame", FrameID, Parent);
	InitializeTrackFrame(TrackFrame, TrackerInd, PosX, PosY);
	AddIconToTrackFrame(TrackFrame, FrameID, Parent, TrackerInd);
	AddButtonsToTrackFrame(TrackFrame, FrameID);
	
	function TrackFrame.Event:LeftClick()
		if SelectedTrackFrame ~= nil then
			SelectedTrackFrame:SetBackgroundColor(0, 0, 0, 0);
			SelectedTrackFrame.EditButton:SetVisible(false);
			SelectedTrackFrame.TestButton:SetVisible(false);
		end
		
		self:SetBackgroundColor(0.5, 0.5, 0.5, 1);
		SelectedTrackFrame = self;
		SelectedTrackFrame.EditButton:SetVisible(true);
		SelectedTrackFrame.TestButton:SetVisible(true);
		
		if FrameEditWindow ~= nil then
			FrameEditWindow:CloseMe();
			FrameEditWindow = nil;
		end
	end
	
	return TrackFrame;
end

--------------------------------------------------------------------------------

--Creates the tracker list to be placed in the scroll field on the main gui
function CreateTrackerContentFrame(ID, Parent)
	local TrackerList = {};
	local TrackerCount = GetTrackerCount();
	local TrackerInd = next(Trackers);
	local IconSpacing = GUIVals.TrackFrameSize + GUIVals.IconPadding;
	
	local ScrollFrame = UI.CreateFrame("Frame", ID .. "_scrollframe", Parent);
	local FrameHeight = IconSpacing * TrackerCount;
	
	--make sure there is enough space for all the trackers, but dont make it so
	--short that the scroll bar disappears
	if FrameHeight <= GUIVals.TrackerListHeight then
		FrameHeight = GUIVals.TrackerListHeight + 1;
	end
	ScrollFrame:SetHeight(FrameHeight);
	ScrollFrame.TrackFrames = {};
	
	for i=1,TrackerCount do
		ScrollFrame.TrackFrames[TrackerInd] = CreateTrackerIconFrame(TrackerInd, ID, ScrollFrame, 0, IconSpacing * (i-1));
		TrackerInd = next(Trackers, TrackerInd);
	end
	
	return ScrollFrame;
end

--Creats a ScrollView frame
function CreateScrollView(ID, Parent, Width, Height)
	local ScrollView = UI.CreateFrame("SimpleScrollView", ID .. "_scrollview", Parent);
	ScrollView:SetBackgroundColor(0, 0, 0, 0.5);
	ScrollView:SetWidth(Width);
	ScrollView:SetHeight(Height);
	Library.LibSimpleWidgets.SetBorder(ScrollView, GUIVals.BorderWidth, 0.5, 0.5, 0.5, 1);
	return ScrollView;
end

--Creates the scrolling list of trackers for the main gui
function CreateTrackerScrollView(ID, Parent)
	local ScrollView = CreateScrollView(ID, Parent, GUIVals.TrackerListWidth, GUIVals.TrackerListHeight);
	local ScrollFrame = CreateTrackerContentFrame(ID, ScrollView);
	
	ScrollView:SetContent(ScrollFrame);
	ScrollView.ScrollFrame = ScrollFrame;
	
	return ScrollView;
end

----------------------Main GUI Functions----------------------------------------

--Initializes the Main GUI position, size, and title
function InitializeMainGUI(MainGUI)
	MainGUI:SetTitle("Spell Trackers");
	MainGUI:SetCloseButtonVisible(true);
	MainGUI:SetWidth(GUIVals.MainGuiWidth);
	MainGUI:SetHeight(GUIVals.MainGuiHeight);
	MainGUI:SetPoint("TOPLEFT", UIParent, "TOPLEFT", 20, 20);
end

--Adds the scrollview list of trackers to the Main GUI
function AddTrackerListToMainGui(MainGUI, WindowID)
	MainGUI.ScrollView = CreateTrackerScrollView(WindowID, MainGUI);
	MainGUI.ScrollView:SetPoint("CENTER", MainGUI, "CENTER", 0, -80);
	
	MainGUI.TrackersTitle = CreateBorderedTitleFrame(WindowID, MainGUI, "My Trackers", GUIVals.PanelTitleSize);
	MainGUI.TrackersTitle:SetPoint("BOTTOMLEFT", MainGUI.ScrollView, "TOPLEFT", 0, -1);
end

--Creates all the Main GUI's buttons and attaches them to it
function AddButtonsToMainGUI(MainGUI, WindowID)
	MainGUI.CreateButton = CreateTrackerButton(WindowID .. "_Createbutton", MainGUI);
	MainGUI.CreateButton:SetPoint("BOTTOMRIGHT", MainGUI.ScrollView, "TOPRIGHT", 0, 5);
	
	MainGUI.HelpButton = CreateHelpButton(WindowID .. "_Helpbutton", MainGUI);
	MainGUI.HelpButton:SetPoint("BOTTOMCENTER", MainGUI, "BOTTOMCENTER", 0, -GUIVals.XPadLeft);
	
	MainGUI.TestAllButton = CreateTestAllButton(WindowID .. "_TestAllbutton", MainGUI);
	MainGUI.TestAllButton:SetPoint("TOPRIGHT", MainGUI.ScrollView, "BOTTOMRIGHT", 0, 5);
end

--Adds a text block of information to the Main GUI
function AddInfoToMainGUI(MainGUI, WindowID)
	local TrackerCount = GetTrackerCount();
	local Info = "Click the New Tracker button to get started";
	if TrackerCount > 0 then
		Info = "Click one of the trackers in the list above to edit or test it, " 
		.. "or click the New Tracker button at the top to get started. "
		.. "For further instructions or more information, click the Help button below";
	end
	
	MainGUI.InfoText = CreateTextFrame(WindowID .. "_InfoText", MainGUI, Info, 
	                                   GUIVals.InfoTextFontSize, GUIVals.InfoTextHeight, 
	                                   GUIVals.InfoTextWidth);
	MainGUI.InfoText:SetPoint("TOPCENTER", MainGUI.ScrollView, "BOTTOMCENTER", 0, 60);
end

--Creates the main gui
function CreateMainGUI()
	local WindowID = "SpellTrackers_MainGUI" .. tostring(Inspect.Time.Frame());
	local MainGUI = UI.CreateFrame("SimpleWindow", WindowID, context);
	InitializeMainGUI(MainGUI);
	AddTrackerListToMainGui(MainGUI, WindowID); --must be called before creating buttons
	AddButtonsToMainGUI(MainGUI, WindowID);
	AddInfoToMainGUI(MainGUI, WindowID);
	
	function MainGUI:CloseMe()
		CloseColorPicker();
		self:SetVisible(false);
		MainGui = nil;
		if FrameEditWindow ~= nil then
			FrameEditWindow:CloseMe();
		end
		SelectedTrackFrame = nil;
	end
	
	function MainGUI.Event:Close()
		self:CloseMe();
		StopAllTestModes();
		HideAllTrackers();
		RefreshCooldownList();
		UpdateTrackerLists();
		
		ApplyInvertedTrackers();
		ApplyResourceTrackers();
		RefreshAllTrackers();
	end
	
	function MainGUI:RefreshList(TrackerInd)
		TrackerInd = TrackerInd or nil;
		local Left = self:GetLeft();
		local Top = self:GetTop();
		RefreshListHelper(Left, Top, TrackerInd);
	end
	
	function MainGUI:DeselectFrame()
	if SelectedTrackFrame ~= nil then
			SelectedTrackFrame:SetBackgroundColor(0, 0, 0, 0);
			SelectedTrackFrame.EditButton:SetVisible(false);
			SelectedTrackFrame.TestButton:SetVisible(false);
		end
	end
	
	function MainGUI:SelectFrame(Ind)
		local NewFrame = self.ScrollView.ScrollFrame.TrackFrames[Ind]
		if NewFrame ~= nil then
			self:DeselectFrame();
			NewFrame:SetBackgroundColor(0.5, 0.5, 0.5, 1);
			SelectedTrackFrame = NewFrame;
			SelectedTrackFrame.EditButton:SetVisible(true);
			SelectedTrackFrame.TestButton:SetVisible(true);
			
			if FrameEditWindow ~= nil then
				FrameEditWindow:CloseMe();
				FrameEditWindow = nil;
			end
		end
	end
	
	function MainGUI:EditFrame(Ind)
		local ThisFrame = self.ScrollView.ScrollFrame.TrackFrames[Ind];
		if ThisFrame ~= nil then
			if FrameEditWindow ~= nil then
				FrameEditWindow:CloseMe();
				FrameEditWindow = nil;
			end
			
			FrameEditWindow = CreateEditWindow(Ind);
		end
	end
	
	return MainGUI;
end

--------------------------------------------------------------------------------

--Helper function for refreshing the tracker list on the main gui
function RefreshListHelper(x, y, TrackerInd)
	MainGui:CloseMe();
	MainGuiRefresh.Refresh = true;
	MainGuiRefresh.TrackerInd = TrackerInd;
	MainGuiRefresh.x = x;
	MainGuiRefresh.y = y;
end

--Opens the main gui, bound to a slash command
function OpenMainGUI()
	if MainGui == nil then
		MainGui = CreateMainGUI();
		FrameEditWindow = nil;
		SelectedTrackFrame = nil;
	end
end

--Refreshes the icon representing a tracker on the main gui tracker list
function RefreshEditWindowIcon()
	if FrameEditWindow == nil then
		return;
	end
	
	local EditWindow = FrameEditWindow;
	
	local TexIndex = Trackers[EditWindow.MyTracker].Config.AltTex;
	Trackers[EditWindow.MyTracker]:UpdateDisplayedTexture(nil);
	EditWindow.AltImg:SetTexture(AddonID, AlternateTextures[TexIndex]);
	if Trackers[EditWindow.MyTracker].Config.UseSpellIcon == false then
		SelectedTrackFrame.Icon:SetTexture(AddonID, AlternateTextures[TexIndex]);
	end
end

--Refreshes the main gui, a workaround for a strange graphical bug
function RefreshMainGui()
	if MainGuiRefresh.Refresh == true then
		MainGuiRefresh.Refresh = false;
		OpenMainGUI();
		if MainGuiRefresh.TrackerInd ~= nil then
			MainGui:SelectFrame(MainGuiRefresh.TrackerInd);
			MainGui:EditFrame(MainGuiRefresh.TrackerInd);
			Trackers[MainGuiRefresh.TrackerInd]:SetTestMode(true);
			MainGuiRefresh.TrackerInd = nil;
		end
		MainGui:SetPoint("TOPLEFT", UIParent, "TOPLEFT", MainGuiRefresh.x, MainGuiRefresh.y);
	end
end

--Updates lists of trackers grouped by unit
function UpdateTrackerLists()
	ClearTrackerLists();
	
	local TrackerInd = next(Trackers);
	local CurrentRole = Inspect.TEMPORARY.Role();
	while TrackerInd ~= nil do
		local Tracker = Trackers[TrackerInd];
		
		if Tracker.Config.ShowInSpec[CurrentRole] == true then
			if Tracker.Config.SpellType ~= "Cooldown" then --cooldowns are in a special list
				if IsResource(Tracker.Config.SpellType) == true then
					Trackers_Resource[TrackerInd] = TrackerInd;
				end
				
				if Tracker.Config.TrackUnit == "Self" then
					Trackers_Player[TrackerInd] = TrackerInd;
				elseif Tracker.Config.TrackUnit == "Target" then
					Trackers_Target[TrackerInd] = TrackerInd;
				elseif Tracker.Config.TrackUnit == "Focus" then
					Trackers_Focus[TrackerInd] = TrackerInd;
				elseif Tracker.Config.TrackUnit == "Pet" then
					Trackers_Pet[TrackerInd] = TrackerInd;
				end
			end
			Trackers_Role[TrackerInd] = TrackerInd;
		end
		
		TrackerInd = next(Trackers, TrackerInd);
	end
end

--Clears all the trackers
function ClearTrackerLists()
	--Trackers_Player = nil;
	Trackers_Player = {}; --key and value is an index into the master tracker list
	--Trackers_Target = nil;
	Trackers_Target = {};
	--Trackers_Focus = nil;
	Trackers_Focus = {};
	--Trackers_Pet = nil;
	Trackers_Pet = {};
	--Trackers_Role = nil;
	Trackers_Role = {};
	Trackers_Resource = {};
end

function RefreshMajorUnit(Unit)
	local UnitDetail = FindUnitDetail(Unit);

	MajorUnits[Unit] = 
		{
			Name = UnitDetail.name,
			ID = UnitDetail.id,
			Health = UnitDetail.health,
			MaxHealth = UnitDetail.healthMax,
			Mana = UnitDetail.mana,
			MaxMana = UnitDetail.manaMax,
			Energy = UnitDetail.energy,
			MaxEnergy = UnitDetail.energyMax,
			Power = UnitDetail.power,
			MaxPower = UnitDetail.powerMax,
			Combo = UnitDetail.combo,
			Combat = UnitDetail.combat
		};
	if MajorUnits[Unit].Health == nil then
		--print("nil health");
	end
end

--Determines whether or not this is a resource type
function IsResource(Type)
	if Type == "Health" then
		return true;
	else
		return false;
	end
end

--Gets the value and percentage of the given resource on the given unit
function ResourceDetail(Unit, Type)
	--local UnitDetail = 1;--FindUnitDetail(Unit);
	
	--if UnitDetail ~= nil then
		local Value = 0;
		local Percentage = 0;
		if Type == "Health" then
			Value = MajorUnits[Unit].Health; --UnitDetail.health;
			if Value == nil then
				RefreshMajorUnit(Unit);
				Value = MajorUnits[Unit].Health;
				
				if Value == nil then
					print("still nil");
					Value = 0;
					Percentage = 0;
				end
			else
				if MajorUnits[Unit].MaxHealth == nil then
					MajorUnits[Unit].MaxHealth = Value;
				end
					Percentage = (Value / MajorUnits[Unit].MaxHealth) * 100;
				--(UnitDetail.health / UnitDetail.healthMax) * 100;
			end
		--elseif Type == "Mana" then
		end
		return Value, Percentage;
	--end
	
	--return 0, 0;
end

--Manage updates to the player's current role
function UpdateRole()
	local CurrentRole = Inspect.TEMPORARY.Role();
	if CurrentRole ~= PlayerRole then --role changed, hide current trackers and
		PlayerRole = CurrentRole;       --apply new ones
		UpdateTrackerLists();
		RefreshCooldownList();
		HideAllTrackers();
		ApplyInvertedTrackers();
		ApplyResourceTrackers();
	end
end

--Manage updates to the player's current combat status
function UpdateCombat()
	local CombatStatus = CharacterUnit.combat;
	CharacterUnit = Inspect.Unit.Detail("player");
	RefreshMajorUnit("Self");
	if CombatStatus ~= CharacterUnit.combat then
		if CombatStatus == true then
			HideCombatTrackers(); --exiting combat, hide combat trackers
		else
			RefreshCooldownList();
			ApplyCombatTrackers(); --entering combat, add valid combat trackers
		end
	end
end

--Manage updates to the player's current target
function UpdateTarget()
	local TargetID = Inspect.Unit.Lookup("player.target");
	if CurrentTarget ~= TargetID then --target changed, refresh all target trackers
		ShowNonPlayerTrackers(false, "Target"); --hide all current target trackers
		CurrentTarget = TargetID; --update target before refreshing trackers
		RefreshTrackers("Target"); --show all that should now be seen
	end
end

--Manage updates to the player's current focus target
function UpdateFocus()
	local FocusID = Inspect.Unit.Lookup("focus");
	if CurrentFocus ~= FocusID then --focus changed, refresh all focus trackers
		ShowNonPlayerTrackers(false, "Focus");
		CurrentFocus = FocusID;
		RefreshTrackers("Focus");
	end
end

--Manage changes to the player's pet
function UpdatePet()
	local PetID = Inspect.Unit.Lookup("player.pet");
	if CurrentPet ~= PetID then --pet changed, refresh all pet trackers
		ShowNonPlayerTrackers(false, "Pet");
		CurrentPet = PetID;
		RefreshTrackers("Pet");
	end
end

--Attempts to find the ID of one of the player's spells by its name
function FindSpellID(SpellName)
	MyAbilities = Inspect.Ability.New.List();
	local AbilInd = next(MyAbilities);
	
	while AbilInd ~= nil do
		local ThisAbil = Inspect.Ability.New.Detail(AbilInd);
		if ThisAbil.name == SpellName then
			return AbilInd;
		end
		AbilInd = next(MyAbilities, AbilInd);
	end
	
	return nil;
end

--Converts the TrackUnit format string into the format needed for Inspect functions
function TrackUnitToInspect(UnitName)
	if UnitName == "Self" then
		return "player";
	elseif UnitName == "Target" then
		return "player.target";
	elseif UnitName == "Focus" then
		return "focus";
	elseif UnitName == "Pet" then
		return "player.pet";
	end
end

--Unit lookup using the TrackUnit parameter format
function FindUnit(UnitName)
	return Inspect.Unit.Lookup(TrackUnitToInspect(UnitName));
end

--Unit detail lookup using the TrackUnit parameter format
function FindUnitDetail(UnitName)
	return Inspect.Unit.Detail(TrackUnitToInspect(UnitName));
end

--Returns the amount to multiply the text frame size by for a given value
function GetValueWidthModifier(Value)
	if InRange(Value, 100000000, 999999999) or InRange(Value, 100000, 999999) then
		return 3.7; --hundred thousands or hundred millions = XXX.Xk or XXX.Xm
	elseif InRange(Value, 10000000, 99999999) or InRange(Value, 10000, 99999) then
		return 3.2; --ten thousands or ten millions = XX.Xk or XX.Xm
	elseif Value >= 1000 then --anything above 9999 will be formatted differently
		return 2.7; --4 digits
	elseif Value >= 100 then
		return 2.0; --3 digits
	elseif Value >= 10 then
		return 1.3; --2 digits
	else
		return 1;
	end
end

--Returns the amount to multiply the text frame size by for a given percentage
function GetPercentWidthModifier(Percentage)
	if Percentage == 100 then
		return 4; --100.0%
	elseif Percentage >= 10 then
		return 3.2; --10.0% to 99.9%
	else
		return 2.7; --0.0% to 9.9%
	end
end

--Returns the amount to multiply the timer frame size by for a given remainder
function GetTimerWidthModifier(Remaining)
	if Remaining >= 3600 then --more than 1 hour
		return 4;
	elseif Remaining >= 60 then --more than 1 minute
		return 3;
	elseif Remaining >= 10 then --double digit seconds
		return 2.9;
	else --single digit seconds
		return 2.5;
	end
end

--Returns a formatted string to be used for a value
function GetValueFormattedString(Value)
	if Value >= 1000000 then
		return string.format("%.1f", (Value / 1000000)) .. "m";
	elseif Value >= 1000 then
		return string.format("%.1f", (Value / 1000)) .. "k";
	else
		return tostring(Value);
	end
end

--Returns a formatted string to be used for a percentage
function GetPercentageFormattedString(Percentage)
	return string.format("%d.%1d", math.floor(Percentage), Percentage % 1) .. "%";
end

--Formats the remaining time provided to be used for the timer
function GetTimerFormattedString(Remaining)
	if Remaining >= 3600 then --more than 1 hour
		return string.format("%d:%02d:%02d", math.floor(Remaining / 3600), math.floor(Remaining / 60) % 60, math.floor(Remaining) % 60);
	elseif Remaining >= 60 then --more than 1 minute
		return string.format("%d:%02d", math.floor(Remaining / 60), math.floor(Remaining) % 60);
	elseif Remaining >= 10 then --double digit seconds
		return tostring(Remaining - Remaining % 0.01);
	else --single digit seconds
		return tostring(Remaining - Remaining % 0.01);
	end
end

--Returns the formatted string to be used on a timer while in Test Mode
function GetTestModeTimerFormattedString(Tracker)
	if IsResource(Tracker.Config.SpellType) == true then
		if Tracker.Config.ShowPercent == true then
			return "999.9%"; --absurd values to make it clear this is a test
		else
			return "999.9m";
		end
	else
		return "9:99:99";
	end
end

--Refreshes the Icon data for the given tracker
function RefreshIconData(Tracker)
	Tracker.Icon:SetWidth(Tracker.Config.TrackerSize);
	Tracker.Icon:SetHeight(Tracker.Config.TrackerSize);
	Tracker.Icon:SetAlpha(Tracker.Config.TrackerAlpha);
end

--Refreshes the Timer data for the given tracker
function RefreshTimerData(Tracker)
	Tracker.Timer:SetWidth(Tracker.Config.TimerSize * 4);
	Tracker.Timer:SetHeight(Tracker.Config.TimerSize);
	Tracker.Timer:SetFontSize(Tracker.Config.TimerSize);
	
	local TimerColor = Tracker.Config.TimerColor;
	Tracker.Timer:SetFontColor(TimerColor.r, TimerColor.g, TimerColor.b, TimerColor.a);
	Tracker.Timer:SetAlpha(TimerColor.a);
	if Tracker.Config.ShowTimerBG == true then
		Tracker.Timer:SetBackgroundColor(0,0,0,Tracker.Config.TimerColor.a);
	else
		Tracker.Timer:SetBackgroundColor(0,0,0,0);
	end
	
	Tracker.Timer:SetPoint("TOPLEFT", UIParent, "TOPLEFT", Tracker.Config.TimerPos.x, Tracker.Config.TimerPos.y);
	Tracker:SetTimerVisibility(Tracker.Config.ShowTimer);
end

--Refreshes the Stacks data for the given tracker
function RefreshStacksData(Tracker)
	Tracker.Stacks:SetHeight(Tracker.Config.StacksSize);
	Tracker.Stacks:SetWidth(Tracker.Config.StacksSize);
	Tracker.Stacks:SetFontSize(Tracker.Config.StacksSize);
	
	local StacksColor = Tracker.Config.StacksColor;
	Tracker.Stacks:SetFontColor(StacksColor.r, StacksColor.g, StacksColor.b, StacksColor.a);
	Tracker.Stacks:SetAlpha(StacksColor.a);
	if Tracker.Config.ShowStacksBG == true then
		Tracker.Stacks:SetBackgroundColor(0,0,0,Tracker.Config.StacksColor.a);
	else
		Tracker.Stacks:SetBackgroundColor(0,0,0,0);
	end
	Tracker.Stacks:SetPoint("TOPLEFT", UIParent, "TOPLEFT", Tracker.Config.StacksPos.x, Tracker.Config.StacksPos.y);
	Tracker:SetStacksVisibility(Tracker.Config.ShowStacks);
end

--Refreshes the Test Mode settings for the given tracker
function RefreshTestModeSettings(Tracker)
	if Tracker.TestMode == true then
		if Tracker.Config.ShowStacks == true then
			Tracker.Stacks:SetText("9");
			Tracker.Stacks:SetVisible(true);
		end
		
		if Tracker.Config.ShowTimer then
			local FormattedText = GetTestModeTimerFormattedString(Tracker);
			Tracker.Timer:SetText(FormattedText);
			Tracker.Timer:SetVisible(true);
		end
	end
end

--Determines whether or not the values passes the threshold based on the operator
function EvaluateThreshold(Value, Threshold, Operator)
	if Operator == ">" then
		if Value > Threshold then
			return true;
		end
	elseif Operator == ">=" then
		if Value >= Threshold then
			return true;
		else
		end
	elseif Operator == "<" then
		if Value < Threshold then
			return true;
		end
	elseif Operator == "<=" then
		if Value <= Threshold then
			return true;
		end
	elseif Operator == "=" then
		if Value == Threshold then
			return true;
		end
	end
	return false;
end

--Picks which tracker list to iterate through based on tracked unit
function GetTrackerList(Tracked)
	if Tracked == "Target" then
		return Trackers_Target; --these lists contain indices to the master tracker list
	elseif Tracked == "Focus" then
		return Trackers_Focus;
	elseif Tracked == "Pet" then
		return Trackers_Pet;
	else
		return Trackers_Player;
	end
end

function RefreshMajorUnits()
	RefreshMajorUnit("Self");
	RefreshMajorUnit("Target");
	RefreshMajorUnit("Focus");
	RefreshMajorUnit("Pet");
end

table.insert(Command.Slash.Register("spt"), {OpenMainGUI, AddonID, "Slash Command"});