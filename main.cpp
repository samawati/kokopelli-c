#include <chrono>
#include <iostream>
#include <fstream>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "TextEditor.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glcanvas.h"

static void 
glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "glfw Error %d: %s\n", error, description);
}
int main(int argc, char** argv) {
    struct glcanvas_t *g = glcanvas_create("Preview", (Vec2f){ 640, 480 });
    ImGui::CreateContext();
    GLFWwindow* window = glfwCreateWindow(640, 720, "Code Editor", NULL, NULL);
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowBorderSize = 0.0f;
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0627451, 0.0627451, 0.0627451, 1);
    io.Fonts->AddFontFromFileTTF("../Inconsolata.ttf", 16.0f);
    TextEditor editor;
    auto lang = TextEditor::LanguageDefinition::Lua();
    editor.SetLanguageDefinition(lang);
    bool from_file = false, needs_eval = true, result_valid = false, just_saved = false;
    if (argc > 1) {
        std::ifstream input(argv[1]);
        if (input.is_open()) {
            std::vector<std::string> lines;
            std::string line;
            while (std::getline(input, line)) {
                lines.emplace_back(std::move(line));
            }
            input.close();
            editor.SetTextLines(lines);
            from_file = true;
        } else {
            std::cerr << "Could not open file '" << argv[1] << "'\n";
        }
    }
    bool active = false;
    while (!glfwWindowShouldClose(g->window) && !glfwWindowShouldClose(window)) {
        glfwWaitEventsTimeout(0.1f);
        int display_w, display_h;
        glfwMakeContextCurrent(window);
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            glfwGetFramebufferSize(window, &display_w, &display_h);
            if (!io.WantCaptureKeyboard) {
                if (io.KeyCtrl && io.KeysDown[GLFW_KEY_S]) {
                    if (!just_saved && from_file) {
                        std::ofstream output(argv[1]);
                        if (output.is_open()) {
                            for (auto& line: editor.GetTextLines()) {
                                output << line << "\n";
                            }
                            output.close();
                        } else {
                            std::cerr << "Failed to save to '" << argv[1] << "'\n";
                        }
                        just_saved = true;
                    }
                } else {
                    just_saved = false;
                }
            }
            ImGui::Begin("Text editor", nullptr, ImGuiWindowFlags_NoTitleBar 
                    | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar
                    | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar);
            	if (ImGui::BeginMenuBar()) {
                    if (ImGui::BeginMenu("File")) {
                        if (ImGui::MenuItem("New", "Ctrl-N")) break; 
                        ImGui::Separator();
                        if (ImGui::MenuItem("Open", "Ctrl+O")) break;
                        if (ImGui::MenuItem("Reload", "Ctrl+R")) break; 
                        ImGui::Separator();
                        if (ImGui::MenuItem("Save", "Ctrl+S")) break;
                        if (ImGui::MenuItem("Save As", "Shift+Ctrl+S")) break;
                        ImGui::Separator();  
                        if (ImGui::MenuItem("About")) break;    
                        if (ImGui::MenuItem("Quit", "Ctrl+Q")) break;
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Edit")) {
                        bool ro = editor.IsReadOnly();
                        if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
                            editor.SetReadOnly(ro);
                        ImGui::Separator();
                        if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor.CanUndo()))
                            editor.Undo();
                        if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
                            editor.Redo();
                        ImGui::Separator();
                        if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
                            editor.Copy();
                        if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
                            editor.Cut();
                        if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
                            editor.Delete();
                        if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
                            editor.Paste();
                        ImGui::Separator();
                        if (ImGui::MenuItem("Select all", nullptr, nullptr))
                            editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));
                        ImGui::EndMenu();
                    }                   
                    if (ImGui::BeginMenu("View")) {
                        if (ImGui::MenuItem("Show output", "Ctrl+D")) break;
                        if (ImGui::MenuItem("Show script", "Ctrl+T")) break;
                        ImGui::Separator();
                        if (ImGui::BeginMenu("Shading mode")) {
                            if(ImGui::RadioButton("Shaded", &g->current_3dshader, SHADED_SHADER) ||
                                ImGui::RadioButton("Wireframe", &g->current_3dshader, WIREFRAME_SHADER) ||
                                    ImGui::RadioButton("Normals", &g->current_3dshader, NORMALS_SHADER) ||
                                        ImGui::RadioButton("Subdivision", &g->current_3dshader, SUB_SHADER)) g->redraw = true;
                            ImGui::EndMenu();
                        }
                        ImGui::Separator();
                        if (ImGui::MenuItem("Show axes")) break;
                        if (ImGui::MenuItem("Show bounds")) break;
                        if (ImGui::MenuItem("Show traverses")) break;
                        ImGui::Separator();
                        if (ImGui::MenuItem("Re-render", "Ctrl+Return")) break;
                        ImGui::EndMenu();
                    }                 
                    if (ImGui::BeginMenu("Export")) {
                        if (ImGui::MenuItem(".png")) break;
                        if (ImGui::MenuItem(".svg")) break;
                        if (ImGui::MenuItem(".stl")) break;
                        if (ImGui::MenuItem(".dot")) break;
                        ImGui::Separator();
                        if (ImGui::MenuItem(".asdf")) break;
                        ImGui::Separator();
                        if (ImGui::MenuItem("Show CAM panel", "Ctrl+M")) break;
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }
                if (needs_eval)
                    result_valid = glcanvas_reeval(g, editor.GetText().c_str());
                if (result_valid == false)
                    display_h -= ImGui::GetFrameHeight();
                needs_eval = just_saved;
                editor.Render("TextEditor", ImVec2(display_w-15, display_h-35));
                if (result_valid == false)
                    ImGui::Text("%s", lua_tostring (g->L, -1));
            ImGui::End();

            ImGui::Render();
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.0627451, 0.0627451, 0.0627451, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);
        }
        glfwMakeContextCurrent(g->window);
        {
            glcanvas_poll(g);
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(g->window);
    glfwTerminate();
}
