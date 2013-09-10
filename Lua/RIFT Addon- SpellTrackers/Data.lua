--[[
Random notes and TO-DO:
	*header for all files
	
	*review serialization processes
	 
	*implement resource tracking:
		-Health
		-Mana/Energy/Power
		-Combo Points/Attack Points (player only, probably the same thing)
		-Charge (player only)
	
	*implement import/export feature
	
	*test lots of things
	*make trackers always appear in the same order on the GUI
	*make spell name no longer be case-sensitive via uppercase string function
	 
	*make separate source file dedicated to GUI creation functions
	
	*magic numbers in Main GUI, edit window, and help window creation
	 
	*Optimize resource updates, probably need to perform an initial validation
	
	*Some issue with MaxHealth being nil on the first check, but works afterwards
	
	*Update function to editor panel or something, make sure user doesnt enter more
	 than 100 when using percentages
	
	*RemoveBuffHelper should probably be using AddMe(Buff) instead of SetVisible(true)
	
	*clean up CreateTracker
	
	
	
	buff
	debuff
	cooldown
	health
	mana/energy
	charge
	combo points
	
--]]


--Global Declarations
Addon, Shared = ...
AddonID = Addon.identifier;

Trackers = {};
CooldownList = {};
Trackers_Player = {};
Trackers_Target = {};
Trackers_Focus = {};
Trackers_Pet = {};
Trackers_Role = {};
Trackers_Resource = {};
MyAbilities = Inspect.Ability.New.List();

MainGuiRefresh = {
	Refresh = false,
	TrackerInd = nil,
	x = 0,
	y = 0
};

CharacterUnit = Inspect.Unit.Detail("player");
CharacterName = CharacterUnit.name;
PlayerID = Inspect.Unit.Lookup("player");
PlayerRole = Inspect.TEMPORARY.Role();

--experimental
MajorUnits = 
{
	["Self"] = 
	{
		Name = CharacterUnit.name,
		ID = CharacterUnit.id,
		Health = CharacterUnit.health,
		MaxHealth = CharacterUnit.healthMax,
		Mana = CharacterUnit.mana,
		MaxMana = CharacterUnit.manaMax,
		Energy = CharacterUnit.energy,
		MaxEnergy = CharacterUnit.energyMax,
		Power = CharacterUnit.power,
		MaxPower = CharacterUnit.powerMax,
		Combo = CharacterUnit.combo,
		Combat = CharacterUnit.combat
	},
};
--

CurrentTarget = nil;
CurrentFocus = nil;
CurrentPet = nil;

WindowLeft = math.floor(UIParent:GetLeft());
WindowRight = math.floor(UIParent:GetRight());
WindowTop = math.floor(UIParent:GetTop());
WindowBottom = math.floor(UIParent:GetBottom());

NextRefresh = Inspect.Time.Frame();
RefreshDelay = 0.3;

MyTooltips = {
TestButtonTooltip = "Enter test mode to preview this\neffect (Icon, Timer & Stacks are\ndraggable in test mode)",
InvertTooltip = "Show this effect when buff,\ndebuff, or cooldown is not active",
SpellNameTooltip = "Name of the spell to track with\nthis effect. This is case-sensitive",
MySpellsTooltip = "Check this box to only track\nbuffs and debuffs cast by you",
SpellImageTooltip = "Use the spell's image for the icon.\n(Defaults to the above image if not found)",
InCombatTooltip = "Hide this effect when out of combat",
TimerBGTooltip = "Gives the timer a background",
StacksBGTooltip = "Gives the stacks a background",
NewButtonTooltip = "Create a new Spell Tracker",
EditbuttonTooltip = "Open the Tracker Editor",
ImageTitleTooltip = "The image to display as the icon\nfor this effect",
RolesTooltip = "Only show this effect in the\nroles selected below",
TextColorTooltip = "Click the square to select\na color for the text",
TextAlphaTooltip = "0 = fully transparent,\n0.5 = 50% transparent,\n1 = fully opaque",
HelpButtonTooltip = "Instructions on how to\nconfigure Spell Trackers",
TestAllTooltip = "Places all of your trackers\ninto test mode",
ThresholdPercentTooltip = "Use percentage for threshold\ninstead of value",
};


