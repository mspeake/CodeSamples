local BUTTON_NORMAL = "textures/dropdownbutton.png"
local BUTTON_HIGHLIGHT = "textures/dropdownbutton_highlight.png"
local BUTTON_CLICKED = "textures/dropdownbutton_clicked.png"

-- Helper Functions

local function contains(tbl, val)
  for k, v in pairs(tbl) do
    if v == val then
      return true
    end
  end
  return false
end

local function UpdateSelection(self, index)
  local item = self.items[index]

  if index == nil then
    self.current:SetText("Select...")
  else
    self.current:SetText(item)
  end
end


-- Current Frame Events

local function CurrentClick(self)
  local widget = self:GetParent()
  if not widget.enabled then return end
  local dropdown = widget.dropdown
  dropdown:SetVisible(not dropdown:GetVisible())
end


-- Dropdown Frame Events

local function DropdownItemClick(self)
  local widget = self.widget
  widget.dropdown:SetVisible(false)
  widget:SetSelectedIndex(self.index)
end

local function DropdownItemMouseIn(self)
  self:SetBackgroundColor(0.3, 0.3, 0.3, 1)
end

local function DropdownItemMouseOut(self)
  self:SetBackgroundColor(0, 0, 0, 0)
end


-- Public Functions

local function SetBorder(self, width, r, g, b, a)
  Library.LibSimpleWidgets.SetBorder(self.current, width, r, g, b, a)
  Library.LibSimpleWidgets.SetBorder(self.dropdown, width, r, g, b, a)
end

local function SetBackgroundColor(self, r, g, b, a)
  self.current:SetBackgroundColor(r, g, b, a)
  self.dropdown:SetBackgroundColor(r, g, b, a)
end

local function GetFontSize(self)
  return self.current:GetFontSize()
end

local function SetFontSize(self, size)
  assert(type(size) == "number", "param 1 must be a number!")
  self.current:SetFontSize(size)

  for i, itemFrame in ipairs(self.itemFrames) do
    itemFrame:SetFontSize(size)
  end

  self:ResizeToFit()
end

local function GetShowArrow(self)
  return self.button:GetVisible()
end

local function SetShowArrow(self, showArrow)
  self.button:SetVisible(showArrow)
end

local function ResizeToFit(self)
  self.current:ClearAll()
  self:SetHeight(self.current:GetHeight())

  -- Find max item width, resets anchoring of each item frame
  local maxWidth = self.current:GetWidth()
  for i, itemFrame in ipairs(self.itemFrames) do
    if itemFrame:GetVisible() then
      itemFrame:ClearAll()
      maxWidth = math.max(maxWidth, itemFrame:GetWidth())
    end
  end

  self.current:SetAllPoints(self)
  self:SetWidth(maxWidth + self.button:GetWidth())

  -- re-anchor item frames
  local dropdownHeight = 0
  local prevItemFrame
  for i, itemFrame in ipairs(self.itemFrames) do
    if itemFrame then
      if prevItemFrame then
        itemFrame:SetPoint("TOPLEFT", prevItemFrame, "BOTTOMLEFT")
        itemFrame:SetPoint("TOPRIGHT", prevItemFrame, "BOTTOMRIGHT")
      else
        itemFrame:SetPoint("TOPLEFT", self.dropdownContent, "TOPLEFT")
        itemFrame:SetPoint("TOPRIGHT", self.dropdownContent, "TOPRIGHT")
      end
    end
    dropdownHeight = dropdownHeight + itemFrame:GetHeight()
    prevItemFrame = itemFrame
  end

  self.dropdownContent:SetHeight(dropdownHeight)
  self.dropdown:SetHeight(math.min(self.maxDropdownHeight, dropdownHeight))
  self.dropdown:SetScrollInterval(self.current:GetHeight())
end

local function GetEnabled(self)
  return self.enabled
end

local function SetEnabled(self, enabled)
  assert(type(enabled) == "boolean", "param 1 must be a boolean!")

  self.enabled = enabled
  if enabled then
    self.current:SetFontColor(1, 1, 1, 1)
  else
    self.current:SetFontColor(0.5, 0.5, 0.5, 1)
    self.dropdown:SetVisible(false)
  end
end

local function GetMaxDropdownHeight(self)
  return self.maxDropdownHeight
