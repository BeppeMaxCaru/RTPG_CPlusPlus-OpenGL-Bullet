#include <chrono>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"

#include "../utilsV2/ModelV2.h"

#include "../utilsV2/PhysicsV2.h"

/////////////////////////////////////////////////////////
//Functions declarations

//Link functions to GLFWindow
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

//Render soft body
void drawSoftBody(Shader& shader, CamV2& camera, btSoftBody& softBody, glm::vec3 softBodyColor);

//Generate mesh
MeshV2 getSoftBodyMesh(btSoftBody* softBody, glm::vec3 softBodyColor);

/////////////////////////////////////////////////////////
//Setup values

//Screen settings
const GLuint SCR_WIDTH = 1200;
const GLuint SCR_HEIGHT = 600;

//Time calculation
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

//Soft bodies physics class
PhysicsV2 physics;

//Soft bodies attributes to generate and render them
vector<MeshV2> generatedSoftBodiesMeshes;
vector<glm::vec3> softBodiesColours;

//Main function
int main() {

    ////////////////////////////////////////////////////
    //OpenGL preliminary operations (version, window, context, ecc.)

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "RTPG Project Carugno", NULL, NULL);
    if (!window) 
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    gladLoadGL();
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    ////////////////////////////////////////////////////
    //Shader setup

    //Read vertex and fragment shaders
    //Generate and link shader program
    Shader shaderProgram("Shaders/basic.vert", "Shaders/basic.frag");

    //////////////////////////////////////////////////////
    //Application loading

    //Enable depth test
    glEnable(GL_DEPTH_TEST);

    //Create camera in starting position
    CamV2 CamV2(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0f, 0.0f, 2.0f));

    //Import models at the start
    //Simple models
    ModelV2 planeModel("models/cube.obj");
    ModelV2 cubeModel("models/cube.obj");
    ModelV2 sphereModel("models/sphere.obj");
    //More complex models
    //Longer loading times
    //ModelV2 bunnyModel("models/bunny_lp.obj");
    //ModelV2 yodaModel("models/babyyoda.obj");

    /////////////////////////////////////////////////////////
    //Bullet setup

    //Setup simulated world
    physics.setupPhysics();

    //Generate world plane
    glm::vec3 planeScale = glm::vec3(50.0f, 0.1f, 50.0f);
    btRigidBody* plane = physics.genWorldPlane(planeScale, 0.0f);

    ///////////////////////////////////////////////////
    //Time steps

    //Set the maximum delta time for the update of the physical simulation
    GLfloat maxSecPerFrame = 1.0f / 60.0f;

    //////////////////////////////////////////////////////////
    //GUI

    //GUI setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410 core");

    //GUI parameters collection for generating soft bodies!
    static int selectedModel = NULL;
    vector<const char*> availableModels = { "Cube", "Sphere" };

    float position[3] = { 0.0f, 3.0f, 0.0f };
    float rotation[3] = { 0.0f, 0.0f, 0.0f };
    float scale[3] = { 1.0f, 1.0f, 1.0f };
    float mass = 100.0f;

    float color[3] = { 1.0f, 1.0f, 1.0f};

    float internalPressure = 100.0f;

    bool generate = false;

    //Frame rate monitor
    auto startTime = chrono::high_resolution_clock::now();

    ////////////////////////////////////////////////////////////////////////
    //Rendering loop!

    while (!glfwWindowShouldClose(window)) 
    {

        processInput(window);

        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.0f, 0.9f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //////////////////////////////////////////////////
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //Unlink mouse from OpenGL window when I'm above the GUI window
        if (!io.WantCaptureMouse)
        {
            CamV2.Inputs(window);
        }

        //Step simulation forward
        physics.world->stepSimulation((deltaTime < maxSecPerFrame ? deltaTime : maxSecPerFrame), 10);

        //Activate shader program
        shaderProgram.Use();

        //CamV2.Inputs(window);
        //Updates and exports the camera matrix to the Vertex Shader
        CamV2.updateMatrix(45.0f, 0.1f, 1000.0f);

        /////////////////////////////////////////////////////////

        //Static world plane
        btTransform t;
        plane->getMotionState()->getWorldTransform(t);
        planeModel.meshes[0].Draw(shaderProgram, CamV2, glm::mat4(1.0f),
            glm::vec3(t.getOrigin().x(), t.getOrigin().y(), t.getOrigin().z()),
            glm::quat(t.getRotation().x(), t.getRotation().y(), t.getRotation().z(), t.getRotation().w()),
            planeScale);
        //cout << planeModel.meshes.size() << "piano" << endl;

        /////////////////////////////////////////////
        //GUI
        
        //Helper
        //ImGui::ShowDemoWindow();

        //Start GUI
        ImGui::Begin("Soft bodies generator");

        //Here there are the GUI widgets

        //Select model
        ImGui::Combo("Model", &selectedModel, availableModels.data(), availableModels.size());
        //Position
        ImGui::InputFloat3("Position", position, "%.2f");
        //Rotations
        ImGui::SliderFloat("Yaw", &rotation[0], -180.0f, 180.0f, "%.2f", 0);
        ImGui::SliderFloat("Pitch", &rotation[1], -90.0f, 90.0f, "%.2f", 0);
        ImGui::SliderFloat("Roll", &rotation[2], -180.0f, 180.0f, "%.2f", 0);
        //Colour selection
        ImGui::ColorEdit3("Color", color, ImGuiColorEditFlags_Float);
        //Mass
        ImGui::DragFloat("Mass", &mass, 0.005f, 0.0f, FLT_MAX, "%.2f", 0);
        //Internal pressure
        ImGui::DragFloat("Internal pressure", &internalPressure, 0.005f, 0.0f, FLT_MAX, "%.2f", 0);
        
        //When clicked spawn new body
        generate = ImGui::Button("Generate");

        //Stop accepting inputs
        ImGui::End();

        //GUI rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        //Generate a soft body using all parameters passed to the GUI when generate is true
        if (generate == true) cout << "button pressed" << endl;
        if (generate == true)
        {
            //Generate softBody
            btSoftBody* softBody{};
            if (selectedModel == 0)
                softBody = physics.generateSoftBodyTest(cubeModel, position, rotation, scale, mass, internalPressure);
            else if (selectedModel == 1)
                softBody = physics.generateSoftBodyTest(sphereModel, position, rotation, scale, mass, internalPressure);
            //Generate its colours
            softBodiesColours.push_back(glm::make_vec3(color));

            //Retrieve soft body mesh
            MeshV2 softBodyMesh = getSoftBodyMesh(softBody, glm::make_vec3(color));
            generatedSoftBodiesMeshes.push_back(softBodyMesh);
        }

        //Switch generate to false otherwise bodies keep being generated!
        generate = false;

        /////////////////////////////////////////////////////////////////////
        //Frame rate evaluation
        //Calculate elapsed time
        auto elapsedTime = std::chrono::high_resolution_clock::now() - startTime;
        startTime = std::chrono::high_resolution_clock::now();
        //Calculate frame rate
        double frameRate = 1.0 / std::chrono::duration_cast<std::chrono::duration<double>>(elapsedTime).count();
        //Print the frame rate
        std::cout << "Frame rate: " << frameRate << std::endl;

        //////////////////////////////////////////////////////////////////////
        //Rendering

        //Soft bodies only (to speed up development)
        for (int i = 0; i < physics.world->getSoftBodyArray().size(); i++)
        {
            btSoftBody* softBodyToDraw = physics.world->getSoftBodyArray()[i];
            drawSoftBody(shaderProgram, CamV2, *softBodyToDraw, softBodiesColours[i]);
        }

        ///////////////////////////////
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    /////////////////////////////////////////////////////////

    //Free resources
    //GUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    //Shader program
    shaderProgram.Delete();

    //Physics
    physics.deletePhysics();

    //OpenGL
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;

}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