GUIVals = {
	TrackerListWidth = 250,
	TrackerListHeight = 300,
	EditorWidth = 450,
	EditorHeight = 675,
	HelpWidth = 410,
	HelpHeight = 595,
	MainGuiWidth = 390,
	MainGuiHeight = 660,
	TitleSize = 14,
	PanelTitleSize = 20,
	XPadLeft = 20,
	BorderWidth = 1.5,
	SlideBarWidth = 200,
	SlideBarHeight = 10,
	PanelWidth = 400,
	PanelHeight = 200,
	PanelPadding = 15,
	TabviewPadding = 40,
	PanelTitleWidth = 90,
	CheckboxSize = 20,
	FirstPanelY = 290,
	IconSize = 80,
	TrackFrameSize = 80,
	IconPadding = 5,
	RoleSpacingX = 80,
	ButtonWidth = 120,
	ButtonHeight = 45,
	SliderBoxWidth = 40,
	SliderBoxHeight = 25,
	AlphaEditBoxWidth = 30,
	AlphaEditBoxHeight = 20,
	PreviewIconSize = 80,
	InfoTextWidth = 310,
	InfoTextHeight = 160,
	InfoTextFontSize = 16,
	ColorFrameSize = 20,
	ThresholdRBX = 150
};

SliderRanges = {
	TrackerMin = 20,
	TrackerMax = 300,
	TimerMin = 10,
	TimerMax = 100,
};

EditWindowPanelsConfig = {
	[1] = {  --IconPanel
		Title = "Icon",
		Width = GUIVals.PanelWidth,
		Height = GUIVals.PanelHeight,
		Pos = {
			x = GUIVals.XPadLeft,
			y = GUIVals.FirstPanelY,
		},
		TitleLayout = {
			Size = {
				Text = "Size",
				y = 70
			},
			PosX = {
				Text = "Position X",
				y = 110
			},
			PosY = {
				Text = "Position Y",
				y = 150
			}
		},
		SliderLayout = {
			Size = {
			MinVal = SliderRanges.TrackerMin,
			MaxVal = SliderRanges.TrackerMax,
			MyVar = "TrackerSize",
			SubVar = nil
			},
			PosX = {
			MinVal = WindowLeft,
			MaxVal = WindowRight,
			MyVar = "TrackerPos",
			SubVar = "x"
			},
			PosY = {
			MinVal = WindowTop,
			MaxVal = WindowBottom,
			MyVar = "TrackerPos",
			SubVar = "y"
			}
		},
		ColorSection = {
			AlphaEdit = {
				x = 200,
				y = 35,
				Tooltip = MyTooltips.TextAlphaTooltip,
				Title = "Transparency",
				MyVar = "TrackerAlpha",
				SubVar = nil
			}
		}
	},
	[2] = {  --TimerPanel
		Title = "Timer",
		Width = GUIVals.PanelWidth,
		Height = GUIVals.PanelHeight,
		Pos = {
			x = GUIVals.XPadLeft,
			y = GUIVals.FirstPanelY,
		},
		TitleLayout = {
			Size = {
				Text = "Size",
				y = 70
			},
			PosX = {
				Text = "Position X",
				y = 110
			},
			PosY = {
				Text = "Position Y",
				y = 150
			}
		},
		SliderLayout = {
			Size = {
			MinVal = SliderRanges.TimerMin,
			MaxVal = SliderRanges.TimerMax,
			MyVar = "TimerSize",
			SubVar = nil
			},
			PosX = {
			MinVal = WindowLeft,
			MaxVal = WindowRight,
			MyVar = "TimerPos",
			SubVar = "x"
			},
			PosY = {
			MinVal = WindowTop,
			MaxVal = WindowBottom,
			MyVar = "TimerPos",
			SubVar = "y"
			}
		},
		CheckboxLayout = {
			Display = {
				Text = "Show Timer",
				x = 0,
				y = 0,
				MyVar = "ShowTimer"
			},
			Background = {
				Text = "Show Background",
				x = 0,
				y = GUIVals.CheckboxSize,
				MyVar = "ShowTimerBG",
				Tooltip = MyTooltips.TimerBGTooltip
			},
		},
		ColorSection = {
			AlphaEdit = {
				x = 200,
				y = 35,
				Tooltip = MyTooltips.TextAlphaTooltip,
				Title = "Transparency",
				MyVar = "TimerColor",
				SubVar = "a"
			},
			ColorEdit = {
				x = 200,
				y = 5,
				Tooltip = MyTooltips.TextColorTooltip,
				Title = "Color",
				MyVar = "TimerColor"
			}
		}
	},
	[3] = {  --StacksPanel
		Title = "Stacks",
		Width = GUIVals.PanelWidth,
		Height = GUIVals.PanelHeight,
		Pos = {
			x = GUIVals.XPadLeft,
			y = GUIVals.FirstPanelY,
		},
		TitleLayout = {
			Size = {
				Text = "Size",
				y = 70
			},
			PosX = {
				Text = "Position X",
				y = 110
			},
			PosY = {
				Text = "Position Y",
				y = 150
			}
		},
		SliderLayout = {
			Size = {
			MinVal = SliderRanges.TimerMin,
			MaxVal = SliderRanges.TimerMax,
			MyVar = "StacksSize",
			SubVar = nil
			},
			PosX = {
			MinVal = WindowLeft,
			MaxVal = WindowRight,
			MyVar = "StacksPos",
			SubVar = "x"
			},
			PosY = {
			MinVal = WindowTop,
			MaxVal = WindowBottom,
			MyVar = "StacksPos",
			SubVar = "y"
			}
		},
		CheckboxLayout = {
			Display = {
				Text = "Show Stacks",
				y = 0,
				MyVar = "ShowStacks"
			},
			Background = {
				Text = "Show Background",
				y = GUIVals.CheckboxSize,
				MyVar = "ShowStacksBG",
				Tooltip = MyTooltips.StacksBGTooltip
			},
		},
		ColorSection = {
			AlphaEdit = {
				x = 200,
				y = 35,
				Tooltip = MyTooltips.TextAlphaTooltip,
				Title = "Transparency",
				MyVar = "StacksColor",
				SubVar = "a"
			},
			ColorEdit = {
				x = 200,
				y = 5,
				Tooltip = MyTooltips.TextColorTooltip,
				Title = "Color",
				MyVar = "StacksColor"
			}
		}
	}
};

