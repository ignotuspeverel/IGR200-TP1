#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// constants
const static float kSizeSun = 1;
const static float kSizeEarth = 0.5;
const static float kSizeMoon = 0.25;
const static float kRadOrbitEarth = 10;
const static float kRadOrbitMoon = 2;

// Window parameters
GLFWwindow *g_window = nullptr;

// GPU objects
GLuint g_program = 0; // A GPU program contains at least a vertex shader and a fragment shader

// OpenGL identifiers
GLuint g_vao = 0;
GLuint g_posVbo = 0;
GLuint g_colVbo = 0;
GLuint g_ibo = 0;

// All vertex positions packed in one array [x0, y0, z0, x1, y1, z1, ...]
std::vector<float> g_vertexPositions;
// All triangle indices packed in one array [v00, v01, v02, v10, v11, v12, ...] with vij the index of j-th vertex of the i-th triangle
std::vector<unsigned int> g_triangleIndices;
// All vertex colors packed in one array [r0, g0, b0, r1, g1, b1, ...]
std::vector<float> g_vertexColors;

// Basic camera model
class Camera
{
public:
    inline float getFov() const { return m_fov; }
    inline void setFoV(const float f) { m_fov = f; }
    inline float getAspectRatio() const { return m_aspectRatio; }
    inline void setAspectRatio(const float a) { m_aspectRatio = a; }
    inline float getNear() const { return m_near; }
    inline void setNear(const float n) { m_near = n; }
    inline float getFar() const { return m_far; }
    inline void setFar(const float n) { m_far = n; }
    inline void setPosition(const glm::vec3 &p) { m_pos = p; }
    inline glm::vec3 getPosition() { return m_pos; }

    inline glm::mat4 computeViewMatrix() const
    {
        return glm::lookAt(m_pos, glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
    }

    // Returns the projection matrix stemming from the camera intrinsic parameter.
    inline glm::mat4 computeProjectionMatrix() const
    {
        return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_near, m_far);
    }

private:
    glm::vec3 m_pos = glm::vec3(0, 0, 0);
    float m_fov = 45.f;        // Field of view, in degrees
    float m_aspectRatio = 1.f; // Ratio between the width and the height of the image
    float m_near = 0.1f;       // Distance before which geometry is excluded from the rasterization process
    float m_far = 10.f;        // Distance after which the geometry is excluded from the rasterization process
};
Camera g_camera;

