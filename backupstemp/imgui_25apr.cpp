extern "C" {
#include "core/hooks.h"
#include "core/log.h"
}

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <cstdint>
#include <dlfcn.h>
#include <jni.h>
#include <time.h>

#include "libs/imgui/imgui.h"

#define LOG_TAG "MiniFeatureImGui"

namespace {

using EglSwapBuffersT = EGLBoolean (*)(EGLDisplay display, EGLSurface surface);
static_assert(sizeof(ImDrawIdx) == 2, "ImGui GLES1 backend requires 16-bit ImDrawIdx");

EglSwapBuffersT g_orig_swap_buffers = nullptr;
bool g_imgui_initialized = false;
timespec g_last_frame_time = {};
float g_touch_x = 0.0f;
float g_touch_y = 0.0f;
bool g_touch_down = false;
GLuint g_font_texture = 0;

void createFontsTextureIfNeeded() {
	if (g_font_texture != 0 && glIsTexture(g_font_texture)) {
		return;
	}

	ImGuiIO &io = ImGui::GetIO();
	unsigned char *pixels = nullptr;
	int width = 0;
	int height = 0;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	if (pixels == nullptr || width <= 0 || height <= 0) {
		LOGE("ImGui font atlas pixels unavailable.");
		return;
	}

	const GLboolean tex2d_enabled = glIsEnabled(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, &g_font_texture);
	glBindTexture(GL_TEXTURE_2D, g_font_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		width,
		height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		pixels
	);

	io.Fonts->SetTexID((ImTextureID) (intptr_t) g_font_texture);
	if (!tex2d_enabled) {
		glDisable(GL_TEXTURE_2D);
	}

	LOGI("ImGui font atlas uploaded (%dx%d).", width, height);
}

void renderDrawDataGLES1(ImDrawData *draw_data, int fb_width, int fb_height) {
	if (draw_data == nullptr || fb_width <= 0 || fb_height <= 0) {
		return;
	}

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	const GLboolean blend = glIsEnabled(GL_BLEND);
	const GLboolean cull = glIsEnabled(GL_CULL_FACE);
	const GLboolean depth = glIsEnabled(GL_DEPTH_TEST);
	const GLboolean scissor = glIsEnabled(GL_SCISSOR_TEST);
	const GLboolean texture2d = glIsEnabled(GL_TEXTURE_2D);
	const GLboolean lighting = glIsEnabled(GL_LIGHTING);
	const GLboolean color_array = glIsEnabled(GL_COLOR_ARRAY);
	const GLboolean texcoord_array = glIsEnabled(GL_TEXTURE_COORD_ARRAY);
	const GLboolean vertex_array = glIsEnabled(GL_VERTEX_ARRAY);

	GLint matrix_mode;
	glGetIntegerv(GL_MATRIX_MODE, &matrix_mode);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glViewport(0, 0, fb_width, fb_height);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrthof(0.0f, draw_data->DisplaySize.x, draw_data->DisplaySize.y, 0.0f, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	for (int n = 0; n < draw_data->CmdListsCount; n++) {
		const ImDrawList *cmd_list = draw_data->CmdLists[n];
		const ImDrawVert *vtx_buffer = cmd_list->VtxBuffer.Data;
		const ImDrawIdx *idx_buffer = cmd_list->IdxBuffer.Data;

		glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid *) &vtx_buffer->pos);
		glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid *) &vtx_buffer->uv);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (const GLvoid *) &vtx_buffer->col);

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
			const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback != nullptr) {
				pcmd->UserCallback(cmd_list, pcmd);
				continue;
			}

			const ImVec4 clip = pcmd->ClipRect;
			const GLint clip_x = (GLint) clip.x;
			const GLint clip_y = (GLint) (fb_height - clip.w);
			const GLsizei clip_w = (GLsizei) (clip.z - clip.x);
			const GLsizei clip_h = (GLsizei) (clip.w - clip.y);
			if (clip_w <= 0 || clip_h <= 0) {
				idx_buffer += pcmd->ElemCount;
				continue;
			}

			glScissor(clip_x, clip_y, clip_w, clip_h);
			glBindTexture(GL_TEXTURE_2D, (GLuint) (intptr_t) pcmd->GetTexID());
			glDrawElements(
				GL_TRIANGLES,
				(GLsizei) pcmd->ElemCount,
				GL_UNSIGNED_SHORT,
				(const GLvoid *) idx_buffer
			);
			idx_buffer += pcmd->ElemCount;
		}
	}

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(matrix_mode);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

	if (blend)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
	if (cull)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
	if (depth)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
	if (scissor)
		glEnable(GL_SCISSOR_TEST);
	else
		glDisable(GL_SCISSOR_TEST);
	if (texture2d)
		glEnable(GL_TEXTURE_2D);
	else
		glDisable(GL_TEXTURE_2D);
	if (lighting)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);
	if (color_array)
		glEnableClientState(GL_COLOR_ARRAY);
	else
		glDisableClientState(GL_COLOR_ARRAY);
	if (texcoord_array)
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	else
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if (vertex_array)
		glEnableClientState(GL_VERTEX_ARRAY);
	else
		glDisableClientState(GL_VERTEX_ARRAY);
}