EditWindowResourcePanelsConfig = 
{
	[1] = 
	{  --IconPanel
		Title = "Icon",
		Width = GUIVals.PanelWidth,
		Height = GUIVals.PanelHeight,
		Pos = 
		{
			x = GUIVals.XPadLeft,
			y = GUIVals.FirstPanelY,
		},
		TitleLayout = 
		{
			Size = 
			{
				Text = "Size",
				y = 70
			},
			PosX = 
			{
				Text = "Position X",
				y = 110
			},
			PosY = 
			{
				Text = "Position Y",
				y = 150
			}
		},
		SliderLayout = 
		{
			Size = 
			{
				MinVal = SliderRanges.TrackerMin,
				MaxVal = SliderRanges.TrackerMax,
				MyVar = "TrackerSize",
				SubVar = nil
			},
			PosX = 
			{
				MinVal = WindowLeft,
				MaxVal = WindowRight,
				MyVar = "TrackerPos",
				SubVar = "x"
			},
			PosY = 
			{
				MinVal = WindowTop,
				MaxVal = WindowBottom,
				MyVar = "TrackerPos",
				SubVar = "y"
			}
		},
		ColorSection = 
		{
			AlphaEdit = 
			{
				x = 200,
				y = 35,
				Tooltip = MyTooltips.TextAlphaTooltip,
				Title = "Transparency",
				MyVar = "TrackerAlpha",
				SubVar = nil
			}
		}
	},
	[2] = 
	{ --ValuePanel
		Title = "Value",
		Width = GUIVals.PanelWidth,
		Height = GUIVals.PanelHeight,
		Pos = 
		{
			x = GUIVals.XPadLeft,
			y = GUIVals.FirstPanelY,
		},
		TitleLayout = 
		{
			Size = 
			{
				Text = "Size",
				y = 70
			},
			PosX = 
			{
				Text = "Position X",
				y = 110
			},
			PosY = 
			{
				Text = "Position Y",
				y = 150
			}
		},
		SliderLayout = 
		{
			Size = 
			{
				MinVal = SliderRanges.TimerMin,
				MaxVal = SliderRanges.TimerMax,
				MyVar = "TimerSize",
				SubVar = nil
			},
			PosX = 
			{
				MinVal = WindowLeft,
				MaxVal = WindowRight,
				MyVar = "TimerPos",
				SubVar = "x"
			},
			PosY = 
			{
				MinVal = WindowTop,
				MaxVal = WindowBottom,
				MyVar = "TimerPos",
				SubVar = "y"
			}
		},
		CheckboxLayout = 
		{
			Display = 
			{
				Text = "Show Value",
				x = 0,
				y = 0,
				MyVar = "ShowTimer"
			},
			Percent = 
			{
				Text = "Show as Percentage",
				x = 0,
				y = GUIVals.CheckboxSize,
				MyVar = "ShowPercent"
			},
			Background = 
			{
				Text = "Show Background",
				x = 0,
				y = GUIVals.CheckboxSize * 2,
				MyVar = "ShowTimerBG",
				Tooltip = MyTooltips.TimerBGTooltip
			},
		},
		ColorSection = 
		{
			AlphaEdit = 
			{
				x = 200,
				y = 35,
				Tooltip = MyTooltips.TextAlphaTooltip,
				Title = "Transparency",
				MyVar = "TimerColor",
				SubVar = "a"
			},
			ColorEdit = 
			{
				x = 200,
				y = 5,
				Tooltip = MyTooltips.TextColorTooltip,
				Title = "Color",
				MyVar = "TimerColor"
			}
		}
	},
	[3] = 
	{  --ThresholdPanel
		Title = "Threshold",
		Width = GUIVals.PanelWidth,
		Height = GUIVals.PanelHeight,
		Pos = 
		{
			x = GUIVals.XPadLeft,
			y = GUIVals.FirstPanelY,
		},
		TitleLayout = 
		{
			Size = 
			{
				Text = "Value",
				y = 150
			},
		},
		EditTextLayout = 
		{
			MyVar = "ThresholdValue",
			Pos = 
			{
				x = 0,
				y = 0
			},
		},
		CheckboxLayout = 
		{
			Background = 
			{
				Text = "Percent",
				y = GUIVals.CheckboxSize,
				MyVar = "ThresholdPercent",
				Tooltip = MyTooltips.ThresholdPercentTooltip
			},
		},
		EditTextLayout = 
		{
			[1] = 
			{
				MyVar = "ThresholdValue",
				Pos = 
				{
					x = 200,
					y = 50
				},
			},
		},
		RadioGroupLayout = 
		{
			MyVar = "ThresholdOperator",
			Buttons = 
			{
				[1] = 
				{
					Text = ">",
					Pos = 
					{
						x = GUIVals.ThresholdRBX,
						y = 0,
					},
				},
				[2] = 
				{
					Text = ">=",
					Pos = 
					{
						x = GUIVals.ThresholdRBX,
						y = GUIVals.CheckboxSize,
					},
				},
				[3] = 
				{
					Text = "<",
					Pos = 
					{
						x = GUIVals.ThresholdRBX,
						y = GUIVals.CheckboxSize * 2,
					},
				},
				[4] = 
				{
					Text = "<=",
					Pos = 
					{
						x = GUIVals.ThresholdRBX,
						y = GUIVals.CheckboxSize * 3,
					},
				},
				[5] = 
				{
					Text = "=",
					Pos = 
					{
						x = GUIVals.ThresholdRBX,
						y = GUIVals.CheckboxSize * 4,
					},
				},
			},
		},
	},
	
};

