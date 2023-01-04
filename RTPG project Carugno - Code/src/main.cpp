#include <chrono>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"

#include "../utilsV2/ModelV2.h"

#include "../utilsV2/PhysicsV2.h"

//Function declaration
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

void drawSoftBody(Shader& shader, CamV2& camera, btSoftBody& softBody, glm::vec3 softBodyColor);

//NEW
MeshV2 getSoftBodyMesh(btSoftBody* softBody, glm::vec3 softBodyColor);


// settings
const GLuint SCR_WIDTH = 1200;
const GLuint SCR_HEIGHT = 600;

// parameters for time calculation (for animations)
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

//NEW
PhysicsV2 physics;

vector<MeshV2> generatedSoftBodiesMeshes;
vector<glm::vec3> softBodiesColours;

int main() {

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

    Shader shaderProgram("Shaders/basic.vert", "Shaders/basic.frag");

    //////////////////////////////////////////////////////

    glEnable(GL_DEPTH_TEST);

    CamV2 CamV2(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.0f, 0.0f, 2.0f));

    //Import models at the start
    //Simpler models
    ModelV2 planeModel("models/cube.obj");
    ModelV2 cubeModel("models/cube.obj");
    ModelV2 sphereModel("models/sphere.obj");
    //More complex models
    //ModelV2 bunnyModel("models/bunny_lp.obj");
    //ModelV2 yodaModel("models/babyyoda.obj");

    /////////////////////////////
    //Setup simulated world
    physics.setupPhysics();

    //Works + rigid bodies
    glm::vec3 planeScale = glm::vec3(50.0f, 0.1f, 50.0f);
    btRigidBody* plane = physics.genWorldPlane(planeScale, 0.0f);




    /////////////////////////////////////////////////

    //GenV3 now working correctly!
    // genSoftSphereV3 worked correctly
    // We just split it by moving the procedure that merges meshes into
    // the Model conostructor so as soon as the model is imported also the unique mesh
    // is generated.
    // The part instead where we actually generate the soft body has been 
    // refactored as the function generateSoftBodyFromModel so that it doesn't have to generate
    // again the unique mesh but it can simply take a model and from it generate the corresponding
    // soft body!

    //This version is V4 and includes optimization and refactoring
    //softTest = physics.generateSoftBodyTest(sphereModel);

    ///////////////////////////////////////////////////

    // we set the maximum delta time for the update of the physical simulation
    GLfloat maxSecPerFrame = 1.0f / 60.0f;

    //////////////////////////////////////////////////////////
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410 core");

    //ImGui parameters for generating soft bodies!

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

        //Unlink mouse from OpenGL window when I'm above the gui window
        if (!io.WantCaptureMouse)
        {
            CamV2.Inputs(window);
        }

        //Simulation steps
        physics.world->stepSimulation((deltaTime < maxSecPerFrame ? deltaTime : maxSecPerFrame), 10);

        shaderProgram.Use();

        //CamV2.Inputs(window);
        // Updates and exports the camera matrix to the Vertex Shader
        CamV2.updateMatrix(45.0f, 0.1f, 1000.0f);


        /////////////////////////////////////////////////////////

        //Funzionaaaaaaaaaaaaaaaaaaaa!!!!!!!!!!!!!!!!!!!!!!!!!
        //Static world plane
        btTransform t;
        plane->getMotionState()->getWorldTransform(t);
        planeModel.meshes[0].Draw(shaderProgram, CamV2, glm::mat4(1.0f),
            glm::vec3(t.getOrigin().x(), t.getOrigin().y(), t.getOrigin().z()),
            glm::quat(t.getRotation().x(), t.getRotation().y(), t.getRotation().z(), t.getRotation().w()),
            planeScale);
        //cout << planeModel.meshes.size() << "piano" << endl;

        ///////////////////////////////////////
        //Helper
        //ImGui::ShowDemoWindow();

        ImGui::Begin("Soft bodies generator");

        //Here there are the widgets for the gui

        //Select model
        ImGui::Combo("Model", &selectedModel, availableModels.data(), availableModels.size());
        //Position
        ImGui::InputFloat3("Position", position, "%.2f");
        //Rotation
        //ImGui::InputFloat3("Rotation", rotation, "%.2f");
        ImGui::SliderFloat("Yaw", &rotation[0], -180.0f, 180.0f, "%.2f", 0);
        ImGui::SliderFloat("Pitch", &rotation[1], -90.0f, 90.0f, "%.2f", 0);
        ImGui::SliderFloat("Roll", &rotation[2], -180.0f, 180.0f, "%.2f", 0);
        //Scale
        //ImGui::DragFloat3("Scale", scale, 0.005f, 0.0f, FLT_MAX, "%.2f", 0);
        //Colours -> at the moment no way to assign a color to each soft body
        ImGui::ColorEdit3("Color", color, ImGuiColorEditFlags_Float);
        //Mass
        ImGui::DragFloat("Mass", &mass, 0.005f, 0.0f, FLT_MAX, "%.2f", 0);
        //Soft body internal pressure
        ImGui::DragFloat("Internal pressure", &internalPressure, 0.005f, 0.0f, FLT_MAX, "%.2f", 0);
        
        //When clicked spawn new body
        generate = ImGui::Button("Generate");

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        //OK
        //Generate a soft body using all parameters passed to the gui when generate is true
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
            cout << generatedSoftBodiesMeshes.size() << " new vec!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
        }


        //Switch generate to false otherwise bodies keep being generated!
        generate = false;

        /////////////////////////////////////////////////////////////////////
        //Frame rate evaluation
        // Calculate the elapsed time
        auto elapsedTime = std::chrono::high_resolution_clock::now() - startTime;
        startTime = std::chrono::high_resolution_clock::now();
        // Calculate the frame rate
        double frameRate = 1.0 / std::chrono::duration_cast<std::chrono::duration<double>>(elapsedTime).count();
        // Print the frame rate
        std::cout << "Frame rate: " << frameRate << std::endl;

        ///////////////////////////////////////////////////////////////////////
        //Depending on GUI selection do stuff

        //Soft bodies only (to speed up development)
        
        for (int i = 0; i < physics.world->getSoftBodyArray().size(); i++)
        {
            btSoftBody* softBodyToDraw = physics.world->getSoftBodyArray()[i];
            drawSoftBody(shaderProgram, CamV2, *softBodyToDraw, softBodiesColours[i]);
        }

        /*
        for (int i = 0; i < physics.world->getSoftBodyArray().size(); i++)
        {
            generatedSoftBodiesMeshes[i] = getSoftBodyMesh(physics.world->getSoftBodyArray()[i], softBodiesColours[i]);
            generatedSoftBodiesMeshes[i].Draw(shaderProgram, CamV2);
        }
        */

        ///////////////////////////////
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    shaderProgram.Delete();

    physics.deletePhysics();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;

}

