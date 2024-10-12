// Example of some of the code usage
/*
#insdsddsdsclude "Vec.h"
#isdssdnclude "Mat.h"
#includsddde "glcosdazasre.h"
#indsdsdsclude "Buffers.h"
#indssddssdclude "Texture.h"
#insddssdsclude "Pazazrogram.h"
#incazazlude <GLFaazazW/glfw3.h>


unsigned count = 0;

bool init()
{
    prog.init("shader.vs", "shader.fs");
    std::vector<Vec<float>> canvas = {
        Vec(-1.0f, -1.0f, 0.0f),
        Vec(-1.0f, 1.0f, 0.0f),
        Vec(1.0f, 1.0f, 0.0f),
        Vec(1.0f, -1.0f, 0.0f)};

    std::vector<Vec<float>> canvastext = {
        Vec(0.0f, 0.0f, 0.0f),
        Vec(0.0f, 1.0f, 0.0f),
        Vec(1.0f, 1.0f, 0.0f),
        Vec(1.0f, 0.0f, 0.0f)

    };

    std::vector<unsigned> indices = {0, 1, 2,
                                     2, 3, 0};

    model1 = createBuffers(canvas, indices, canvastext);
    count = indices.size();
    initTextRess(imageress, read_texture<textuc>(1, datapath, matimg));
    return true;
}

void quit()
{
    freeBuffers(model1);
    release(imageress);
    freeTexture(imageress);
}

void draw()
{
   
}
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

/*


 

/*

int main(int argc, char **argv)
{
    int width = 1024;
    int height = 576;
    // Context initialization
    // GLFW Init
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(width, height, "TestOpengl", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // GLEW Init
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glViewport(0, 0, width, height);
    // Set the callback function for window resize
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height)
                                   { glViewport(0, 0, width, height); });
    //(VSync)
    glfwSwapInterval(1);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearDepth(1);
    // Context initialization

    if (!init())
    {
        return 1;
    }

    bool close = false;
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        // dessiner
        draw();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    quit();
    return 0;
}
*/