//Rendering function
void drawSoftBody(Shader& shader, CamV2& camera, btSoftBody& softBody, glm::vec3 softBodyColor)
{
    //Get soft body vertices
    vector<Vertex> softVertices;
    //Map vertices
    unsigned int index = 0;

    //Recover soft body indices using mapping (since soft bodies in Bullet do not have them)
    vector<GLuint> softIndices;

    //Reserve size to save time and avoid reallocations
    softVertices.reserve(softBody.m_nodes.size());
    softIndices.reserve(softBody.m_faces.size() * 3);


    //Get soft body vertices
    //BY ITERATING OVER NODES; NOT FACES!!!!!
    for (unsigned int i = 0; i < softBody.m_nodes.size(); i++)
    {
        Vertex vertex;

        vertex.Position = glm::vec3(
            softBody.m_nodes[i].m_x.x(),
            softBody.m_nodes[i].m_x.y(),
            softBody.m_nodes[i].m_x.z()
        );

        vertex.Normal = glm::vec3(
            softBody.m_nodes[i].m_n.x(),
            softBody.m_nodes[i].m_n.y(),
            softBody.m_nodes[i].m_n.z()
        );

        vertex.Color = softBodyColor;

        //Create vertices list
        softVertices.push_back(vertex);

    }

    //Get soft body indices
    unsigned int numNodes = softBody.m_nodes.size();

    for (unsigned int i = 0; i < softBody.m_faces.size(); i++)
    {
        btSoftBody::Face face = softBody.m_faces[i];
        for (int j = 0; j < 3; j++)
        {
            btSoftBody::Node* node = face.m_n[j];
            for (int k = 0; k < numNodes; k++)
            {
                if (node == &softBody.m_nodes[k])
                {
                    softIndices.push_back(static_cast<GLuint>(k));
                    break;
                }
            }
        }
    }

    //V1
    //Li passo alla GPU per disegnare il softbody
    //MeshV2 mesh(softVertices, softIndices);
    //mesh.Draw(shader, camera);

    //V2
    MeshV2 mesh2(softVertices, softIndices, 0);
    mesh2.drawV2(shader, camera);

}

