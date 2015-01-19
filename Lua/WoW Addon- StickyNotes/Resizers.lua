function CreateResizeWidgets(parent)
	local LeftResizer = CreateDraggableFrame("LeftResizer", UIParent);
	LeftResizer:SetClampedToScreen(true);
	LeftResizer.Resizing = false;
	LeftResizer.LastPos = {};
	LeftResizer.Parent = parent;
	LeftResizer:SetSize(ResizerSize, ResizerSize);
	LeftResizer.Texture = GenerateTexture(nil, "BACKGROUND", {R=0.0, G=1.0, B=1.0, A=1.0}, LeftResizer);
	LeftResizer.Texture:SetTexture("Interface\\AddOns\\StickyNotes\\textures\\UI-PassiveOverlay");
	LeftResizer.Texture:SetTexCoord(1, 0, 1, 0);
	LeftResizer:SetFrameLevel(WidgetFrameLevel);
	
	-------------------------------------------------------------------------------
	local RightResizer = CreateDraggableFrame("RightResizer", UIParent);
	RightResizer:SetClampedToScreen(true);
	RightResizer.Resizing = false;
	RightResizer.LastPos = {};
	RightResizer.Parent = parent;
	RightResizer:SetSize(ResizerSize, ResizerSize);
	RightResizer.Texture = GenerateTexture(nil, "BACKGROUND", {R=0.0, G=1.0, B=1.0, A=1.0}, RightResizer);
	RightResizer.Texture:SetTexture("Interface\\AddOns\\StickyNotes\\textures\\UI-PassiveOverlay");
	RightResizer.Texture:SetTexCoord(0, 1, 1, 0);
	RightResizer:SetFrameLevel(WidgetFrameLevel);
	
	LeftResizer:SetScript("OnDragStart", LeftDragStart);
	LeftResizer:SetScript("OnDragStop", LeftDragStop);
	LeftResizer:SetScript("OnUpdate", UpdateLeftResizer);
	
	-----------------------------------------------------
	RightResizer:SetScript("OnDragStart", RightDragStart);
	RightResizer:SetScript("OnDragStop", RightDragStop);
	RightResizer:SetScript("OnUpdate", UpdateRightResizer);
	
	parent.LeftResizer = LeftResizer;
	parent.RightResizer = RightResizer;
	
	AttachResizers(parent);
end

function AttachResizers(parent)
	parent.LeftResizer:ClearAllPoints();
	parent.LeftResizer:SetPoint("BOTTOMLEFT", parent, "BOTTOMLEFT");
	parent.RightResizer:ClearAllPoints();
	parent.RightResizer:SetPoint("BOTTOMRIGHT", parent, "BOTTOMRIGHT");
end

function LeftDragStart(self, button)
	self:StartMoving();
	self.Resizing = true;
	self.LastPos.X = (self:GetLeft());
	self.LastPos.Y = (self:GetBottom());
end

function LeftDragStop(self)
	DragStopCommon(self);
end

function RightDragStart(self, button)
	self:StartMoving();
	self.Resizing = true;
	self.LastPos.X = (self:GetRight());
	self.LastPos.Y = (self:GetBottom());
end

function RightDragStop(self)
	DragStopCommon(self);
end

function UpdateLeftResizer(self, arg1)
	if(self.Resizing == true) then
		local BaseFrame = self.Parent;
		local Note = BaseFrame.Parent;
		local CurrentX = (self:GetLeft());
		local CurrentY = (self:GetBottom());
		
		local DeltaX = (CurrentX - self.LastPos.X);
		local DeltaY = (CurrentY - self.LastPos.Y);
		
		local NewWidth = self.Parent:GetWidth() - DeltaX;
		local NewHeight = self.Parent:GetHeight() - DeltaY;
		
		if(NewWidth < MinimumNoteWidth) then
			NewWidth = MinimumNoteWidth + 1;
			self.Resizing = false;
		else
			self.LastPos.X = CurrentX;
		end
		
		if(NewHeight < TitleBarHeight + ResizerSize) then
			NewHeight = TitleBarHeight + ResizerSize + 1;
			self.Resizing = false;
		else
			self.LastPos.Y = CurrentY;
		end
		
		self.Parent:SetSize(NewWidth, NewHeight);
		
		Note.TitleBar:SetWidth(NewWidth);
		Note.TitleBar.Title:SetWidth(NewWidth);
		Note.TitleBar:SetPoint("BOTTOMLEFT", UIParent, "BOTTOMLEFT", self:GetLeft(), Note.TitleBar:GetBottom());
		
		if(self.Resizing == false) then
			DragStopCommon(self);
		else
			AttachResizers(self.Parent);
			self.LastPos.X = (self:GetLeft());
			self.LastPos.Y = (self:GetBottom());
		end
	end
end

function UpdateRightResizer(self, arg1)
	if(self.Resizing == true) then
		local BaseFrame = self.Parent;
		local Note = BaseFrame.Parent;
		local CurrentX = (self:GetRight());
		local CurrentY = (self:GetBottom());
		
		local DeltaX = (CurrentX - self.LastPos.X);
		local DeltaY = (CurrentY - self.LastPos.Y);
		
		local NewWidth = self.Parent:GetWidth() + DeltaX;
		local NewHeight = self.Parent:GetHeight() - DeltaY;
		
		if(NewWidth < MinimumNoteWidth) then
			NewWidth = MinimumNoteWidth + 1;
			self.Resizing = false;
		else
			self.LastPos.X = CurrentX;
		end
		
		if(NewHeight < TitleBarHeight + ResizerSize) then
			NewHeight = TitleBarHeight + ResizerSize + 1;
			self.Resizing = false;
		else
			self.LastPos.Y = CurrentY;
		end
		
		self.Parent:SetSize(NewWidth, NewHeight);
		
		Note.TitleBar:SetWidth(NewWidth);
		Note.TitleBar.Title:SetWidth(NewWidth);
		Note.TitleBar:SetPoint("BOTTOMRIGHT", UIParent, "BOTTOMLEFT", self:GetRight(), Note.TitleBar:GetBottom());
		
		if(self.Resizing == false) then
			DragStopCommon(self);
		else
			AttachResizers(self.Parent);
			self.LastPos.X = (self:GetRight());
			self.LastPos.Y = (self:GetBottom());
		end
	end
end

function DragStopCommon(self)
	self:StopMovingOrSizing();
	self.Resizing = false;
	self.Parent:StopMovingOrSizing();
	AttachResizers(self.Parent);
	AttachNoteComponents(self.Parent.Parent);
	local Key = self.Parent.Parent.Key;
	local Width, Height = self.Parent:GetSize();
	CharacterSavedData[Key].Size.Width = Width;
	CharacterSavedData[Key].Size.Height = Height;
end

function ResizersApplyVisibility(note)
	local IsLocked = note.LockWidget.Locked;
	local IsMinimized = note.Minimizer.Minimized;
	if((IsLocked == true) or (IsMinimized == true)) then
		note.BaseFrame.LeftResizer:Hide();
		note.BaseFrame.RightResizer:Hide();
	else
		note.BaseFrame.LeftResizer:Show();
		note.BaseFrame.RightResizer:Show();
	end
end