// ImGui - standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)
// (GL3W is a helper library to access OpenGL functions since there is no standard header to access modern OpenGL functions easily. Alternatives are GLEW, Glad, etc.)

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <GL/gl3w.h>    // This example is using gl3w to access OpenGL functions (because it is small). You may use glew/glad/glLoadGen/etc. whatever already works for you.
#include <GLFW/glfw3.h>
#include "./../jsonRVM.h"

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


namespace nlohmann
{
	namespace detail
	{
		void to_json(json& jsonValue, const ImVec2& data)
		{
			jsonValue[0] = data.x;
			jsonValue[1] = data.y;
		}

		void from_json(const json& jsonValue, ImVec2& data)
		{
			data = { 0, 0 };
			if (jsonValue.is_array())
			{
				if (jsonValue.size() > 0) data.x = jsonValue[0].get<float>();
				if (jsonValue.size() > 1) data.y = jsonValue[1].get<float>();
			}
		}

		void to_json(json& jsonValue, const ImVec4& data)
		{
			jsonValue[0] = data.x;
			jsonValue[1] = data.y;
			jsonValue[2] = data.z;
			jsonValue[3] = data.w;
		}

		void from_json(const json& jsonValue, ImVec4& data)
		{
			data = { 0, 0, 0, 0 };
			if (jsonValue.is_array())
			{
				if (jsonValue.size() > 0) data.x = jsonValue[0].get<float>();
				if (jsonValue.size() > 1) data.y = jsonValue[1].get<float>();
				if (jsonValue.size() > 2) data.z = jsonValue[2].get<float>();
				if (jsonValue.size() > 3) data.w = jsonValue[3].get<float>();
			}
		}
	}
}


static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

extern double g_Time;

void __fastcall viewport(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (!subview.is_object()) subview = json::object();
	if (!subview.count("visible")) subview["visible"] = true;

	if (!objview.is_object()) objview = json::object();
	if (!objview.count("title")) objview["title"] = ""s;
	if (!objview.count("elements")) objview["elements"] = json::array();

	bool&	visible = subview["visible"].get_ref<bool&>();

	g_Time = 0.0f;
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(800, 600, objview["title"].get_ref<string&>().c_str(), NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    gl3wInit();

    // Setup ImGui binding
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
    ImGui_ImplGlfwGL3_Init(window, true);

    // Setup style
    //ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them. 
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple. 
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    bool show_demo_window = false;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    while (!glfwWindowShouldClose(window) && visible)
    {
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();

        // 1. Show a simple window.
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets automatically appears in a window called "Debug".
        {
            static float f = 0.0f;
            static int counter = 0;
            ImGui::Text("Hello, world!");                           // Display some text (you can use a format string too)
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f    
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our windows open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (NB: most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }

        // 2. Show another window
		ExecEntity(EV, objview["elements"], Value);

        // 3. Show the ImGui demo window. Most of the sample code is in ImGui::ShowDemoWindow(). Read its code to learn more about Dear ImGui!
        if (show_demo_window)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}


void __fastcall form(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (!subview.is_object()) subview = json::object();
	if (!subview.count("visible")) subview["visible"] = true;

	if (!objview.is_object()) objview = json::object();
	if (!objview.count("title")) objview["title"] = ""s;
	if (!objview.count("elements")) objview["elements"] = json::array();

	bool&	visible = subview["visible"].get_ref<bool&>();

	if (visible)
	{
		ImGui::Begin(objview["title"].get_ref<string&>().c_str(), &visible);
		ExecEntity(EV, objview["elements"], Value);
		ImGui::End();
	}
}


void __fastcall text(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (!subview.is_object()) subview = json::object();
	if (!subview.count("visible")) subview["visible"] = true;
	bool&	visible = subview["visible"].get_ref<bool&>();

	if (visible)
	{
		if (objview.is_string())
			ImGui::Text(objview.get_ref<string&>().c_str());
		else
			ImGui::Text(objview.dump().c_str());
	}
}


void __fastcall button(json &EV, json &Value)
{
	json& subview = jref(EV["->"]);
	json objview; ViewEntity(jref(EV["ctx"]), jref(EV["<-"]), objview);

	if (!subview.is_object()) subview = json::object();
	if (!subview.count("visible")) subview["visible"] = true;

	if (!objview.is_object()) objview = json::object();
	if (!objview.count("text")) objview["text"] = ""s;
	if (!objview.count("clicked")) objview["clicked"] = json();

	bool&	visible = subview["visible"].get_ref<bool&>();

	if (visible)
	{
		ImVec2	size = { get_float(objview, "width"s), get_float(objview, "height"s) };
		if (ImGui::Button(objview["text"].get_ref<string&>().c_str(), size))
			ExecEntity(EV, objview["clicked"], Value);	//	��������� � ������� ���������
	}
}


__declspec(dllexport) void __fastcall ImportRelationsModel(json &Ent)
{
	Addx86Entity(Ent["imgui"], "viewport"s, viewport, "");
	Addx86Entity(Ent["imgui"], "form"s, form, "");
	Addx86Entity(Ent["imgui"], "text"s, text, "");
	Addx86Entity(Ent["imgui"], "button"s, button, "");
}
