#include "mini.h"
#include "features/imgui/imgui_lua.h"

#include "lauxlib.h"
#include "lua.h"

static const char* imgui_wrapper_code = 
    "return function(ig)\n"
    "    ig.Windows = {}\n"
    "    ig.CreateWindow = function(title, width, height, draw_callback)\n"
    "        ig.Windows[title] = { width = width, height = height, draw = draw_callback, first_run = true }\n"
    "    end\n"
    "    ig.StartMasterLoop = function()\n"
    "        while true do\n"
    "                if ig.IsEnabled() then\n"
    "                    ig.Clear()\n"
    "                    for title, win in pairs(ig.Windows) do\n"
    "                        if win.first_run then\n"
    "                            ig.SetNextWindowSize(win.width, win.height)\n"
    "                            win.first_run = false\n"
    "                        end\n"
    "                        \n"
    "                        ig.Begin(title, true)\n"
    "                        win.draw()\n"
    "                        ig.End()\n"
    "                    end\n"
    "                end\n"
    "            Program.Wait(0.033)"
    "        end\n"
    "    end\n"
    "end\n";

static int miniLL_imgui_set_enabled(lua_State *L) {
    miniImguiLua_SetEnabled(lua_toboolean(L, 1));
    return 0;
}

static int miniLL_imgui_is_enabled(lua_State *L) {
    lua_pushboolean(L, miniImguiLua_IsEnabled());
    return 1;
}

static int miniLL_imgui_clear(lua_State *L) {
    miniImguiLua_Clear();
    return 0;
}

static int miniLL_imgui_begin(lua_State *L) {
    const char *title = luaL_optstring(L, 1, "Mini.ImGui");
    int has_close = lua_toboolean(L, 2); 
    miniImguiLua_Begin(title, has_close);
    return 0;
}

static int miniLL_imgui_end(lua_State *L) {
    miniImguiLua_End();
    return 0;
}

static int miniLL_imgui_text(lua_State *L) {
    miniImguiLua_Text(luaL_optstring(L, 1, ""));
    return 0;
}

static int miniLL_imgui_separator(lua_State *L) {
    miniImguiLua_Separator();
    return 0;
}

static int miniLL_imgui_same_line(lua_State *L) {
    miniImguiLua_SameLine();
    return 0;
}

static int miniLL_imgui_set_next_window_size(lua_State *L) {
    miniImguiLua_SetNextWindowSize((float)luaL_checknumber(L, 1), (float)luaL_checknumber(L, 2));
    return 0;
}

static int miniLL_imgui_button(lua_State *L) {
    lua_pushboolean(L, miniImguiLua_Button(luaL_checkstring(L, 1)));
    return 1;
}

static int miniLL_imgui_checkbox(lua_State *L) {
    lua_pushboolean(L, miniImguiLua_Checkbox(luaL_checkstring(L, 1), lua_toboolean(L, 2)));
    return 1;
}

static int miniLL_imgui_slider_float(lua_State *L) {
    lua_pushnumber(L, miniImguiLua_SliderFloat(luaL_checkstring(L, 1), luaL_optnumber(L, 2, 0.0), luaL_optnumber(L, 3, 0.0), luaL_optnumber(L, 4, 100.0)));
    return 1;
}

static int miniLL_imgui_slider_int(lua_State *L) {
    lua_pushinteger(L, miniImguiLua_SliderInt(luaL_checkstring(L, 1), luaL_optinteger(L, 2, 0), luaL_optinteger(L, 3, 0), luaL_optinteger(L, 4, 100)));
    return 1;
}

static int miniLL_imgui_input_text(lua_State *L) {
    lua_pushstring(L, miniImguiLua_InputText(luaL_checkstring(L, 1), luaL_optstring(L, 2, "")));
    return 1;
}

static int miniLL_imgui_color_edit3(lua_State *L) {
    float r = (float)luaL_optnumber(L, 2, 1.0);
    float g = (float)luaL_optnumber(L, 3, 1.0);
    float b = (float)luaL_optnumber(L, 4, 1.0);
    miniImguiLua_ColorEdit3(luaL_checkstring(L, 1), &r, &g, &b);
    lua_pushnumber(L, r);
    lua_pushnumber(L, g);
    lua_pushnumber(L, b);
    return 3;
}

static int miniLL_imgui_collapsing_header(lua_State *L) {
    lua_pushboolean(L, miniImguiLua_CollapsingHeader(luaL_checkstring(L, 1), lua_toboolean(L, 2)));
    return 1;
}

static const luaL_Reg mini_imgui_lib[] = {
    {"SetEnabled",        miniLL_imgui_set_enabled},
    {"IsEnabled",         miniLL_imgui_is_enabled},
    {"Clear",             miniLL_imgui_clear},
    {"Begin",             miniLL_imgui_begin},
    {"End",               miniLL_imgui_end},
    {"Text",              miniLL_imgui_text},
    {"Separator",         miniLL_imgui_separator},
    {"SameLine",          miniLL_imgui_same_line},
    {"SetNextWindowSize", miniLL_imgui_set_next_window_size},
    {"Button",            miniLL_imgui_button},
    {"Checkbox",          miniLL_imgui_checkbox},
    {"SliderFloat",       miniLL_imgui_slider_float},
    {"SliderInt",         miniLL_imgui_slider_int},
    {"InputText",         miniLL_imgui_input_text},
    {"ColorEdit3",        miniLL_imgui_color_edit3},
    {"CollapsingHeader",  miniLL_imgui_collapsing_header},
    {NULL, NULL}
};

int miniLL_open_imgui(lua_State *L) {
    lua_newtable(L);
    luaL_register(L, NULL, mini_imgui_lib);

    if (luaL_loadstring(L, imgui_wrapper_code) == 0) {
        if (lua_pcall(L, 0, 1, 0) == 0) {
            lua_pushvalue(L, -2); 
            if (lua_pcall(L, 1, 0, 0) != 0) {
                lua_pop(L, 1);
            }
        } else {
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1);
    }

    return 1;
}

void initLL_imgui(void) { }
