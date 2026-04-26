# Features:

- **Fully Functional Touch**
- **Functional Dear ImGui Functions**
- **Lua API**

# TODO:

- **Work on InputText()**

# Example:

```lua
local ig = Mini.ImGui
local chkbox = false
local slider = 1.0
if r == nil then r, g, b = 1.0, 0.0, 0.0 end

ig.CreateWindow("test window", 400, 300, function()
    ig.Text("test text")
    ig.Separator()

    chkbox = ig.Checkbox("chkbox", chkbox)
    slider = ig.SliderFloat("slider", slider, 1.0, 10.0)

    if ig.Button("button") then
      -- execute code
    end

     if ig.CollapsingHeader("collapsing header test", true) then
        r, g, b = ig.ColorEdit3("test color", r, g, b)
    end
end)

ig.StartMasterLoop()
```

# CreateWindow(title, width, height, callback_function)
**Description: The primary way to create a UI. Automatically handles window sizing, the render loop, and queue clearing.**

**Usage:**
```lua
Mini.ImGui.CreateWindow("My Mod", 400, 300, function()
end)
```

# SetEnabled(bool)
**Description: Shows or hides the entire ImGui overlay (all windows).**

**Usage:**
```lua
Mini.ImGui.SetEnabled(true)
```

# Text(string)
**Description: Displays a static label.**

**Usage:**
```lua
ig.Text("Player HP: " .. hp)
```

# Button(label)
**Description: Creates a clickable button. Returns true only on the frame it is clicked.**

**Usage:**
```lua
if ig.Button("Kill All Enemies") then
end
```

# Checkbox(label, current_state)
**Description: A toggle switch. Returns the new boolean state.**

**Usage:**
```lua
god_mode = ig.Checkbox("God Mode", god_mode)
```
**NOTE: USE GETTERS AND SETTERS. Example:**

```lua
local ig = Mini.ImGui
local test_bool = false

ig.CreateWindow("Debug Window", 400, 300, function()
    test_bool = ig.Checkbox("Debug Box", test_bool)

    if test_bool then
        Game.SetCinematicMode(true, true)
    else
        Game.SetCinematicMode(false, false)
    end
end)
ig.StartMasterLoop()
```

# SliderFloat(label, value, min, max)
**Description: A horizontal slider for decimal numbers. Returns the updated value.**

**Usage:**
```lua
speed = ig.SliderFloat("Speed", speed, 1.0, 20.0)
```

# SliderInt(label, value, min, max)
**Description: A horizontal slider for whole numbers. Returns the updated value.**

**Usage:**
```lua
coins = ig.SliderInt("Coins", coins, 0, 9999)
```

# Separator()
**Description: Draws a horizontal line to group items.**

**Usage:**
```lua
ig.Separator()
```

# SameLine()
**Description: Forces the next widget to appear on the same line as the previous one.**

**Usage:**
```lua
ig.Button("Save")
ig.SameLine()
ig.Button("Load")
```

# CollapsingHeader(label, default_open)
**Description: Creates an expandable section. Returns true if expanded. Use an if statement to hide the elements inside.**

**Usage:**
```lua
if ig.CollapsingHeader("Advanced Settings", false) then
    ig.Text("These only show when open!")
end
```

# ColorEdit3(label, r, g, b)
**Description: Opens a color picker. Returns three decimal values (Red, Green, Blue).**

**Usage:**
```lua
r, g, b = ig.ColorEdit3("Aura Color", r, g, b)
```

# More Functions

You can request for more Dear ImGui Functions by creating a Issue (GitHub Issue) or by messaging me on Discord (@xdukinja/XDukinja#7824). 