MeshV2 getSoftBodyMesh(btSoftBody* softBody, glm::vec3 softBodyColor)
{
    //Get soft body vertices
    vector<Vertex> softVertices;
    //Vertices mapping
    unsigned int index = 0;

    //Build indices correctly using the mapping (since missing in the Bullet soft body)
    vector<GLuint> softIndices;

    //Reserve size to save time and avoid reallocations
    softVertices.reserve(softBody->m_nodes.size());
    softIndices.reserve(softBody->m_faces.size() * 3);

    //Recover soft body vertices components
    //DO IT OVER THE NODES, NOT THE FACES!!!!!

    //Get vertices
    for (unsigned int i = 0; i < softBody->m_nodes.size(); i++)
    {
        Vertex vertex;

        vertex.Position = glm::vec3(
            softBody->m_nodes[i].m_x.x(),
            softBody->m_nodes[i].m_x.y(),
            softBody->m_nodes[i].m_x.z()
        );

        vertex.Normal = glm::vec3(
            softBody->m_nodes[i].m_n.x(),
            softBody->m_nodes[i].m_n.y(),
            softBody->m_nodes[i].m_n.z()
        );

        vertex.Color = softBodyColor;

        //Create vertices list
        softVertices.push_back(vertex);
        //Prepare indexing
        //Insert node and corresponding assigned index

    }

    //Get indices
    unsigned int numNodes = softBody->m_nodes.size();

    for (unsigned int i = 0; i < softBody->m_faces.size(); i++)
    {
        btSoftBody::Face face = softBody->m_faces[i];
        for (int j = 0; j < 3; j++)
        {
            btSoftBody::Node* node = face.m_n[j];
            for (int k = 0; k < numNodes; k++)
            {
                if (node == &softBody->m_nodes[k])
                {
                    softIndices.push_back(static_cast<GLuint>(k));
                    break;
                }
            }
        }
    }

    return MeshV2(softVertices, softIndices);

}