EditWindowTitlesLayout = 
{
	SpellName = 
	{
		Title = "Spell Name",
		x = GUIVals.XPadLeft,
		y = 60,
		Tooltip = MyTooltips.SpellNameTooltip
	},
	Image = 
	{
		Title = "Image",
		x = GUIVals.XPadLeft,
		y = 200,
		Tooltip = MyTooltips.ImageTitleTooltip
	},
	Roles = 
	{
		Title = "Roles",
		x = GUIVals.XPadLeft,
		y = 545,
		Tooltip = MyTooltips.RolesTooltip
	}
}

EditWindowListsLayout = 
{
	SpellType = 
	{
		Title = "Spell Type",
		MyVar = "SpellType",
		Pos = 
		{
			x = GUIVals.XPadLeft,
			y = 130
		},
		Items = 
		{
			[1] = "Buff",
			[2] = "Debuff",
			[3] = "Cooldown",
			[4] = "Health"
		},
		Values = 
		{
			[1] = "Buff",
			[2] = "Debuff",
			[3] = "Cooldown",
			[4] = "Health"
		}
	},
	TrackUnit = 
	{
		Title = "Track Unit",
		MyVar = "TrackUnit",
		Pos = 
		{
			x = GUIVals.XPadLeft + 150,
			y = 130
		},
		Items = 
		{
			[1] = "Self",
			[2] = "Target",
			[3] = "Focus",
			[4] = "Pet"
		},
		Values = 
		{
			[1] = "Self",
			[2] = "Target",
			[3] = "Focus",
			[4] = "Pet"
		}
	}
};