//OK
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

//OK
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

//Identica a DrawSoftV2
void drawSoftBody(Shader& shader, CamV2& camera, btSoftBody& softBody, glm::vec3 softBodyColor)
{
    //Ricostruisco vertici e indici del soft body
    vector<Vertex> softVertices;
    //Mappa i vertici
    unsigned int index = 0;

    //Indici dei vertici ricavati correttamente usando la mappa
    vector<GLuint> softIndices;

    //Reserve size to save time and avoid reallocations
    softVertices.reserve(softBody.m_nodes.size());
    softIndices.reserve(softBody.m_faces.size() * 3);


    //Recupero tutte le componenti di ciascun vertice del soft body
    //PASSANDO DAI NODI, NON DALLE FACCE!!!!!!!!!
    //Fatto!
    //Adesso serve merge dei vertici!!!!!!!!!!!
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
        //Prepare indexing
        //Insert node and corresponding assigned index

    }

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

    //OFFICIAL VERSION
    //Li passo alla GPU per disegnare il softbody
    //MeshV2 mesh(softVertices, softIndices);
    //mesh.Draw(shader, camera);

    MeshV2 mesh2(softVertices, softIndices, 0);
    mesh2.drawV2(shader, camera);

}

MeshV2 getSoftBodyMesh(btSoftBody* softBody, glm::vec3 softBodyColor)
{
    //Ricostruisco vertici e indici del soft body
    vector<Vertex> softVertices;
    //Mappa i vertici
    unsigned int index = 0;

    //Indici dei vertici ricavati correttamente usando la mappa
    vector<GLuint> softIndices;

    //Reserve size to save time and avoid reallocations
    softVertices.reserve(softBody->m_nodes.size());
    softIndices.reserve(softBody->m_faces.size() * 3);


    //Recupero tutte le componenti di ciascun vertice del soft body
    //PASSANDO DAI NODI, NON DALLE FACCE!!!!!!!!!
    //Fatto!
    //Adesso serve merge dei vertici!!!!!!!!!!!
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

