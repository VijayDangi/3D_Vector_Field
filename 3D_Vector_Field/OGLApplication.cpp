#include "OGLApplication.h"
#include "include/FPSCamera.h"
#include "include/ShaderProgram.h"

#include <glm/gtc/noise.hpp>

#include <vector>

namespace OglApplication {

    enum ECameraMovement
    {
        Forward = 0x01,
        Backward = 0x02,
        Left = 0x04,
        Right = 0x08,
        Up = 0x10,
        Down = 0x20
    };

    struct
    {
        bool blendEnabled;
        GLenum blendSrcRGB;
        GLenum blendDstRGB;
        GLenum blendSrcAlpha;
        GLenum blendDstAlpha;
    } BlendState;

    struct Particle
    {
        glm::vec4 posiition;
        glm::vec4 velocity;
        float age;
        float maxAge;
        float padding[2];
    };

    FPSCamera gFPSCamera(0.1f, 1.0f, glm::vec3(0.0f, 10.0f, 40.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -15.0f);
    ECameraMovement cameraMovement = (ECameraMovement)0;
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 inverseProjectionMatrix = glm::mat4(1.0f);

    float zNearPlane = 0.1f;
    float zFarPlane = 200.0f;

    int windowWidth = 800;
    int windowHeight = 600;

    GLuint infiniteGridProgram = 0;
    GLuint emptyVAO = 0;
    bool gShowInfiniteGrid = false;

    //////////
    const int NUM_PARTICLES = 100000;
    const glm::vec3 MIN_AABB(-10.0f, -10.0f, -10.0f);
    const glm::vec3 MAX_AABB(10.0f, 10.0f, 10.0f);
    GLuint ssbo[2];

    int readIndex = 0;
    int writeIndex = 0;
    double totalTime = 0.0;

    GLuint computeShaderProgram = 0;
    bool pauseSimulation = false;
    
    //////////
    GLuint renderShaderProgram = 0;
    GLuint renderVAO[2]{};
    float particleSize = 1.0f;
    
    //////////
    const int VECTOR_FIELD_SIZE = 64;
    const float VECTOR_FIELD_CENTER = 64 * 0.5f;
    const int MAX_VELOCITY_FIELD_TYPE = 2;
    int currentVelocityFieldType = 0;
    GLuint vectorFieldTexture[MAX_VELOCITY_FIELD_TYPE] = {0};

    bool startSimulation = false;
    int numParticles = 0;
    int frameCount = 0;

    // GetRandomValue() : Generate values in [0, 1]
    float GetRandomValue()
    {
        //code
        return float(rand()) / float(RAND_MAX);
    }

    // GetRandomValueInRange()
    float GetRandomValueInRange(float min, float max)
    {
        //code
        return min + GetRandomValue() * (max - min);
    }

    glm::vec3 getLorenzVelocity(glm::vec3 pos) {
        // Standard Lorenz constants
        const float sigma = 10.0f;
        const float rho = 28.0f;
        const float beta = 8.0f / 3.0f;

        glm::vec3 velocity;
        
        // The Lorenz equations
        velocity.x = sigma * (pos.y - pos.x);
        velocity.y = pos.x * (rho - pos.z) - pos.y;
        velocity.z = (pos.x * pos.y) - (beta * pos.z);

        return velocity;
    }

    // Function Definitions
    static void generateVectorFieldTextures()
    {
        // code
        std::vector<glm::vec3> vectorFieldData0(VECTOR_FIELD_SIZE * VECTOR_FIELD_SIZE * VECTOR_FIELD_SIZE);
        std::vector<glm::vec3> vectorFieldData1(VECTOR_FIELD_SIZE * VECTOR_FIELD_SIZE * VECTOR_FIELD_SIZE);

        for(int z = 0; z < VECTOR_FIELD_SIZE; ++z)
        {
            for(int y = 0; y < VECTOR_FIELD_SIZE; ++y)
            {
                for(int x = 0; x < VECTOR_FIELD_SIZE; ++x)
                {
                    // Map the grid indices (0 to 63) to our physical simulation bounds (-10 to 10)
                    float px = MIN_AABB.x + (x / (float)(VECTOR_FIELD_SIZE - 1))  * (MAX_AABB.x - MIN_AABB.x);
                    float py = MIN_AABB.y + (y / (float)(VECTOR_FIELD_SIZE - 1)) * (MAX_AABB.y - MIN_AABB.y);
                    float pz = MIN_AABB.z + (z / (float)(VECTOR_FIELD_SIZE - 1))  * (MAX_AABB.z - MIN_AABB.z);
                    
                    int index = x + (y * VECTOR_FIELD_SIZE) + (z * VECTOR_FIELD_SIZE * VECTOR_FIELD_SIZE);

                        // 0
                    // Calculate the analytical velocity at this exact point
                    {
                        // (Using the core requirement example field)
                        glm::vec3 velocity;
                        velocity.x = sin(py);
                        velocity.y = cos(px);
                        velocity.z = sin(pz);
                        vectorFieldData0[index] = velocity;
                    }

                        // 1
                    {
                        // We scale X and Y by 3, and map Z from [-10, 10] to [0, 50]
                        glm::vec3 lorenzSpacePos;
                        lorenzSpacePos.x = px * 2.5f; 
                        lorenzSpacePos.y = py * 2.5f;
                        lorenzSpacePos.z = (pz + 10.0f) * 2.5f; // Shift Z up so it starts at 0

                        glm::vec3 velocity = getLorenzVelocity(lorenzSpacePos);
                        velocity *= 0.05f;

                        vectorFieldData1[index] = velocity;
                    }
                }
            }
        }

        // create 3D texture
        glGenTextures(MAX_VELOCITY_FIELD_TYPE, vectorFieldTexture);
            // 0
        {
            glBindTexture(GL_TEXTURE_3D, vectorFieldTexture[0]);

            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F, VECTOR_FIELD_SIZE, VECTOR_FIELD_SIZE, VECTOR_FIELD_SIZE, 0, GL_RGB, GL_FLOAT, &vectorFieldData0[0][0]);

            glBindTexture(GL_TEXTURE_3D, 0);
        }

            // 1
        {
            glBindTexture(GL_TEXTURE_3D, vectorFieldTexture[1]);

            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F, VECTOR_FIELD_SIZE, VECTOR_FIELD_SIZE, VECTOR_FIELD_SIZE, 0, GL_RGB, GL_FLOAT, &vectorFieldData1[0][0]);

            glBindTexture(GL_TEXTURE_3D, 0);
        }
    }

    bool Init()
    {
        // code
        // ================== Shaders Initialization ==================
            // Infinite Grid Shader Program
        ss_gl::SShadersInfo infiniteGridShaderInfo[] = {
            { GL_VERTEX_SHADER, ss_gl::EShaderLoadAs::File, "content/shaders/infinite_grid/vertex_shader.vert", 0 },
            { GL_FRAGMENT_SHADER, ss_gl::EShaderLoadAs::File, "content/shaders/infinite_grid/fragment_shader.frag", 0 },
        };

        infiniteGridProgram = CreateProgram( infiniteGridShaderInfo, _ARRAYSIZE(infiniteGridShaderInfo), nullptr, 0, nullptr);
        if( infiniteGridProgram == 0) { return false; }

        // Generate initial particles data
        std::vector<Particle> initialParticles(NUM_PARTICLES);
        for(int i = 0; i < NUM_PARTICLES; i++)
        {
            initialParticles[i].posiition = glm::vec4(GetRandomValueInRange(MIN_AABB.x, MAX_AABB.x), GetRandomValueInRange(MIN_AABB.y, MAX_AABB.y), GetRandomValueInRange(MIN_AABB.z, MAX_AABB.z), 1.0f);
            initialParticles[i].age = GetRandomValueInRange(0.0f, 5.0f); // Random initial age between 0 and 5 seconds
            initialParticles[i].velocity = glm::vec4(0.0f);
            // initialParticles[i].maxAge = MAX_AGE;
            initialParticles[i].maxAge = 5.0f + GetRandomValue() * 5.0f; // Random max age between 5 and 10 seconds
        }

            // Compute Shader Program
        ss_gl::SShadersInfo computeShaderInfo[] = { { GL_COMPUTE_SHADER, ss_gl::EShaderLoadAs::File, "content/shaders/vector_field/advect.comp", 0 } };
        computeShaderProgram = CreateProgram( computeShaderInfo, _ARRAYSIZE(computeShaderInfo), nullptr, 0, nullptr);
        if( computeShaderProgram == 0) { return false; }

            // Render Shader Program
        ss_gl::SShadersInfo renderShaderInfo[] = {
            { GL_VERTEX_SHADER, ss_gl::EShaderLoadAs::File, "content/shaders/vector_field/vertex_shader.vert", 0 },
            { GL_FRAGMENT_SHADER, ss_gl::EShaderLoadAs::File, "content/shaders/vector_field/fragment_shader.frag", 0 },
        };

        renderShaderProgram = CreateProgram( renderShaderInfo, _ARRAYSIZE(renderShaderInfo), nullptr, 0, nullptr);
        if( renderShaderProgram == 0) { return false; }

        // ===================== Buffers Initialization =====================
        // Create SSBO
        glGenBuffers(2, ssbo);

        for (int i = 0; i < 2; i++)
        {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[i]);
            glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(Particle), nullptr, GL_DYNAMIC_COPY);
        }

        // Upload the initial data to the first SSBO
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[0]);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(Particle), initialParticles.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        // Setup Vao for rendering particles
        glGenVertexArrays(2, renderVAO);
        
        for(int i = 0; i < 2; i++)
        {
            glBindVertexArray(renderVAO[i]);
            glBindBuffer(GL_ARRAY_BUFFER, ssbo[i]);

            // Position Attribute
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, posiition));

            // Velocity Attribute
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, velocity));

            // Age Attribute
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)offsetof(Particle, age));

            glBindVertexArray(0);
        }

        // Empty VAO. This is used for rendering the infinite grid which doesn't have any vertex attributes.
        glGenVertexArrays(1, &emptyVAO);

        // Generate the vector field texture
        generateVectorFieldTextures();

        // Initial Render State
        glEnable(GL_DEPTH_TEST);
        // glDepthFunc(GL_LEQUAL);
        // glClearDepth(1.0f);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glEnable(GL_PROGRAM_POINT_SIZE);

        return true;
    }

    static void Update(double deltaTime)
    {
        // code
        totalTime += deltaTime;

        if (cameraMovement & ECameraMovement::Forward) { gFPSCamera.moveForward(deltaTime); }
        if (cameraMovement & ECameraMovement::Backward) { gFPSCamera.moveBackward(deltaTime); }
        if (cameraMovement & ECameraMovement::Left) { gFPSCamera.moveLeft(deltaTime); }
        if (cameraMovement & ECameraMovement::Right) { gFPSCamera.moveRight(deltaTime); }
        if (cameraMovement & ECameraMovement::Up) { gFPSCamera.moveUp(deltaTime); }
        if (cameraMovement & ECameraMovement::Down) { gFPSCamera.moveDown(deltaTime); }
    }

    void SaveBlendState()
    {
        BlendState.blendEnabled = glIsEnabled(GL_BLEND);
        glGetIntegerv(GL_BLEND_SRC_RGB, (GLint *)&BlendState.blendSrcRGB);
        glGetIntegerv(GL_BLEND_DST_RGB, (GLint *)&BlendState.blendDstRGB);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint *)&BlendState.blendSrcAlpha);
        glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint *)&BlendState.blendDstAlpha);
    }

    void RestoreBlendState()
    {
        if (BlendState.blendEnabled) { glEnable(GL_BLEND); } else { glDisable(GL_BLEND); }
        glBlendFuncSeparate(BlendState.blendSrcRGB, BlendState.blendDstRGB, BlendState.blendSrcAlpha, BlendState.blendDstAlpha);
    }

    void RenderImGui(double deltaTime)
    {
        ImGuiIO &io = ImGui::GetIO();

        if(startSimulation)
        {
            ImGui::Begin("Controls");
                ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

                ImGui::NewLine();

                ImGui::Text( "Camera Position: [ %f, %f, %f]", gFPSCamera.vPosition[0], gFPSCamera.vPosition[1], gFPSCamera.vPosition[2]);
                ImGui::Text( "Camera Pitch %f, Yaw: %f", gFPSCamera.fPitch, gFPSCamera.fYaw);
                ImGui::DragFloat( "Camera Movement Speed", &gFPSCamera.movementSpeed, 0.01f, 0.0f, 30.0f);
                ImGui::DragFloat( "Camera Rotate Speed", &gFPSCamera.mouseSensitivity, 0.01f, 0.0f, 20.0f);
                ImGui::NewLine();

                ImGui::Text("Delta Time: %f", deltaTime);
                ImGui::Text("Elapsed Time: %f", totalTime);

                ImGui::NewLine();
                ImGui::SliderInt("Velocity Field Type", &currentVelocityFieldType, 0, MAX_VELOCITY_FIELD_TYPE - 1);
                ImGui::SliderFloat("Particle Size", &particleSize, 0.5f, 20.0f);

                ImGui::NewLine();
                ImGui::Checkbox("Show Infinite Grid", &gShowInfiniteGrid);
                ImGui::Checkbox("Pause Simulation", &pauseSimulation);

                ImGui::NewLine();
                ImGui::Text("Particle Count: %d", numParticles);
            ImGui::End();
        }
        else
        {
            ImVec2 textSize = ImGui::CalcTextSize("Press the button below to start the simulation.");
            textSize.x += 40.0f; // Add some padding

            ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Once, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(ImVec2(textSize.x, 100.0f));

            ImGui::Begin("Startup Message", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
                ImGui::SetCursorPosX(20.0f);
                ImGui::Text("Press the button below to start the simulation.");

                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f);
                ImGui::SetCursorPosX((textSize.x - ImGui::CalcTextSize("Start Simulation").x) * 0.5f);
                if(ImGui::Button("Start Simulation"))
                {
                    startSimulation = true;
                }
            ImGui::End();
        }
    }

    void Render(double deltaTime)
    {
        // code
        if(!startSimulation)
        {
            return;
        }

        Update( deltaTime);

        // Rendering code
        glm::mat4 viewMatrix = gFPSCamera.getViewMatrix();
        glm::mat4 inverseViewMatrix = glm::inverse(viewMatrix);

        glm::mat4 viewProjectionMatrix = projectionMatrix * viewMatrix;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // ---------------------------------
        // 1. Compute Pass
        // ---------------------------------
        if(!pauseSimulation)
        {
            glUseProgram(computeShaderProgram);
            glUniform1f(glGetUniformLocation(computeShaderProgram, "u_time"), (float)totalTime);
            glUniform1f(glGetUniformLocation(computeShaderProgram, "u_deltaTime"), (float)deltaTime);
            glUniform3f(glGetUniformLocation(computeShaderProgram, "u_minAABB"), MIN_AABB.x, MIN_AABB.y, MIN_AABB.z);
            glUniform3f(glGetUniformLocation(computeShaderProgram, "u_maxAABB"), MAX_AABB.x, MAX_AABB.y, MAX_AABB.z);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_3D, vectorFieldTexture[currentVelocityFieldType]);

            // Bind Buffer A as Input (Read)
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo[readIndex]);
            // Bind Buffer B as Output (Write)
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo[writeIndex]);

            // Dispatch Compute Shader
            GLuint numGroups = (NUM_PARTICLES + 255) / 256; // Assuming local size of 256
            glDispatchCompute(numGroups, 1, 1);

            // Prevent the render pipeline from reading the SSBOs before the compute is done.
            glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
        }

        // ---------------------------------
        // 2. Render Pass
        // ---------------------------------
        glUseProgram(renderShaderProgram);

        glm::mat4 mvp = glm::rotate(glm::mat4(1.0f), glm::radians(float(totalTime) * 20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        mvp = viewProjectionMatrix * mvp;
        glUniformMatrix4fv(glGetUniformLocation(renderShaderProgram, "u_viewProjMatrix"), 1, GL_FALSE, &mvp[0][0]);
        glUniform1f(glGetUniformLocation(renderShaderProgram, "u_particleSize"), particleSize);
        glBindVertexArray(renderVAO[writeIndex]);
        glDrawArrays(GL_POINTS, 0, numParticles);
        glEnable(GL_POINT_SMOOTH);
        glBindVertexArray(0);

        if( numParticles < NUM_PARTICLES)
        {
            numParticles += 50;
            if(numParticles > NUM_PARTICLES)
            {
                numParticles = NUM_PARTICLES;
            }
        }


        // Swap read and write buffers for the next frame
        if(!pauseSimulation)
        {
            std::swap(readIndex, writeIndex);
        }

        // Draw Infinite Grid
        if(gShowInfiniteGrid)
        {
            SaveBlendState();

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glUseProgram(infiniteGridProgram);
            glUniformMatrix4fv(glGetUniformLocation(infiniteGridProgram, "u_view"), 1, GL_FALSE, &viewMatrix[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(infiniteGridProgram, "u_inverse_view"), 1, GL_FALSE, &inverseViewMatrix[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(infiniteGridProgram, "u_projection"), 1, GL_FALSE, &projectionMatrix[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(infiniteGridProgram, "u_inverse_projection"), 1, GL_FALSE, &inverseProjectionMatrix[0][0]);
            glUniform1f(glGetUniformLocation(infiniteGridProgram, "u_gridIntensity"), 0.2f);

            glBindVertexArray(emptyVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);

            RestoreBlendState();
        }
    }

    void Resize(unsigned int width, unsigned int height)
    {
        // code
        glViewport(0, 0, width, height);

        windowWidth = width;
        windowHeight = height;

        projectionMatrix = glm::perspectiveRH(glm::radians(45.0f), (float)windowWidth / (float)windowHeight, zNearPlane, zFarPlane);
        inverseProjectionMatrix = glm::inverse(projectionMatrix);
    }

    void KeyDownEventHandler(unsigned int key)
    {
        // code
        switch (key)
        {
            case 'W':
                cameraMovement = (ECameraMovement)(cameraMovement | ECameraMovement::Forward);
            break;

            case 'S':
                cameraMovement = (ECameraMovement)(cameraMovement | ECameraMovement::Backward);
            break;

            case 'A':
                cameraMovement = (ECameraMovement)(cameraMovement | ECameraMovement::Left);
            break;

            case 'D':
                cameraMovement = (ECameraMovement)(cameraMovement | ECameraMovement::Right);
            break;

            case 'Q':
                cameraMovement = (ECameraMovement)(cameraMovement | ECameraMovement::Down);
            break;

            case 'E':
                cameraMovement = (ECameraMovement)(cameraMovement | ECameraMovement::Up);
            break;
        }
    }

    void KeyUpEventHandler(unsigned int key)
    {
        // code
        switch (key)
        {
            case 'W':
                cameraMovement = (ECameraMovement)(cameraMovement & ~ECameraMovement::Forward);
            break;

            case 'S':
                cameraMovement = (ECameraMovement)(cameraMovement & ~ECameraMovement::Backward);
            break;

            case 'A':
                cameraMovement = (ECameraMovement)(cameraMovement & ~ECameraMovement::Left);
            break;

            case 'D':
                cameraMovement = (ECameraMovement)(cameraMovement & ~ECameraMovement::Right);
            break;

            case 'Q':
                cameraMovement = (ECameraMovement)(cameraMovement & ~ECameraMovement::Down);
            break;

            case 'E':
                cameraMovement = (ECameraMovement)(cameraMovement & ~ECameraMovement::Up);
            break;
        }
    }

    void MouseDownEventHandler(EMouseButton mouse)
    {
        // code
    }

    void MouseUpEventHandler(EMouseButton button)
    {
        // code
    }

    void MouseMoveEventHandler(EMouseButton button, unsigned int x, unsigned int y)
    {
        // code
        static int mousePosX, mousePosY;
        int mouseDx, mouseDy;
        // int newX = -1, newY = -1;
        // POINT pt;

        // code
        ImGuiIO &io = ImGui::GetIO();
        if(!io.WantCaptureMouse)
        {
            mouseDx = mousePosX - x;
            mouseDy = mousePosY - y;

            if( abs(mouseDx) >= windowWidth / 2) { mouseDx = 0; }
            if( abs(mouseDy) >= windowHeight / 2) { mouseDy = 0; }


            if( button & EMouseButton::Left)   //calculate camera pitch
            {
                float pitchChange = mouseDy * 0.1f;
                float yawChange = -mouseDx * 0.1f;
                gFPSCamera.rotate(pitchChange, yawChange);
            }
        }

        mousePosX = x;
        mousePosY = y;

        // //set current mouse position
        // if( ( button & MK_LBUTTON ) || ( button & MK_RBUTTON ) || ( button & MK_MBUTTON ))
        // {
        //     newX = WrapInt( mouseX, 22, g_window_width - 22);
        //     newY = WrapInt( mouseY, 22, g_window_height - 22);

        //     if((newX != mouseX) || (newY != mouseY))
        //     {
        //         pt.x = newX;
        //         pt.y = newY;

        //         ClientToScreen( hwnd, &pt);
        //         SetCursorPos( pt.x, pt.y);
                
        //         mousePosX = newX;
        //         mousePosY = newY;
        //     }
        // }
    }

    void MouseWheelEventHandler(EMouseButton button, unsigned int x, unsigned int y, float delta)
    {
        // code
    }

    void Destroy()
    {
        // code
        if(vectorFieldTexture[0] != 0)
        {
            glDeleteTextures(4, vectorFieldTexture);
            for(int i = 0; i < 4; i++)
            {
                vectorFieldTexture[i] = 0;
            }
        }

        for (int i = 0; i < 2; i++)
        {
            if (ssbo[i] != 0)
            {
                glDeleteBuffers(1, &ssbo[i]);
                ssbo[i] = 0;
            }

            if( renderVAO[i] != 0)
            {
                glDeleteVertexArrays(1, &renderVAO[i]);
                renderVAO[i] = 0;
            }
        }

        if(emptyVAO != 0)
        {
            glDeleteVertexArrays(1, &emptyVAO);
            emptyVAO = 0;
        }

        if(renderShaderProgram)
        {
            ss_gl::DeleteProgram(renderShaderProgram);
            renderShaderProgram = 0;
        }

        if(computeShaderProgram)
        {
            ss_gl::DeleteProgram(computeShaderProgram);
            computeShaderProgram = 0;
        }

        if(infiniteGridProgram)
        {
            ss_gl::DeleteProgram(infiniteGridProgram);
            infiniteGridProgram = 0;
        }
    }
};