EditWindowCheckboxes = 
{
	MySpells = 
	{
		Text = "Only Track My Spells",
		x = 280,
		y = 103,
		MyVar = "MySpells",
		SubVar = nil,
		Tooltip = MyTooltips.MySpellsTooltip
	},
	Invert = 
	{
		Text = "Invert Effect",
		x = 337,
		y = 123,
		MyVar = "Invert",
		SubVar = nil,
		Tooltip = MyTooltips.InvertTooltip
	},
	UseSpellIcon = 
	{
		Text = "Use Spell Image",
		x = GUIVals.XPadLeft,
		y = 260,
		MyVar = "UseSpellIcon",
		SubVar = nil,
		Tooltip = MyTooltips.SpellImageTooltip
	},
	OnlyInCombat = 
	{
		Text = "Only Show In Combat",
		x = 274,
		y = 83,
		MyVar = "OnlyInCombat",
		SubVar = nil,
		Tooltip = MyTooltips.InCombatTooltip
	},
	Role1 = 
	{
		Text = "Role 1",
		x = (GUIVals.XPadLeft * 2),
		y = 580,
		MyVar = "ShowInSpec",
		SubVar = 1
	},
	Role2 = 
	{
		Text = "Role 2",
		x = (GUIVals.XPadLeft * 2),
		y = 600,
		MyVar = "ShowInSpec",
		SubVar = 2
	},
	Role3 = 
	{
		Text = "Role 3",
		x = (GUIVals.XPadLeft * 2),
		y = 620,
		MyVar = "ShowInSpec",
		SubVar = 3
	},
	Role4 = 
	{
		Text = "Role 4",
		x = (GUIVals.XPadLeft * 2) + GUIVals.RoleSpacingX,
		y = 580,
		MyVar = "ShowInSpec",
		SubVar = 4
	},
	Role5 = 
	{
		Text = "Role 5",
		x = (GUIVals.XPadLeft * 2) + GUIVals.RoleSpacingX,
		y = 600,
		MyVar = "ShowInSpec",
		SubVar = 5
	},
	Role6 = 
	{
		Text = "Role 6",
		x = (GUIVals.XPadLeft * 2) + GUIVals.RoleSpacingX,
		y = 620,
		MyVar = "ShowInSpec",
		SubVar = 6
	},
};

function GetDefaultTracker()
	local DefaultTracker = 
	{
		Invert = false,
		ShowStacks = false,
		ShowStacksBG = false,
		ShowTimer = false,
		ShowTimerBG = false,
		UseSpellIcon = false,
		MySpells = false,
		OnlyInCombat = false,
		ShowPercent = false,
		ThresholdPercent = false,
		ThresholdValue = 0,
		ThresholdOperator = ">=",
		TrackerAlpha = 1.0,
		AltTex = 1,
		SpellName = "",
		SpellType = "Buff",
		StacksPos = 
		{
			x = (WindowRight/2) + GUIVals.IconSize - 20,
			y = (WindowBottom/2) + GUIVals.IconSize - 20
		},
		StacksSize = 20,
		Texture = nil,
		TimerPos = 
		{
			x = WindowRight/2,
			y = (WindowBottom/2) + GUIVals.IconSize
		},
		TimerSize = 20,
		TrackUnit = "Self",
		TrackerPos = 
		{
			x = WindowRight/2,
			y = WindowBottom/2
		},
		TrackerSize = GUIVals.IconSize,
		ShowInSpec = 
		{
			[1] = true,
			[2] = true,
			[3] = true,
			[4] = true,
			[5] = true,
			[6] = true
		},
		StacksColor = 
		{
			r = 1,
			g = 1,
			b = 1,
			a = 1.0
		},
		TimerColor = 
		{
			r = 1,
			g = 1,
			b = 1,
			a = 1.0
		}
	};
	
	return DefaultTracker;
