
#include <stdio.h>
#include <stdbool.h>
#include "../libs/glewlib/include/GL/glew.h"
#include "../libs/glfw/include/GLFW/glfw3.h"
#include "file.h"
#include "../libs/piccolo/include.h"
#include "../libs/piccolo/stdlib/picStdlib.h"
#include "../libs/piccolo/debug/disassembler.h"
#include <math.h>

unsigned int compileShader(const char* path, GLenum type) {
    unsigned int shader;
    shader = glCreateShader(type);
    char* shaderSource = readFile(path);
    if(shaderSource == NULL) {
        fprintf(stderr, "%s SHADER COMPILATION FAILED\nFile %s does not exsist", (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT"), path);
        return -1;
    }
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "%s SHADER COMPILATION FAILED\n%s\n", (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT"), infoLog);
        return -1;
    }
    return shader;
}

#define NUM_SPHERES 32

int numSpheres = 0;
unsigned int program;
unsigned int spherePosLocations[NUM_SPHERES];
unsigned int sphereRadiusLocations[NUM_SPHERES];
unsigned int sphereColorLocations[NUM_SPHERES];
unsigned int sphereMetalLocations[NUM_SPHERES];
unsigned int sphereFuzzLocations[NUM_SPHERES];
unsigned int sphereEmisionLocations[NUM_SPHERES];
unsigned int sky1Loc, sky2Loc;

void pushSphere(float x, float y, float z, float radius, float r, float g, float b, float metal, float fuzz, float er, float eg, float eb) {
    glUniform3f(spherePosLocations[numSpheres], x, y, z);
    glUniform1f(sphereRadiusLocations[numSpheres], radius);
    glUniform3f(sphereColorLocations[numSpheres], r, g, b);
    glUniform1f(sphereMetalLocations[numSpheres], metal);
    glUniform1f(sphereFuzzLocations[numSpheres], fuzz);
    glUniform3f(sphereEmisionLocations[numSpheres], er, eg, eb);
    numSpheres++;
}

void setSky(float r1, float g1, float b1, float r2, float g2, float b2) {
    glUniform3f(sky1Loc, r1, g1, b1);
    glUniform3f(sky2Loc, r2, g2, b2);
}

void printError(const char* format, va_list args) {
    vfprintf(stderr, format, args);
}

piccolo_Value pushShphereNative(struct piccolo_Engine* engine, int argc, piccolo_Value* argv, piccolo_Value self) {
    if(argc != 12) {
        piccolo_runtimeError(engine, "Wrong argument count.");
        return PICCOLO_NIL_VAL();
    }
    float args[12];
    for(int i = 0; i < 12; i++) {
        if(!PICCOLO_IS_NUM(argv[i])) {
            piccolo_runtimeError(engine, "Argument must be number.");
            return PICCOLO_NIL_VAL();
        }
        args[i] = PICCOLO_AS_NUM(argv[i]);
    }
    pushSphere(
        args[0],
        args[1],
        args[2],
        args[3],
        args[4],
        args[5],
        args[6],
        args[7],
        args[8],
        args[9],
        args[10],
        args[11]
    );
    return PICCOLO_NIL_VAL();
}

void addLuxLib(struct piccolo_Engine* engine) {
    struct piccolo_Package* lux = piccolo_createPackage(engine);
    lux->packageName = "lux";
    piccolo_defineGlobal(engine, lux, "pushSphere", PICCOLO_OBJ_VAL(piccolo_makeNative(engine, pushShphereNative)));
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    GLFWwindow* window = glfwCreateWindow(300, 150, "Lux", NULL, NULL);
    if (window == NULL) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = true;
    if(glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    float vertices[] = {
        1.0f,  1.0f, 0.0f,  // top right
        1.0f, -1.0f, 0.0f,  // bottom right
        -1.0f, 1.0f, 0.0f,  // top left 
        // second triangle
        1.0f, -1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f,  // bottom left
        -1.0f,  1.0f, 0.0f
    };  

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);

    unsigned int VBO;
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);  

    unsigned int vertexShader = compileShader("src/raytrace.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader("src/raytrace.frag", GL_FRAGMENT_SHADER);

    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        printf("LINKING FAILED\n%s\n", infoLog);
    }

    glUseProgram(program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    for(int i = 0; i < NUM_SPHERES; i++) {
        char buffer[64];
        sprintf(buffer, "uSpherePos[%d]", i);
        spherePosLocations[i] = glGetUniformLocation(program, buffer);
        sprintf(buffer, "uSphereRadius[%d]", i);
        sphereRadiusLocations[i] = glGetUniformLocation(program, buffer);
        sprintf(buffer, "uSphereColor[%d]", i);
        sphereColorLocations[i] = glGetUniformLocation(program, buffer);
        sprintf(buffer, "uSphereMetalness[%d]", i);
        sphereMetalLocations[i] = glGetUniformLocation(program, buffer);
        sprintf(buffer, "uSphereFuzz[%d]", i);
        sphereFuzzLocations[i] = glGetUniformLocation(program, buffer);
        sprintf(buffer, "uSphereEmmision[%d]", i);
        sphereEmisionLocations[i] = glGetUniformLocation(program, buffer);
    }
    sky1Loc = glGetUniformLocation(program, "uSky1");
    sky2Loc = glGetUniformLocation(program, "uSky2");

    // Set up piccolo stuff
        
    struct piccolo_Engine engine;
    piccolo_initEngine(&engine, printError);
    piccolo_addIOLib(&engine);
    piccolo_addTimeLib(&engine);
    piccolo_addMathLib(&engine);
    piccolo_addRandomLib(&engine);
    addLuxLib(&engine);

    struct piccolo_Package* package = piccolo_loadPackage(&engine, "main.pic");
    if(package->compilationError) {
        piccolo_freeEngine(&engine);
        return -1;
    }
    if(!piccolo_executePackage(&engine, package)) {
        piccolo_enginePrintError(&engine, "Runtime error.\n");
        piccolo_freeEngine(&engine);
        return -1;
    }
    piccolo_Value loopFunc = piccolo_getGlobal(&engine, package, "loop");
    if(!PICCOLO_IS_CLOSURE(loopFunc)) {
        piccolo_enginePrintError(&engine, "No loop function found.\n");
        piccolo_freeEngine(&engine);
        return -1;
    }

    while(!glfwWindowShouldClose(window)) {
        numSpheres = 0;
            
        piccolo_Value args[0] = { PICCOLO_NIL_VAL() };
        piccolo_callFunction(&engine, PICCOLO_AS_OBJ(loopFunc), 1, args);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(program);

        glUniform1f(glGetUniformLocation(program, "uTimeSeed"), (float)glfwGetTime());
        int w, h;
        glfwGetWindowSize(window, &w, &h);
        glUniform1f(glGetUniformLocation(program, "uAspectRatio"), (float)w / (float)h);

        setSky(0.0f, 0.0f, 0.3f, 1.0f, 1.0f, 1.0f);

        glUniform1i(glGetUniformLocation(program, "uNumSpheres"), numSpheres);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
}