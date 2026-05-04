#include "./imgui.hpp"

imgui::imgui(GLFWwindow *glfw_window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.IniFilename = NULL;
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
	ImGui_ImplOpenGL3_Init("#version 460");
}

imgui::~imgui() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

bool imgui::is_cursor_hovering_over() const {
	return ImGui::GetIO().WantCaptureMouse;
}

void imgui::begin() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void imgui::end() {
	ImGui::Render();
}

void imgui::draw() {
	bool srgb_enabled = glIsEnabled(GL_FRAMEBUFFER_SRGB);
	glDisable(GL_FRAMEBUFFER_SRGB);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	if (srgb_enabled) {
		glEnable(GL_FRAMEBUFFER_SRGB);
	}
}
