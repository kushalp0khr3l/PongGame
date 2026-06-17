#define SDL_MAIN_HANDLED
#include <glad/glad.h>
#include <SDL3/SDL.h>
#include <iostream>
#include <cmath>
#include <string>

// --- IMGUI INCLUDES ---
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

// Solid color shaders for our game shapes
const char* vertexShaderSource = R"(
    #version 460 core
    layout (location = 0) in vec2 aPos;
    uniform vec2 uOffset; 
    void main() {
        gl_Position = vec4(aPos.x + uOffset.x, aPos.y + uOffset.y, 0.0, 1.0);
    }
)";

const char* fragmentShaderSource = R"(
    #version 460 core
    out vec4 FragColor;
    uniform vec4 uColor;
    void main() {
        FragColor = uColor;
    }
)";

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return -1;

    SDL_Window* window = SDL_CreateWindow("SDL3 Pong + Dear ImGui", 800, 600, SDL_WINDOW_OPENGL);
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

    SDL_GL_SetSwapInterval(1); // VSync

    // ==========================================
    // 1. INITIALIZE DEAR IMGUI
    // ==========================================
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark(); // Sleek dark theme

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init("#version 460");
    // ==========================================

    // Compile Game Shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Paddle Geometry Data
    float paddleVertices[] = {
        -0.9f,  0.2f, -0.85f, 0.2f, -0.85f, -0.2f,
        -0.9f,  0.2f, -0.85f, -0.2f, -0.9f, -0.2f,
        0.85f,  0.2f,  0.9f,   0.2f,  0.9f,  -0.2f,
        0.85f,  0.2f,  0.9f,  -0.2f, 0.85f, -0.2f
    };

    unsigned int paddleVAO, paddleVBO;
    glGenVertexArrays(1, &paddleVAO);
    glGenBuffers(1, &paddleVBO);
    glBindVertexArray(paddleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, paddleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(paddleVertices), paddleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Ball Geometry Data (Triangle Fan Circle)
    const int segments = 32;
    const float PI = 3.1415926f;
    float radius = 0.02f;
    float ballVertices[(segments + 2) * 2];
    ballVertices[0] = 0.0f; ballVertices[1] = 0.0f;
    for (int i = 0; i <= segments; i++) {
        float angle = i * (2.0f * PI / segments);
        ballVertices[(i + 1) * 2] = radius * cosf(angle);
        ballVertices[(i + 1) * 2 + 1] = radius * sinf(angle);
    }

    unsigned int ballVAO, ballVBO;
    glGenVertexArrays(1, &ballVAO);
    glGenBuffers(1, &ballVBO);
    glBindVertexArray(ballVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ballVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ballVertices), ballVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Gameplay States
    float leftPaddleY = 0.0f;
    float rightPaddleY = 0.0f;
    float paddleSpeed = 0.015f;

    float ballX = 0.0f; float ballY = 0.0f;
    float ballSpeedX = 0.01f; float ballSpeedY = 0.007f;
    bool gameStarted = false;
    bool ended = false;

    int leftScore = 0;
    int rightScore = 0;

    // Customizable color state for ImGui slider
    float ballColor[4] = { 0.0f, 1.0f, 1.0f, 1.0f };

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // Forward events to ImGui first (stops keys typing into game if UI is active)
            ImGui_ImplSDL3_ProcessEvent(&event);
            if(ended){
                if (event.type == SDL_EVENT_KEY_DOWN) {
                    if (event.key.scancode == SDL_SCANCODE_SPACE) {

                        rightScore = 0;
                        leftScore = 0;
                    }
                }
            }
            ended = false;

            if (event.type == SDL_EVENT_QUIT) running = false;
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.scancode == SDL_SCANCODE_SPACE) {

                    gameStarted = true;
                }
            }
        }

        //2. START IMGUI FRAME
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        // Only read movement inputs if the user isn't clicking/typing inside an ImGui window
        if (!io.WantCaptureKeyboard) {
            const bool* keys = SDL_GetKeyboardState(NULL);
            if (keys[SDL_SCANCODE_W]) leftPaddleY += paddleSpeed;
            if (keys[SDL_SCANCODE_S]) leftPaddleY -= paddleSpeed;
            if (keys[SDL_SCANCODE_UP]) rightPaddleY += paddleSpeed;
            if (keys[SDL_SCANCODE_DOWN]) rightPaddleY -= paddleSpeed;
        }

        if (leftPaddleY > 0.8f)  leftPaddleY = 0.8f;
        if (leftPaddleY < -0.8f) leftPaddleY = -0.8f;
        if (rightPaddleY > 0.8f)  rightPaddleY = 0.8f;
        if (rightPaddleY < -0.8f) rightPaddleY = -0.8f;

        // Ball Physics & Movement
        if (gameStarted) {
            ballX += ballSpeedX;
            ballY += ballSpeedY;

            if (ballY + radius >= 1.0f || ballY - radius <= -1.0f) ballSpeedY = -ballSpeedY;

            if (ballX - radius <= -0.85f && ballX + radius >= -0.9f) {
                if (ballY <= leftPaddleY + 0.2f && ballY >= leftPaddleY - 0.2f) {
                    ballSpeedX = -ballSpeedX; ballX = -0.83f;
                }
            }
            if (ballX + radius >= 0.85f && ballX - radius <= 0.9f) {
                if (ballY <= rightPaddleY + 0.2f && ballY >= rightPaddleY - 0.2f) {
                    ballSpeedX = -ballSpeedX; ballX = 0.83f;
                }
            }

            if (ballX > 1.0f || ballX < -1.0f) {
                if (ballX > 1.0f) { leftScore++;
                leftPaddleY = 0.0f;
                rightPaddleY = 0.0f;
                
                }
                else { rightScore++; 
                 leftPaddleY = 0.0f;
                rightPaddleY = 0.0f;
                
                }
                ballX = 0.0f; ballY = 0.0f;
                ballSpeedX = -ballSpeedX;

                gameStarted = false;
            }
            
        }
        
        if (leftScore == 10 && rightScore < leftScore) {
            gameStarted = false;
            ImGui::SetNextWindowPos(ImVec2(400.0f, 300.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::Begin("Result", nullptr, ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoScrollbar);
            ImGui::SetWindowFontScale(2.0f);
            ImGui::Text("LeftPlayer Won!! \n Results: L = %d R = %d ", leftScore, rightScore);
            ended = true;

            ImGui::End();
            
            


        }
        if (rightScore == 10 && rightScore > leftScore) {
            gameStarted = false;

            ImGui::SetNextWindowPos(ImVec2(400.0f, 300.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            ImGui::Begin("Result", nullptr, ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoScrollbar);
            ImGui::SetWindowFontScale(2.0f);
            ImGui::Text("RightPlayer Won!! \n Results: L = %d R = %d ", leftScore, rightScore);
            ended = true;
            ImGui::End();
              
            



        }
        if(ended) {
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.scancode == SDL_SCANCODE_SPACE) {

                    rightScore = 0;
                    leftScore = 0;
                }
            }
        }
        

        // ==========================================
        // 2. START IMGUI FRAME
        // ==========================================
        //ImGui_ImplOpenGL3_NewFrame();
        //ImGui_ImplSDL3_NewFrame();
        //ImGui::NewFrame();

        // DRAW UI ELEMENT A: Transparency Scoreboard Overlay
        ImGui::SetNextWindowPos(ImVec2(280, 20), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(240, 60), ImGuiCond_Always);
        ImGui::Begin("Scores", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::SetWindowFontScale(2.5f); // Crisp text enlargement
        ImGui::Text("%d   |   %d", leftScore, rightScore);
        ImGui::End();

        // DRAW UI ELEMENT B: A Live Engine Tweaker Panel
        //ImGui::Begin("Engine Control Settings");
        //ImGui::Text("Application Average: %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        //ImGui::Separator();
        //ImGui::SliderFloat("Ball Speed X", &ballSpeedX, -0.03f, 0.03f);
        //ImGui::ColorEdit4("Ball Color Selector", ballColor);
        //if (ImGui::Button("Reset Match Score")) {
        //    leftScore = 0;
        //    rightScore = 0;
        //}
        //ImGui::End();
        // ==========================================

        if (!gameStarted && !ended) {
            // This centers the text window right in the middle of a 800x600 screen
            ImGui::SetNextWindowPos(ImVec2(400.0f, 300.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

            // We open a window with flags to make it completely invisible except for the text
            ImGui::Begin("Start Menu", nullptr, ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoScrollbar);
            ImGui::SetWindowFontScale(2.0f);
            ImGui::Text("PRESS SPACE TO START GAME");

            ImGui::End();
        }

        // --- RENDER GAME GRAPHICS (Raw OpenGL) ---
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        int offsetLoc = glGetUniformLocation(shaderProgram, "uOffset");
        int colorLoc = glGetUniformLocation(shaderProgram, "uColor");

        // Draw Paddles
        glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
        glBindVertexArray(paddleVAO);
        glUniform2f(offsetLoc, 0.0f, leftPaddleY);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glUniform2f(offsetLoc, 0.0f, rightPaddleY);
        glDrawArrays(GL_TRIANGLES, 6, 6);

        // Draw Ball using the color linked to the ImGui color wheel variable
        glUniform4f(colorLoc, ballColor[0], ballColor[1], ballColor[2], ballColor[3]);
        glUniform2f(offsetLoc, ballX, ballY);
        glBindVertexArray(ballVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2);

        // ==========================================
        // 3. RENDER IMGUI DRAW DATA OVER GAME
        // ==========================================
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // ==========================================

        SDL_GL_SwapWindow(window);
    }

    // --- CLEANUP IMGUI ---
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    // Game Resource Cleanup
    glDeleteVertexArrays(1, &paddleVAO);
    glDeleteBuffers(1, &paddleVBO);
    glDeleteVertexArrays(1, &ballVAO);
    glDeleteBuffers(1, &ballVBO);
    glDeleteProgram(shaderProgram);

    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}