class Mesh {
public:
    // load gpu geometry for the mesh, with this step we initialize the final mesh
    void init() {
        // vao of the mesh
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        //vbo of the mesh
        size_t vertexBufferSize = sizeof(float) * m_vertexPositions.size();
        size_t normalBufferSize = sizeof(float) * m_vertexNormals.size();
        glGenBuffers(1, &m_posVbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_posVbo);
        glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, m_vertexPositions.data(), GL_DYNAMIC_READ);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &m_normalVbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_normalVbo);
        glBufferData(GL_ARRAY_BUFFER, normalBufferSize, m_vertexNormals.data(), GL_DYNAMIC_READ);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(1);

        //ibo of the mesh
        size_t indexBufferSize = sizeof(unsigned int) * m_triangleIndices.size();
        glGenBuffers(1, &m_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, m_triangleIndices.data(), GL_DYNAMIC_READ);

        glBindVertexArray(0); // deactivate the VAO for now, will be activated again when rendering
    }; // should properly set up the geometry buffer
    // render the mesh
    void render() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const glm::mat4 viewMatrix = g_camera.computeViewMatrix();
        const glm::mat4 projMatrix = g_camera.computeProjectionMatrix();
        const glm::vec3 camPosition = g_camera.getPosition();

        glUniformMatrix4fv(glGetUniformLocation(g_program, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMatrix)); 
        glUniformMatrix4fv(glGetUniformLocation(g_program, "projMat"), 1, GL_FALSE, glm::value_ptr(projMatrix));
        glUniform3f(glGetUniformLocation(g_program, "camPos"), camPosition[0], camPosition[1], camPosition[2]);

        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, m_triangleIndices.size(), GL_UNSIGNED_INT, 0);
    }; // should be called in the main rendering loop
    // generate a unit sphere, create the buffer
    static std::shared_ptr<Mesh> genSphere(const size_t resolution=16) {
        std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
        const float pi = glm::pi<float>();
        const float pi2 = pi * 2.f;
        const float step_phi = pi / resolution;
        const float step_theta = pi2 / resolution;
        for (size_t i = 0; i <= resolution; i++) {
            float phi = step_phi * float(i);
            for (size_t j = 0; j <= resolution; j++) {
                float theta = step_theta * float(j);
                float x = std::sin(phi) * std::cos(theta);
                float y = std::sin(phi) * std::sin(theta);
                float z = std::cos(phi);
                mesh->m_vertexPositions.push_back(x);
                mesh->m_vertexPositions.push_back(y);
                mesh->m_vertexPositions.push_back(z);
                mesh->m_vertexNormals.push_back(x);
                mesh->m_vertexNormals.push_back(y);
                mesh->m_vertexNormals.push_back(z);
            }
        }
        for (size_t i = 0; i < resolution; i++) {
            for (size_t j = 0; j < resolution; j++) {
                mesh->m_triangleIndices.push_back(i * (resolution+1) + j + 1);
                mesh->m_triangleIndices.push_back(i * (resolution+1) + j);
                mesh->m_triangleIndices.push_back((i + 1) * (resolution+1) + (j + 1));
                mesh->m_triangleIndices.push_back((i + 1) * (resolution+1) + (j + 1));
                mesh->m_triangleIndices.push_back(i * (resolution+1) + j);
                mesh->m_triangleIndices.push_back((i + 1) * (resolution+1) + j);
            }
        }
        std::cout << "sphere generated" << std::endl;
        return mesh;
    }; 

private:
    std::vector<float> m_vertexPositions;
    std::vector<float> m_vertexNormals;
    std::vector<unsigned int> m_triangleIndices;
    GLuint m_vao = 0;
    GLuint m_posVbo = 0;
    GLuint m_normalVbo = 0;
    GLuint m_ibo = 0;
};


GLuint loadTextureFromFileToGPU(const std::string &filename)
{
    int width, height, numComponents;
    // Loading the image in CPU memory using stb_image
    unsigned char *data = stbi_load(
        filename.c_str(),
        &width, &height,
        &numComponents, // 1 for a 8 bit grey-scale image, 3 for 24bits RGB image, 4 for 32bits RGBA image
        0);

    GLuint texID;
    // TODO: create a texture and upload the image data in GPU memory using
    // glGenTextures, glBindTexture, glTexParameteri, and glTexImage2D

    // Free useless CPU memory
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0); // unbind the texture

    return texID;
}

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback(GLFWwindow *window, int width, int height)
{
    g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    glViewport(0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
}

// Executed each time a key is entered.
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS && key == GLFW_KEY_W)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);            // Draw mesh edges as lines. Uncomment this line to draw only lines.
    }
    else if (action == GLFW_PRESS && key == GLFW_KEY_F)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);            // Draw mesh faces. Uncomment this line to draw only faces.
    }
    else if (action == GLFW_PRESS && (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q))
    {
        glfwSetWindowShouldClose(window, true);               // Closes the application if the escape key is pressed
    }
}

// Executed each time when an error occurs.
void errorCallback(int error, const char *desc)
{
    std::cout << "Error " << error << ": " << desc << std::endl;
}

