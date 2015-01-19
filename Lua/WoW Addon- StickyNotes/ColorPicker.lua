SelectedColor = {
	R = 1.0,
	G = 1.0,
	B = 1.0,
	A = 1.0 };
	
ColorAssignerCallback = nil;
ColorEditingFrame = nil;

function ShowColorPicker(r, g, b, a, callback)
	ColorPickerFrame:SetColorRGB(r,g,b);
	ColorPickerFrame.hasOpacity, ColorPickerFrame.opacity = (a ~= nil), a;
	ColorPickerFrame.previousValues = {r,g,b,a};
	ColorPickerFrame.func, ColorPickerFrame.opacityFunc, ColorPickerFrame.cancelFunc = 
		callback, callback, callback;
	ColorPickerFrame:Hide(); -- Need to run the OnShow handler.
	ColorPickerFrame:Show();
end

function ColorCallback(restore)
	local NewR, NewG, NewB, NewA;
	if restore then
		-- The user bailed, we extract the old color from the table created by ShowColorPicker.
		NewR, NewG, NewB, NewA = unpack(restore);
	else
		-- Something changed
		NewA, NewR, NewG, NewB = OpacitySliderFrame:GetValue(), ColorPickerFrame:GetColorRGB();
	end

	-- Update our internal storage.
	SelectedColor.R, SelectedColor.G, SelectedColor.B, SelectedColor.A = NewR, NewG, NewB, NewA;
	-- And update any UI elements that use this color...
	
	if(ColorAssignerCallback ~= nil) then
		ColorAssignerCallback(SelectedColor.R, SelectedColor.G, SelectedColor.B, SelectedColor.A, ColorEditingFrame);
		--ColorPickerFrame:Hide(); --do not hide here
	end
end

function TextColorCallback(r, g, b, a, guiframe)
	guiframe.TextColorEditor.Texture:SetTexture(r, g, b, a);
	guiframe.CurrentNote.TextField:SetTextColor(r, g, b, a);
	guiframe.CurrentNote.TextField.Color = {R=r, G=g, B=b, A=a};
	local Key = guiframe.CurrentNote.Key;
	CharacterSavedData[Key].TextColor = {R=r, G=g, B=b, A=a};
end

function NoteColorCallback(r, g, b, a, guiframe)
	guiframe.NoteColorEditor.Texture:SetTexture(r, g, b, a);
	guiframe.CurrentNote.BaseFrame.Texture:SetTexture(r, g, b, a);
	guiframe.CurrentNote.BaseFrame.Color = {R=r, G=g, B=b, A=a};
	local Key = guiframe.CurrentNote.Key;
	CharacterSavedData[Key].NoteColor = {R=r, G=g, B=b, A=a};
end

function TitleTextColorCallback(r, g, b, a, guiframe)
	guiframe.TitleColorEditor.Texture:SetTexture(r, g, b, a);
	guiframe.CurrentNote.TitleBar.Title:SetTextColor(r, g, b, a);
	local Key = guiframe.CurrentNote.Key;
	CharacterSavedData[Key].TitleColor = {R=r, G=g, B=b, A=a};
end

function TitleBarColorCallback(r, g, b, a, guiframe)
	guiframe.BarEditor.Texture:SetTexture(r, g, b, a);
	guiframe.CurrentNote.TitleBar.Texture:SetTexture(r, g, b, a);
	guiframe.CurrentNote.TitleBar.Color = {R=r, G=g, B=b, A=a};
	local Key = guiframe.CurrentNote.Key;
	CharacterSavedData[Key].TitleBarColor = {R=r, G=g, B=b, A=a};
end