end

local function SetMaxDropdownHeight(self, maxDropdownHeight)
  self.maxDropdownHeight = maxDropdownHeight
  self.dropdown:SetHeight(math.min(maxDropdownHeight, self.dropdownContent:GetHeight()))
end

local function GetItems(self)
  return self.items
end

local function SetItems(self, items, values)
  assert(type(items) == "table", "param 1 must be a table!")
  assert(values == nil or type(values) == "table", "param 2 must be a table!")

  self.items = items
  self.values = values or {}

  -- setup item frames
  local dropdownHeight = 0
  local prevItemFrame
  for i, v in ipairs(items) do
    local itemFrame
    if not self.itemFrames[i] then
      itemFrame = UI.CreateFrame("Text", self.dropdownContent:GetName().."Item"..i, self.dropdownContent)
      if prevItemFrame then
        itemFrame:SetPoint("TOPLEFT", prevItemFrame, "BOTTOMLEFT")
        itemFrame:SetPoint("TOPRIGHT", prevItemFrame, "BOTTOMRIGHT")
      else
        itemFrame:SetPoint("TOPLEFT", self.dropdownContent, "TOPLEFT")
        itemFrame:SetPoint("TOPRIGHT", self.dropdownContent, "TOPRIGHT")
      end
      itemFrame.Event.LeftClick = DropdownItemClick
      itemFrame.Event.MouseIn = DropdownItemMouseIn
      itemFrame.Event.MouseOut = DropdownItemMouseOut
      itemFrame.index = i
      itemFrame.widget = self
      self.itemFrames[i] = itemFrame
    else
      itemFrame = self.itemFrames[i]
    end
    itemFrame:SetText(v)
    itemFrame:SetVisible(true)
    dropdownHeight = dropdownHeight + itemFrame:GetHeight()
    prevItemFrame = itemFrame
  end

  -- set unused item frames invisible
  if #items < #self.itemFrames then
    for i = #items+1, #self.itemFrames do
      self.itemFrames[i]:SetVisible(false)
    end
  end

  self.dropdownContent:SetHeight(dropdownHeight)
  self.dropdown:SetHeight(math.min(self.maxDropdownHeight, dropdownHeight))

  self.selectedIndex = nil
  UpdateSelection(self, nil)
end

local function GetValues(self)
  return self.values
end

local function GetSelectedItem(self)
  return self.items[self.selectedIndex]
end

local function SetSelectedItem(self, item, silent)
  assert(silent == nil or type(silent) == "boolean", "param 2 must be a boolean!")

  if item then
    for i, v in ipairs(self.items) do
      if v == item then
        self:SetSelectedIndex(i, silent)
        return
      end
    end
  end

  self:SetSelectedIndex(nil, silent)
end

local function GetSelectedValue(self)
  return self.values[self.selectedIndex]
end

local function SetSelectedValue(self, value, silent)
  assert(silent == nil or type(silent) == "boolean", "param 2 must be a boolean!")

  if value then
    for i, v in ipairs(self.values) do
      if v == value then
        self:SetSelectedIndex(i, silent)
        return
      end
    end
  end

  self:SetSelectedIndex(nil, silent)
end

local function GetSelectedIndex(self)
  return self.selectedIndex
end