void ensureImGuiInitialized() {
	if (g_imgui_initialized) {
		return;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGuiIO &io = ImGui::GetIO();
	io.FontGlobalScale = 2.5f;
	ImGui::GetStyle().ScaleAllSizes(1.8f);

	clock_gettime(CLOCK_MONOTONIC, &g_last_frame_time);
	g_imgui_initialized = true;
	LOGI("ImGui context initialized.");
}

void beginFrame(int width, int height) {
	createFontsTextureIfNeeded();

	const timespec now = [] {
		timespec ts = {};
		clock_gettime(CLOCK_MONOTONIC, &ts);
		return ts;
	}();

	double delta = (double) (now.tv_sec - g_last_frame_time.tv_sec);
	delta += (double) (now.tv_nsec - g_last_frame_time.tv_nsec) / 1000000000.0;
	if (delta <= 0.0) {
		delta = 1.0 / 60.0;
	}
	g_last_frame_time = now;

	ImGuiIO &io = ImGui::GetIO();

	io.DisplaySize = ImVec2((float) width, (float) height);
	io.DeltaTime = (float) delta;
	io.MousePos = ImVec2(g_touch_x, g_touch_y);
	io.MouseDown[0] = g_touch_down;

	ImGui::NewFrame();
}

void drawOverlayWindow() {
	//ImGui::SetNextWindowSize(ImVec2(980.0f, 620.0f), ImGuiCond_Always);
	//ImGui::SetNextWindowBgAlpha(0.7f);
	
	ImGui::Begin("SwMini ImGui");
	ImGui::TextUnformatted("Dear ImGui is active.");
	ImGui::Separator();
	ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	ImGui::TextUnformatted("Renderer: OpenGL ES 1.x");
	ImGui::End();
}

EGLBoolean hooked_swap_buffers(EGLDisplay display, EGLSurface surface) {
	ensureImGuiInitialized();

	int width = 0;
	int height = 0;
	eglQuerySurface(display, surface, EGL_WIDTH, &width);
	eglQuerySurface(display, surface, EGL_HEIGHT, &height);

	if (width > 0 && height > 0) {
		beginFrame(width, height);
		drawOverlayWindow();
		ImGui::Render();
		renderDrawDataGLES1(ImGui::GetDrawData(), width, height);
	}

	return g_orig_swap_buffers(display, surface);
}

} // namespace

extern "C" void initF_imgui(void) {
	void *swap = dlsym(RTLD_DEFAULT, "eglSwapBuffers");
	if (swap == nullptr) {
		void *egl_handle = dlopen("libEGL.so", RTLD_NOW);
		if (egl_handle != nullptr) {
			swap = dlsym(egl_handle, "eglSwapBuffers");
		}
	}

	if (swap == nullptr) {
		LOGE("Could not find eglSwapBuffers; ImGui hook disabled.");
		return;
	}

	if (hook_address(swap, (void *) hooked_swap_buffers, (void **) &g_orig_swap_buffers) == nullptr) {
		LOGE("Failed to hook eglSwapBuffers.");
		return;
	}

	LOGI("ImGui hooked via eglSwapBuffers.");
}

extern "C" JNIEXPORT void JNICALL
Java_net_itsjustsomedude_swrdg_NativeBridge_imguiTouch(JNIEnv *, jclass, jint action, jfloat x, jfloat y) {
	g_touch_x = x;
	g_touch_y = y;

	switch (action) {
		case 0: // ACTION_DOWN
		case 5: // ACTION_POINTER_DOWN
		case 2: // ACTION_MOVE
			g_touch_down = true;
			break;
		case 1: // ACTION_UP
		case 3: // ACTION_CANCEL
		case 6: // ACTION_POINTER_UP
			g_touch_down = false;
			break;
		default:
			break;
	}
}
