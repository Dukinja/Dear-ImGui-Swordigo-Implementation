features:

full touch support
all functions work
lua api

todo:

work on text input

example usage:

local ig = Mini.ImGui
local chkbox = false
local slider = 1.0
if r == nil then r, g, b = 1.0, 0.0, 0.0 end

```lua
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
end)```

ig.StartMasterLoop()
