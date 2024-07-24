#include "Program.h"

GLuint compileShader(const GLuint program, const GLenum shader_type, const std::string &source);
bool compileProgram(const GLuint program);

std::string read(const std::string &filename)
{
    // Constructor to create streamer + open
    std::ifstream input(filename);
    if (!input.is_open())
    {
        std::cerr << "Failed to open the file: " << filename << std::endl;
        return "Fail";
    }

    // Reading from the file
    std::stringstream buffer;
    buffer << input.rdbuf();
    input.close();
    return buffer.str();
}

void Program::init(const char *vertex_path, const char *fragment_path)
{

    std::string v_shader_code = read(vertex_path);
    std::string f_shader_code = read(fragment_path);
    shader_id = glCreateProgram();

    GLuint vertex = compileShader(shader_id, GL_VERTEX_SHADER, v_shader_code);
    GLuint fragment = compileShader(shader_id, GL_FRAGMENT_SHADER, f_shader_code);
    if (compileProgram(shader_id))
    {
        this->use();
    }
    else
    {
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        glDeleteProgram(shader_id);
    }
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    GLenum error = glGetError();

    if (error != GL_NO_ERROR)
    {
        std::cerr << "Error duringsShader initialization: " << error << std::endl;
    }
}

// TODO load should have speicifc or jsut delete it
void Program::load(const char *vertex_path, const char *fragment_path)
{
    std::string v_shader_code = read(vertex_path);
    std::string f_shader_code = read(fragment_path);
    shader_id = glCreateProgram();

    GLuint vertex = compileShader(shader_id, GL_VERTEX_SHADER, v_shader_code);
    GLuint fragment = compileShader(shader_id, GL_FRAGMENT_SHADER, f_shader_code);
    if (compileProgram(shader_id))
    {
        this->use();
    }
    else
    {
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        glDeleteProgram(shader_id);
    }
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

// use/activate the shader
void Program::use()
{
    if (shader_id == 0)
    {
        std::cerr << "Progam not found" << std::endl;
    }
    else
    {
        glUseProgram(shader_id);
    }
}

void Program::reset()
{
    this->detachAllShaders(shader_id);
    glUseProgram(shader_id);
    GLenum error = glGetError();

    if (error != GL_NO_ERROR)
    {
        std::cerr << "Error durings shaders dettach: " << error << std::endl;
    }
}

void Program::detachAllShaders(GLuint program)
{
    if (glIsProgram(program) == GL_FALSE)
    {
        std::cerr << "Invalid program ID provided in detachAllShaders or Unintialized Program." << std::endl;
        return;
    }

    // Get the number of attached shaders
    GLint count = 0;
    glGetProgramiv(program, GL_ATTACHED_SHADERS, &count);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error after glGetProgramiv in detachAllShaders. Id: " << error << std::endl;
        return;
    }

    // Fill the
    GLuint *shadersarr = new GLuint[count];
    if (shadersarr == nullptr)
    {
        std::cerr << "Failed to allocate memory for shader array in deatch AllShaders" << std::endl;
        return;
    }
    glGetAttachedShaders(program, count, NULL, shadersarr);

    // Detach each shader
    for (GLint i = 0; i < count; ++i)
    {
        glDetachShader(program, shadersarr[i]);
    }

    error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "An OpenGL error occured before detachAllShaders last call. Id: " << error << std::endl;
    }
    delete[] shadersarr;
}

void Program::addUniform1(const char *uniform, const unsigned int value)
{
    GLint location = glGetUniformLocation(shader_id, uniform);

    if (location == -1)
    {
        std::cerr << "Uniform " << uniform << " not found in shader." << std::endl;
    }
    glUniform1ui(glGetUniformLocation(shader_id, uniform), value);
}

void Program::addUniform1(const char *uniform, const int &value)
{
    GLint location = glGetUniformLocation(shader_id, uniform);

    if (location == -1)
    {
        std::cerr << "Uniform " << uniform << " not found in shader." << std::endl;
    }
    glUniform1i(glGetUniformLocation(shader_id, uniform), value);
}

void Program::addUniform1(const char *uniform, const float &value)
{
    glUniform1f(glGetUniformLocation(shader_id, uniform), value);
}

void Program::addUniform2(const char *uniform, const Vec<GLfloat> &value)
{
    glUniform2fv(glGetUniformLocation(shader_id, uniform), 1, &value.x);
}

void Program::addUniform2(const char *uniform, const Vec<GLuint> &value)
{
    glUniform2uiv(glGetUniformLocation(shader_id, uniform), 1, &value.x);
}

void Program::addUniform3(const char *uniform, const Vec<GLfloat> &value)
{
    GLenum error = glGetError();

    GLint location = glGetUniformLocation(shader_id, uniform);

    if (location == -1)
    {
        std::cerr << "Uniform " << uniform << " not found in shader." << std::endl;
    }
    glUniform3fv(glGetUniformLocation(shader_id, uniform), 1, &value.x);
}

void Program::addUniform3(const char *uniform, const Vec<GLuint> &value)
{
    GLint location = glGetUniformLocation(shader_id, uniform);

    if (location == -1)
    {
        std::cerr << "Uniform " << uniform << " not found in shader." << std::endl;
    }
    glUniform3uiv(glGetUniformLocation(shader_id, uniform), 1, &value.x);
}

/*
GLuint readProgram( const std::string& filename, const std::string& definitions )
{
    GLuint program= glCreateProgram();
    std::string common_source= read(filename);
    bool status= compileProgram(program);
    return program;
}
*/
// GL_VERTEX_SHADER
// GL_FRAGMENT_SHADER
// TODO SEg fault here
GLuint compileShader(const GLuint program, const GLenum shader_type, const std::string &shader_string)
{
    if (shader_string.empty())
    {
        std::cerr << "Error: Shader shader_string code is empty." << std::endl;
        return 0;
    }
    GLuint shader = glCreateShader(shader_type);

    if (shader == 0)
    {
        std::cerr << "Error: Failed to create shader." << std::endl;
        return 0;
    }

    const char *shader_code = shader_string.c_str();
    if (shader_code == nullptr)
    {
        std::cerr << "Shader source string conversion failed." << std::endl;
        return 0;
    }

    glShaderSource(shader, 1, &shader_code, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[1024];

        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cerr << "Compilation of " << (shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader failed." << std::endl
                  << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    else
    {
        glAttachShader(program, shader);
        GLint attachStatus;
        glGetProgramiv(program, GL_ATTACHED_SHADERS, &attachStatus);
        if (attachStatus == 0)
        {
            std::cerr << "Failed to attach shader to program." << std::endl;
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }
}

bool compileProgram(const GLuint program)
{

    // linke les shaders
    glLinkProgram(program);

    // verifie les erreurs
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[1024];

        glGetProgramInfoLog(program, 1024, NULL, infoLog);
        std::cerr << "Error at Program linking" << std::endl;
        return false;
    }
    return true;
}
