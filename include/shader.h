#ifndef SHADER_H
#define SHADER_H

class Shader {

    unsigned int ID;
    
    public:
        Shader();
        Shader(const char* vertexSrc, const char* fragmentSrc);

        void use();

        void setBool(const char* name, bool value);
        void setInt(const char* name, int value);
        void setFloat(const char* name, float value);
};

#endif