end

HelpWindowContent = "[Getting Started]\n\tTo create a new tracker effect, click "
.. "the \"New Tracker\" button at the top of the Spell Trackers window. This will "
.. "create a new spell tracker effect and open the editor window for it. For basic "
.. "information on what each parameter does, hover your mouse over the title.\n\n\n"
.. "[Editing a Tracker]\n  Click on one of the icons in the \"My Trackers\" list to"
.. " select a tracker and bring up the \"Edit\" and \"Test\" buttons for it. "
.. "Clicking Test will toggle test mode for the tracker. While in test mode, a "
.. "preview of the tracker will be displayed with its icon, and if you have opted "
.. "for this tracker to show them, its timer and stacks. While in test mode you "
.. "may adjust the position of the icon, timer, or stacks by dragging them with "
.. "the mouse. Clicking Edit will open the tracker\'s editor window, where you "
.. "will be able to modify several of its aspects.\n\n"
.. "More info on each parameter:\n   *Spell Name: The name "
.. "of the buff, debuff, or cooldown you wish to track. This is case-sensitive, "
.. "so you must enter the name exactly as it appears in game.\n   *Spell Type: "
.. "This indicates whether you are trying to track a buff, debuff, or cooldown.\n"
.. "   *Track Unit: The unit you wish to search for this effect on. You may only "
.. "track your own cooldowns, but you may track buffs and debuffs on yourself, your "
.. "target, your focus target, or your pet.\n   *Image: This is the default image to display, "
.. "it will be shown if you have not checked the \"Use Spell Image\" box, or if the "
.. "spell\'s image cannot be found.\n   *Only Show In Combat: This effect will "
.. "be hidden when you are out of combat.\n"
.. "   *Only Track My Spells: This effect will only track buffs and debuffs that "
.. "you have cast.\n   *Invert Effect: This effect will only be shown when the "
.. "buff or debuff is not present, or when the spell is not on cooldown.\n"
.. "   *Use Spell Image: Displays the sepll\'s icon as the effect\'s image. If "
.. "the spell image cannot be found, it will default to the selected image\n"
.. "   *Roles: If you only wish to track this effect while in certain roles, "
.. "check their corresponding box. By default every tracker will have all roles "
.. "checked, so uncheck those corresponding to the roles you dont wish to track "
.. "this effect in.\n\n   *Icon: This panel contains parameters related to the "
.. "effect image, click the \"Icon\" tab at the top of the panel to view it.\n"
.. "  -The text field labled \"Transparency\" refers to how transparent the image will "
.. "appear. You may input a decimal value between 0 and 1, 0 being fully transparent, "
.. "and 1 being fully opaque.\n  -The slider bars represent the size of the icon and "
.. "its position on the screen. While this effect is in test mode, you may also "
.. "adjust the position of the icon by dragging it with the mouse\n\n   *Stacks: "
.. "This panel contains parameters related to the stacks counter, click the "
.. "\"Stacks\" tab at the top of the panel to view it.\n  -The checkbox labled "
.. "\"Show Stacks\" indicates whether or not you wish to show a stack counter "
.. "for this effect.\n  -The checkbox labled \"Show Background\" indicates "
.. "whether or not you wish to give a black background to the stack counter text."
.. "\n  -The colored square labled \"Color\" shows the color of the stacks "
.. "counter text, clicking this square will open up a color picker for you to "
.. "choose the text color.\n  -The text field labled \"Transparency\" refers to "
.. "how transparent the stacks text(and background, if checked) will be.\n"
.. "  -The slider bars represent the size and position of the stacks counter.\n\n"
.. "   *Timer: This panel contains parameters related to the timer, click the "
.. "\"Timer\" tab at the top of the panel to view it.\n  -The checkbox labled " 
.. "\"Show Timer\" indicates whether or not you wish to show the timer for this "
.. "effect.\n  -The checkbox labled \"Show Background\" indicates whether or not "
.. "you wish to give a black background to the timer text.\n  -The colored square "
.. "labled \"Color\" shows the color of the timer text, clicking this square will "
.. "open up a color picker for you to choose the text color.\n  -The text field "
.. "labled \"Transparency\" refers to how transparent the timer text(and "
.. "background, if checked) will be.\n  -The slider bars represent the size and "
.. "position of the timer.";