// Initialize GLFW, GLEW, OpenGL settings, callbacks etc.
void initGLFW()
{
    glfwSetErrorCallback(errorCallback);

    // Initialize GLFW, the library responsible for window management
    if (!glfwInit())
    {
        std::cerr << "ERROR: Failed to init GLFW" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // Before creating the window, set some option flags
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    // Create the window
    g_window = glfwCreateWindow(
        1024, 768,
        "Interactive 3D Applications (OpenGL) - Simple Solar System",
        nullptr, nullptr);
    if (!g_window)
    {
        std::cerr << "ERROR: Failed to open window" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    // Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
    glfwMakeContextCurrent(g_window);
    glfwSetWindowSizeCallback(g_window, windowSizeCallback);
    glfwSetKeyCallback(g_window, keyCallback);
}

// Initialize OpenGL
void initOpenGL()
{
    // Load extensions for modern OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "ERROR: Failed to initialize OpenGL context" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    glCullFace(GL_BACK);                  // Specifies the faces to cull (here the ones pointing away from the camera)
    glEnable(GL_CULL_FACE);               // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
    glDepthFunc(GL_LESS);                 // Specify the depth test for the z-buffer, if the stored value is greater than the one from the fragment then discard.
    glEnable(GL_DEPTH_TEST);              // Enable the z-buffer test in the rasterization
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f); // specify the background color, used any time the framebuffer is cleared
}

// Loads the content of an ASCII file in a standard C++ string
std::string file2String(const std::string &filename)
{
    std::ifstream t(filename.c_str());   // open the ASCII file for reading, .c_str() transforms a std::string into a C-style char*
    if (!t.is_open()) {
        std::cerr << "ERROR: file " << filename << " not found" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::stringstream buffer;            // temporary buffer to read the file 
    buffer << t.rdbuf();                 // stream the file into the buffer
    if (t.fail()) {
        std::cerr << "ERROR: Could not read file " << filename << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return buffer.str();                 // return the content of the buffer, which is a std::string type
}

// Loads and compile a shader, before attaching it to a program
void loadShader(GLuint program, GLenum type, const std::string &shaderFilename)
{
    GLuint shader = glCreateShader(type);                                    // Create the shader, e.g., a vertex shader to be applied to every single vertex of a mesh
    std::string shaderSourceString = file2String(shaderFilename);            // Loads the shader source from a file to a C++ string
    const GLchar *shaderSource = (const GLchar *)shaderSourceString.c_str(); // Interface the C++ string through a C pointer
    glShaderSource(shader, 1, &shaderSource, NULL);                          // load the vertex shader code
    glCompileShader(shader);
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR in compiling " << shaderFilename << "\n\t" << infoLog << std::endl;
    }
    glAttachShader(program, shader);
    glDeleteShader(shader);
}

void initGPUprogram()
{
    g_program = glCreateProgram(); // Create a GPU program, i.e., two central shaders of the graphics pipeline
    loadShader(g_program, GL_VERTEX_SHADER, "../vertexShader.glsl");
    loadShader(g_program, GL_FRAGMENT_SHADER, "../fragmentShader.glsl");
    glLinkProgram(g_program); // The main GPU program is ready to be handle streams of polygons

    glUseProgram(g_program);
    // TODO: set shader variables, textures, etc.
}

// Define your mesh(es) in the CPU memory
void initCPUgeometry()
{
    // TODO: add vertices and indices for your mesh(es)
    g_vertexPositions = { // the array of vertex positions [x0, y0, z0, x1, y1, z1, ...]
        0.f, 0.f, 0.f,
        1.f, 0.f, 0.f,
        0.f, 1.f, 0.f};
    g_triangleIndices = { 0, 1, 2 }; // indices just for one triangle
    g_vertexColors = { // the array of vertex colors [r0, g0, b0, r1, g1, b1, ...]
        1.f, 0.f, 0.f,
        0.f, 1.f, 0.f,
        0.f, 0.f, 1.f};
    // g_vertexPositions = {};
    // g_triangleIndices = {};
}

void initGPUgeometry()
{
    // Create a single handle, vertex array object that contains attributes,
    // vertex buffer objects (e.g., vertex's position, normal, and color)
#ifdef _MY_OPENGL_IS_33_
    glGenVertexArrays(1, &g_vao); // If your system doesn't support OpenGL 4.5, you should use this instead of glCreateVertexArrays.
#else
    // glCreateVertexArrays(1, &g_vao);
#endif
    glBindVertexArray(g_vao);

    // Generate a GPU buffer to store the positions of the vertices
    size_t vertexBufferSize = sizeof(float) * g_vertexPositions.size(); // Gather the size of the buffer from the CPU-side vector
    size_t colorBufferSize = sizeof(float) * g_vertexColors.size();
#ifdef _MY_OPENGL_IS_33_
    glGenBuffers(1, &g_posVbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_posVbo);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, g_vertexPositions.data(), GL_DYNAMIC_READ);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &g_colVbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_colVbo);
    glBufferData(GL_ARRAY_BUFFER, colorBufferSize, g_vertexColors.data(), GL_DYNAMIC_READ);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(GLfloat), 0);
    glEnableVertexAttribArray(1);

