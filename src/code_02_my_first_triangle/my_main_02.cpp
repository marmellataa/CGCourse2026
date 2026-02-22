#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "../common/debugging.h"

GLuint boxVAO;
GLuint boxVBO;
GLuint boxEBO;
int boxIndexCount;
GLuint positionAttribIndex = 0;
GLuint colorAttribIndex = 1;

void create_box2d(int sizex, int sizey) {
    std::vector<float> vertices;
    std::vector<GLuint> indices;

    float dx = 2.0f / sizex; // passo X
    float dy = 2.0f / sizey; // passo Y

    // generiamo vertici con posizione (x,y) e colore (qui colore semplice)
    for (int j = 0; j <= sizey; j++) {
        for (int i = 0; i <= sizex; i++) {
            float x = -1.0f + i * dx;
            float y = -1.0f + j * dy;

            // posizione
            vertices.push_back(x);
            vertices.push_back(y);

            // colore (qui puoi mettere qualsiasi cosa, tipo gradient)
            vertices.push_back((x + 1.0f) / 2.0f);
            vertices.push_back((y + 1.0f) / 2.0f);
            vertices.push_back(0.5f);
        }
    }

    // generiamo indici per triangoli
    for (int j = 0; j < sizey; j++) {
        for (int i = 0; i < sizex; i++) {
            int row1 = j * (sizex + 1);
            int row2 = (j + 1) * (sizex + 1);

            // primo triangolo
            indices.push_back(row1 + i);
            indices.push_back(row2 + i);
            indices.push_back(row2 + i + 1);

            // secondo triangolo
            indices.push_back(row1 + i);
            indices.push_back(row2 + i + 1);
            indices.push_back(row1 + i + 1);
        }
    }

    boxIndexCount = indices.size();

    // creiamo VAO
    glGenVertexArrays(1, &boxVAO);
    glBindVertexArray(boxVAO);

    // VBO
    glGenBuffers(1, &boxVBO);
    glBindBuffer(GL_ARRAY_BUFFER, boxVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // EBO
    glGenBuffers(1, &boxEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boxEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // attributi
    glEnableVertexAttribArray(0); // posizione
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1); // colore
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
}


int main(int argc, char** argv) {
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;


    // Request OpenGL 4.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // Ask specifically for the core profile (recommended)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // macOS requires this for 3.2+ contexts
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(512, 512, "my_code_02_my_first_triangle", NULL, NULL);

    if (!window){
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // Load GL symbols *after* the context is current
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::fprintf(stderr, "Failed to initialize GLAD\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    /* query for the hardware and software specs and print the result on the console*/
    printout_opengl_glsl_info();

    // shader
    
    ///* create a vertex shader */
    std::string  vertex_shader_src = "#version 410\n \
        in vec2 aPosition;\
        in vec3 aColor;\
        out vec3 vColor;\
        uniform float uDelta;\
        void main(void)\
        {\
         gl_Position = vec4(aPosition+vec2(uDelta,0.0), 0.0, 1.0);\
         vColor = aColor;\
        }\
       ";
    const GLchar* vs_source = (const GLchar*)vertex_shader_src.c_str();
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vs_source, NULL);
    glCompileShader(vertex_shader);


    ///* create a fragment shader */
    std::string   fragment_shader_src = "#version 410 \n \
        layout (location = 0) out vec4 color;\
        in vec3 vColor;\
        uniform float uDelta;\
        void main(void)\
        {\
            color = vec4(vColor+vec3(uDelta,0.0,0.0), 1.0);\
        }";
    const GLchar* fs_source = (const GLchar*)fragment_shader_src.c_str();

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fs_source, NULL);
    glCompileShader(fragment_shader);


    GLuint program_shader = glCreateProgram();
    glAttachShader(program_shader, vertex_shader);
    glAttachShader(program_shader, fragment_shader);

    glBindAttribLocation(program_shader, positionAttribIndex, "aPosition");
    glBindAttribLocation(program_shader, colorAttribIndex, "aColor");
    glLinkProgram(program_shader);


    GLint linked;
    validate_shader_program(program_shader);
    glGetProgramiv(program_shader, GL_LINK_STATUS, &linked);
    if (linked) {
        glUseProgram(program_shader);
    }

    GLint loc = glGetUniformLocation(program_shader, "uDelta");

	create_box2d(1, 10);

    // render loop
    glClearColor(0.2, 0.2, 0.2, 1);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program_shader);

        glBindVertexArray(boxVAO);
        glDrawElements(GL_TRIANGLES, boxIndexCount, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

}