typedef unsigned int CrestShader;

CrestShader CrestShaderInit(const char* VertexPath, const char* FragmentPath) {
    CrestShader Result = 0;
    //NOTE(Zen): load the shaders from their path
    char * VertexSource = CrestLoadFileAsString(VertexPath);
    char * FragmentSource = CrestLoadFileAsString(FragmentPath);

    unsigned int Vertex, Fragment;
    int Success;
    char InfoLog[512];

    //Note(Zen): Compile Vertex Shader
    Vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(Vertex, 1, &VertexSource, 0);
    glCompileShader(Vertex);

    glGetShaderiv(Vertex, GL_COMPILE_STATUS, &Success);
    if(!Success) {
        glGetShaderInfoLog(Vertex, 512, 0, InfoLog);

        OutputDebugStringA("\nFailed to compile vertex shader:");
        OutputDebugStringA(VertexPath);
        OutputDebugStringA("\n");
        OutputDebugStringA(InfoLog);
    }

    //Note(Zen): Compile Fragment Shader
    Fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(Fragment, 1, &FragmentSource, 0);
    glCompileShader(Fragment);

    glGetShaderiv(Fragment, GL_COMPILE_STATUS, &Success);
    if(!Success) {
        glGetShaderInfoLog(Fragment, 512, 0, InfoLog);

        OutputDebugStringA("\nFailed to compile Fragment shader: ");
        OutputDebugStringA(FragmentPath);
        OutputDebugStringA("\n");
        OutputDebugStringA(InfoLog);
    }

    //Note(Zen): Link Shader Program
    Result = glCreateProgram();
    glAttachShader(Result, Vertex);
    glAttachShader(Result, Fragment);
    glLinkProgram(Result);

    glGetProgramiv(Result, GL_LINK_STATUS, &Success);
    if(!Success) {
        glGetProgramInfoLog(Result, 512, 0, InfoLog);
        OutputDebugStringA("\nFailed to link shader Program: ");
        OutputDebugStringA(InfoLog);
    }

    //Note(Zen): delete shaders to reduce memory footprint
    glDeleteShader(Vertex);
    glDeleteShader(Fragment);
    free(VertexSource);
    free(FragmentSource);

    return Result;
}

internal void
CrestShaderSetFloat(CrestShader Shader, const char * UniformName, float Value) {
    glUseProgram(Shader);
    i32 Location = glGetUniformLocation(Shader, UniformName);
    glUniform1f(Location, Value);
}

internal void
CrestShaderSetMatrix(CrestShader Shader, const char * UniformName, matrix * Matrix) {
    glUseProgram(Shader);
    i32 Location = glGetUniformLocation(Shader, UniformName);
    glUniformMatrix4fv(Location, 1, GL_TRUE, Matrix->Elements);
}

internal void
CrestShaderSetV3(CrestShader Shader, const char * UniformName, v3 Vector) {
    glUseProgram(Shader);
    i32 Location = glGetUniformLocation(Shader, UniformName);
    glUniform3f(Location, Vector.x, Vector.y, Vector.z);
}