#else
    // glCreateBuffers(1, &g_posVbo);
    // glBindBuffer(GL_ARRAY_BUFFER, g_posVbo);
    // glNamedBufferStorage(g_posVbo, vertexBufferSize, g_vertexPositions.data(), GL_DYNAMIC_STORAGE_BIT); // Create a data storage on the GPU and fill it from a CPU array
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    // glEnableVertexAttribArray(0);
#endif

    // Same for an index buffer object that stores the list of indices of the
    // triangles forming the mesh
    size_t indexBufferSize = sizeof(unsigned int) * g_triangleIndices.size();
    // std::cout << "indexBufferSize: " << g_triangleIndices.size() << std::endl;
#ifdef _MY_OPENGL_IS_33_
    glGenBuffers(1, &g_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, g_triangleIndices.data(), GL_DYNAMIC_READ);
#else
    // glCreateBuffers(1, &g_ibo);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibo);
    // glNamedBufferStorage(g_ibo, indexBufferSize, g_triangleIndices.data(), GL_DYNAMIC_STORAGE_BIT);
#endif

    glBindVertexArray(0); // deactivate the VAO for now, will be activated again when rendering
}

void initCamera()
{
    int width, height;
    glfwGetWindowSize(g_window, &width, &height);
    g_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));

    g_camera.setPosition(glm::vec3(3.0, 0.0, 0.0));
    g_camera.setNear(0.1);
    g_camera.setFar(40.1);
}

void init()
{
    initGLFW();
    initOpenGL();
    initCPUgeometry();
    initGPUprogram();
    initGPUgeometry();
    initCamera();
}

std::shared_ptr<Mesh> initSphere()
{
    initGLFW();
    initOpenGL();
    std::shared_ptr<Mesh> sphere = Mesh::genSphere(32);
    initGPUprogram();
    sphere->init();
    initCamera();
    return sphere;
}

void clear()
{
    glDeleteProgram(g_program);
    glfwDestroyWindow(g_window);
    glfwTerminate();
}

// The main rendering call
void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.

    const glm::mat4 viewMatrix = g_camera.computeViewMatrix();
    const glm::mat4 projMatrix = g_camera.computeProjectionMatrix();

    glUniformMatrix4fv(glGetUniformLocation(g_program, "viewMat"), 1, GL_FALSE, glm::value_ptr(viewMatrix)); // compute the view matrix of the camera and pass it to the GPU program
    glUniformMatrix4fv(glGetUniformLocation(g_program, "projMat"), 1, GL_FALSE, glm::value_ptr(projMatrix)); // compute the projection matrix of the camera and pass it to the GPU program

    glBindVertexArray(g_vao);                                                   // activate the VAO storing geometry data
    glDrawElements(GL_TRIANGLES, g_triangleIndices.size(), GL_UNSIGNED_INT, 0); // Call for rendering: stream the current GPU geometry through the current GPU program
}

// Update any accessible variable based on the current time
void update(const float currentTimeInSec)
{
    // std::cout << currentTimeInSec << std::endl;
}

int main(int argc, char **argv)
{   
    // mesh init
    std::shared_ptr<Mesh> sphere = initSphere();
    //init(); // Your initialization code (user interface, OpenGL states, scene with geometry, material, lights, etc)
    while (!glfwWindowShouldClose(g_window))
    {
        //update(static_cast<float>(glfwGetTime()));
        //render();
        sphere->render();
        glfwSwapBuffers(g_window);
        glfwPollEvents();
    }
    clear();
    return EXIT_SUCCESS;
}