local function SetSelectedIndex(self, index, silent)
  assert(type(index) == "number", "param 1 must be a number!")
  assert(silent == nil or type(silent) == "boolean", "param 2 must be a boolean!")

  if index and (index < 1 or index > #self.items) then
    index = nil
  end

  if index == self.selectedIndex then
    return
  end

  self.selectedIndex = index
  UpdateSelection(self, index)

  if not silent and self.Event.ItemSelect then
    local item = self.items[index]
    local value = self.values[index]
    self.Event.ItemSelect(self, item, value, index)
  end
end


-- Constructor Function

function Library.LibSimpleWidgets.Select(name, parent)
  local widget = UI.CreateFrame("Frame", name, parent)
  widget.current = UI.CreateFrame("Text", widget:GetName().."Current", widget)
  widget.button = UI.CreateFrame("Texture", widget:GetName().."Button", widget)
  widget.dropdown = UI.CreateFrame("SimpleScrollView", widget:GetName().."DropdownScroller", widget)
  widget.dropdownContent = UI.CreateFrame("Frame", widget:GetName().."Dropdown", widget)

  widget.enabled = true
  widget.items = {}
  widget.values = {}
  widget.itemFrames = {}
  widget.selectedIndex = nil

  widget.current:SetBackgroundColor(0, 0, 0, 1)
  widget.current:SetText("Select...")
  widget.current:SetLayer(1)
  widget.current.Event.LeftClick = CurrentClick

  widget.maxDropdownHeight = widget.current:GetHeight() * 10

  widget.button:SetTexture("LibSimpleWidgets", BUTTON_NORMAL)
  local buttonWidth = widget.button:GetWidth()
  local buttonHeight = widget.button:GetHeight()
  widget.button:SetPoint("TOPRIGHT", widget.current, "TOPRIGHT", 1, -1)
  widget.button:SetPoint("BOTTOMRIGHT", widget.current, "BOTTOMRIGHT", 1, 1)
  widget.button:SetLayer(widget.current:GetLayer()+1)
  widget.button.Event.LeftClick = CurrentClick
  widget.button.Event.MouseIn = function(self) self:SetTexture("LibSimpleWidgets", BUTTON_HIGHLIGHT) end
  widget.button.Event.MouseOut = function(self) self:SetTexture("LibSimpleWidgets", BUTTON_NORMAL) end
  widget.button.Event.LeftDown = function(self) self:SetTexture("LibSimpleWidgets", BUTTON_CLICKED) end
  widget.button.Event.LeftUp = function(self) self:SetTexture("LibSimpleWidgets", BUTTON_HIGHLIGHT) end
  widget.button.Event.Size = function(self) self:SetWidth(self:GetHeight() / 19 * 21) end

  widget.dropdown:SetPoint("TOPLEFT", widget.current, "BOTTOMLEFT", 0, 5)
  widget.dropdown:SetPoint("TOPRIGHT", widget.current, "BOTTOMRIGHT", 0, 5)
  widget.dropdown:SetVisible(false)
  widget.dropdown:SetHeight(widget.current:GetHeight()*3)
  widget.dropdown:SetScrollInterval(widget.current:GetHeight())

  widget.dropdownContent:SetBackgroundColor(0, 0, 0, 1)
  widget.dropdownContent:SetHeight(widget.dropdown:GetHeight())

  widget.dropdown:SetContent(widget.dropdownContent)

  widget:SetWidth(widget.current:GetWidth() + buttonWidth)
  widget:SetHeight(widget.current:GetHeight())
  widget.current:SetAllPoints(widget)

  widget.SetBorder = SetBorder
  widget.SetBackgroundColor = SetBackgroundColor
  widget.GetFontSize = GetFontSize
  widget.SetFontSize = SetFontSize
  widget.GetShowArrow = GetShowArrow
  widget.SetShowArrow = SetShowArrow
  widget.ResizeToDefault = ResizeToFit -- TODO: Deprecated.
  widget.ResizeToFit = ResizeToFit
  widget.GetEnabled = GetEnabled
  widget.SetEnabled = SetEnabled
  widget.GetMaxDropdownHeight = GetMaxDropdownHeight
  widget.SetMaxDropdownHeight = SetMaxDropdownHeight
  widget.GetItems = GetItems
  widget.SetItems = SetItems
  widget.GetValues = GetValues
  widget.GetSelectedIndex = GetSelectedIndex
  widget.SetSelectedIndex = SetSelectedIndex
  widget.GetSelectedItem = GetSelectedItem
  widget.SetSelectedItem = SetSelectedItem
  widget.GetSelectedValue = GetSelectedValue
  widget.SetSelectedValue = SetSelectedValue

  Library.LibSimpleWidgets.EventProxy(widget, {"ItemSelect"})

  Library.LibSimpleWidgets.SetBorder(widget.current, 1, 165/255, 162/255, 134/255, 1, "t")
  Library.LibSimpleWidgets.SetBorder(widget.current, 1, 103/255, 98/255, 88/255, 1, "lr")
  Library.LibSimpleWidgets.SetBorder(widget.current, 1, 17/255, 66/255, 55/255, 1, "b")
  Library.LibSimpleWidgets.SetBorder(widget.dropdown, 1, 165/255, 162/255, 134/255, 1, "tblr")

  return widget
end
