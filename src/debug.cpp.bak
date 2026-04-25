#include "iostream"
#include "glad/glad.h"

#ifdef DEBUG

static void printGLIDs() {
    std::cout << "checking for GL IDs:\n";
    for(int i = 0; i < 50; i++) {
        
        if(glIsProgram(i)) {
            int status;
            glGetProgramiv(i, GL_DELETE_STATUS, &status);
            if(!status) std::cout << i << " is program! " << std::endl;
        }
        else if(glIsTexture(i))
            std::cout << i << " is texture! " << std::endl;
        else if(glIsShader(i)) {
            int status;
            glGetProgramiv(i, GL_DELETE_STATUS, &status);
            if(!status) std::cout << i << " is shader! " << std::endl;
        }
        else if(glIsVertexArray(i))
            std::cout << i << " is vertex array! " << std::endl;
        else if(glIsBuffer(i))
            std::cout << i << " is buffer! " << std::endl;
    }
}

#endif