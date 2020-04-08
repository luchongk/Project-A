#ifndef SHADER_H
#define SHADER_H

class Shader {

    unsigned int ID;
    char name[64];
    
    public:
        Shader(const char* name, const char* vertexSrc, const char* fragmentSrc);
        ~Shader();

        const char* getName();
        unsigned int getId();
        Shader* getNext();
        void setNext(Shader* next);
        
        void use();

        void setBool(const char* name, bool value);
        void setInt(const char* name, int value);
        void setFloat(const char* name, float value);
};

#endif