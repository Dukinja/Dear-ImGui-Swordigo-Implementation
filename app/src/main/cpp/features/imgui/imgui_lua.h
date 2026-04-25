#ifndef SWMINI_IMGUI_LUA_H
#define SWMINI_IMGUI_LUA_H

#ifdef __cplusplus
extern "C" {
#endif

void miniImguiLua_SetEnabled(int enabled);
int miniImguiLua_IsEnabled(void);
void miniImguiLua_Clear(void);
void miniImguiLua_Begin(const char *title, int has_close_btn);
void miniImguiLua_End(void);
void miniImguiLua_Text(const char *text);
void miniImguiLua_Separator(void);
void miniImguiLua_SameLine(void);
void miniImguiLua_SetNextWindowSize(float width, float height);

int miniImguiLua_Button(const char *label);
int miniImguiLua_Checkbox(const char *label, int default_val);
float miniImguiLua_SliderFloat(const char *label, float default_val, float v_min, float v_max);
int miniImguiLua_SliderInt(const char *label, int default_val, int v_min, int v_max);
const char* miniImguiLua_InputText(const char *label, const char *default_val);
void miniImguiLua_ColorEdit3(const char *label, float *r, float *g, float *b);
int miniImguiLua_CollapsingHeader(const char *label, int default_open);

#ifdef __cplusplus
}
#endif

#endif