//
//  MyScene.hpp
//  PolyVoxTest
//
//  Created by Daniel Parnell on 24/12/16.
//  Copyright © 2016 Automagic Software. All rights reserved.
//

#ifndef MyScene_hpp
#define MyScene_hpp

#include <OpenGL/gl3.h>

#include "PolyVox/CubicSurfaceExtractor.h"
#include "PolyVox/MarchingCubesSurfaceExtractor.h"
#include "PolyVox/Mesh.h"
#include "PolyVox/RawVolume.h"

void crossProduct( GLfloat *a, GLfloat *b, GLfloat *res);
void normalize(GLfloat *a);
void setIdentityMatrix( GLfloat *mat, int size);
void multMatrix(GLfloat *a, GLfloat *b);
void setTranslationMatrix(float *mat, float x, float y, float z);
void rotationMatrix(GLfloat *mat, GLfloat *axis, GLfloat angle);

// This structure holds all the data required
// to render one of our meshes through OpenGL.
struct OpenGLMeshData
{
    GLuint noOfIndices;
    GLenum indexType;
    GLuint indexBuffer;
    GLuint vertexBuffer;
    GLuint vertexArrayObject;
    GLfloat translation[3];
    GLfloat offset[3];
    GLfloat scale;
    GLfloat rotationAxis[3];
    GLfloat rotationAngle;
};

class MyScene {
public:
    // Convert a PolyVox mesh to OpenGL index/vertex buffers. Inlined because it's templatised.
    template <typename MeshType>
    void addMesh(const MeshType& surfaceMesh, const PolyVox::Vector3DInt32& translation = PolyVox::Vector3DInt32(0, 0, 0), float scale = 1.0f, const PolyVox::Vector3DInt32& offset = PolyVox::Vector3DInt32(0, 0, 0))
    {
        // This struct holds the OpenGL properties (buffer handles, etc) which will be used
        // to render our mesh. We copy the data from the PolyVox mesh into this structure.
        OpenGLMeshData meshData;
        
        // Create the VAO for the mesh
        glGenVertexArrays(1, &(meshData.vertexArrayObject));
        glBindVertexArray(meshData.vertexArrayObject);
        
        // The GL_ARRAY_BUFFER will contain the list of vertex positions
        glGenBuffers(1, &(meshData.vertexBuffer));
        glBindBuffer(GL_ARRAY_BUFFER, meshData.vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, surfaceMesh.getNoOfVertices() * sizeof(typename MeshType::VertexType), surfaceMesh.getRawVertexData(), GL_STATIC_DRAW);
        
        // and GL_ELEMENT_ARRAY_BUFFER will contain the indices
        glGenBuffers(1, &(meshData.indexBuffer));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshData.indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, surfaceMesh.getNoOfIndices() * sizeof(typename MeshType::IndexType), surfaceMesh.getRawIndexData(), GL_STATIC_DRAW);
        
        // Every surface extractor outputs valid positions for the vertices, so tell OpenGL how these are laid out
        glEnableVertexAttribArray(0); // Attrib '0' is the vertex positions
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(typename MeshType::VertexType), (GLvoid*)(offsetof(typename MeshType::VertexType, position))); //take the first 3 floats from every sizeof(decltype(vecVertices)::value_type)
        
        // Some surface extractors also generate normals, so tell OpenGL how these are laid out. If a surface extractor
        // does not generate normals then nonsense values are written into the buffer here and sghould be ignored by the
        // shader. This is mostly just to simplify this example code - in a real application you will know whether your
        // chosen surface extractor generates normals and can skip uploading them if not.
        glEnableVertexAttribArray(1); // Attrib '1' is the vertex normals.
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(typename MeshType::VertexType), (GLvoid*)(offsetof(typename MeshType::VertexType, normal)));
        
        // Finally a surface extractor will probably output additional data. This is highly application dependant. For this example code
        // we're just uploading it as a set of bytes which we can read individually, but real code will want to do something specialised here.
        glEnableVertexAttribArray(2); //We're talking about shader attribute '2'
        GLint size = (GLint)(std::min)(sizeof(typename MeshType::VertexType::DataType), size_t(4)); // Can't upload more that 4 components (vec4 is GLSL's biggest type)
        glVertexAttribIPointer(2, size, GL_UNSIGNED_INT, sizeof(typename MeshType::VertexType), (GLvoid*)(offsetof(typename MeshType::VertexType, data)));
        
        // We're done uploading and can now unbind.
        glBindVertexArray(0);
        
        // A few additional properties can be copied across for use during rendering.
        meshData.noOfIndices = (GLuint)surfaceMesh.getNoOfIndices();
        meshData.translation[0] = translation.getX();
        meshData.translation[1] = translation.getY();
        meshData.translation[2] = translation.getZ();
        meshData.offset[0] = offset.getX();
        meshData.offset[1] = offset.getY();
        meshData.offset[2] = offset.getZ();
        meshData.scale = scale;
        meshData.rotationAxis[0] = 0;
        meshData.rotationAxis[1] = 1;
        meshData.rotationAxis[2] = 0;
        meshData.rotationAngle = 0;
        
        // Set 16 or 32-bit index buffer size.
        meshData.indexType = sizeof(typename MeshType::IndexType) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
        
        // Now add the mesh to the list of meshes to render.
        addMeshData(meshData);
    }
    
    void addMeshData(OpenGLMeshData meshData)
    {
        mMeshData.push_back(meshData);
    }
    
    
    void render() {
        GLfloat mat[4 * 4];
        GLfloat rot[4 * 4];
        GLfloat off[4 * 4];
        
        glUseProgram(program);
        
        glUniformMatrix4fv(projMatrixLoc,  1, false, projMatrix);
        glUniformMatrix4fv(viewMatrixLoc,  1, false, viewMatrix);
        
        // Iterate over each mesh which the user added to our list, and render it.
        for (OpenGLMeshData meshData : mMeshData)
        {
            bzero(mat, sizeof(mat));
        
            setTranslationMatrix(off, -meshData.offset[0], -meshData.offset[1], -meshData.offset[2]);
            //Set up the model matrrix based on provided translation and scale.
            mat[0] = meshData.scale;
            mat[5] = meshData.scale;
            mat[10] = meshData.scale;
            mat[15] = meshData.scale;
            mat[12] = meshData.translation[0] * meshData.scale;
            mat[13] = meshData.translation[1] * meshData.scale;
            mat[14] = meshData.translation[2] * meshData.scale;

            rotationMatrix(rot, meshData.rotationAxis, meshData.rotationAngle);
            
            multMatrix(rot, off);
            multMatrix(mat, rot);
            glUniformMatrix4fv(modelMatrixLoc, 1, false, mat);
            
            // Bind the vertex array for the current mesh
            glBindVertexArray(meshData.vertexArrayObject);
            // Draw the mesh
            glDrawElements(GL_TRIANGLES, meshData.noOfIndices, meshData.indexType, 0);
            
            // Unbind the vertex array.
            glBindVertexArray(0);
        }
        
        glUseProgram(0);
    }
    
    // ----------------------------------------------------
    // Projection Matrix
    //
     
    void buildProjectionMatrix(float fov, float ratio, float nearP, float farP) {
         
            float f = 1.0f / tan (fov * (M_PI / 360.0));
         
            setIdentityMatrix(projMatrix,4);
         
            projMatrix[0] = f / ratio;
            projMatrix[1 * 4 + 1] = f;
            projMatrix[2 * 4 + 2] = (farP + nearP) / (nearP - farP);
            projMatrix[3 * 4 + 2] = (2.0f * farP * nearP) / (nearP - farP);
            projMatrix[2 * 4 + 3] = -1.0f;
            projMatrix[3 * 4 + 3] = 0.0f;
    }
    
    // ----------------------------------------------------
    // View Matrix
    //
    // note: it assumes the camera is not tilted,
    // i.e. a vertical up vector (remmeber gluLookAt?)
    //
     
    void setCamera(float posX, float posY, float posZ, float lookAtX, float lookAtY, float lookAtZ) {         
            float dir[3], right[3], up[3];
         
            up[0] = 0.0f;   up[1] = 1.0f;   up[2] = 0.0f;
         
            dir[0] =  (lookAtX - posX);
            dir[1] =  (lookAtY - posY);
            dir[2] =  (lookAtZ - posZ);
            normalize(dir);
         
            crossProduct(dir,up,right);
            normalize(right);
         
            crossProduct(right,dir,up);
            normalize(up);
         
            float aux[4*4];
         
            viewMatrix[0]  = right[0];
            viewMatrix[4]  = right[1];
            viewMatrix[8]  = right[2];
            viewMatrix[12] = 0.0f;
         
            viewMatrix[1]  = up[0];
            viewMatrix[5]  = up[1];
            viewMatrix[9]  = up[2];
            viewMatrix[13] = 0.0f;
         
            viewMatrix[2]  = -dir[0];
            viewMatrix[6]  = -dir[1];
            viewMatrix[10] = -dir[2];
            viewMatrix[14] =  0.0f;
         
            viewMatrix[3]  = 0.0f;
            viewMatrix[7]  = 0.0f;
            viewMatrix[11] = 0.0f;
            viewMatrix[15] = 1.0f;
         
            setTranslationMatrix(aux, -posX, -posY, -posZ);
         
            multMatrix(viewMatrix, aux);
    }
    
    void printShaderInfoLog(GLuint obj)
    {
        int infologLength = 0;
        int charsWritten  = 0;
        char *infoLog;
     
        glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
     
        if (infologLength > 0)
            {
                    infoLog = (char *)malloc(infologLength);
                    glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
                    printf("%s\n",infoLog);
                    free(infoLog);
            }
    }
     
    void printProgramInfoLog(GLuint obj)
    {
        int infologLength = 0;
        int charsWritten  = 0;
        char *infoLog;
     
        glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
     
        if (infologLength > 0)
            {
                    infoLog = (char *)malloc(infologLength);
                    glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
                    printf("%s\n",infoLog);
                    free(infoLog);
            }
    }
    
    void setupShaders(const char *vertexShaderSrc, const char *fragmentShaderSrc) {
         
        GLuint p,v,f;
     
        v = glCreateShader(GL_VERTEX_SHADER);
        f = glCreateShader(GL_FRAGMENT_SHADER);
    
        glShaderSource(v, 1, &vertexShaderSrc,NULL);
        glShaderSource(f, 1, &fragmentShaderSrc,NULL);
    
        glCompileShader(v);
        glCompileShader(f);
     
        printShaderInfoLog(v);
        printShaderInfoLog(f);
     
        p = glCreateProgram();
        glAttachShader(p,v);
        glAttachShader(p,f);
     
        glBindFragDataLocation(p, 0, "outputColor");
        
        glBindAttribLocation(p, 0, "position");
        //glBindAttribLocation(p, 1, "normal");
        //glBindAttribLocation(p, 2, "material");
        
        glLinkProgram(p);
        printProgramInfoLog(p);
     
        vertexLoc = glGetAttribLocation(p,"position");
     
        projMatrixLoc = glGetUniformLocation(p, "projectionMatrix");
        viewMatrixLoc = glGetUniformLocation(p, "viewMatrix");
        modelMatrixLoc = glGetUniformLocation(p, "modelMatrix");
     
        
        program = p;
    }
    
    GLuint program;
    
    // Vertex Attribute Locations
    GLuint vertexLoc;
    
    // Uniform variable Locations
    GLuint projMatrixLoc, viewMatrixLoc, modelMatrixLoc;
    
    
    // Index/vertex buffer data
    std::vector<OpenGLMeshData> mMeshData;
    
    // storage for Matrices
    GLfloat projMatrix[4*4];
    GLfloat viewMatrix[4*4];
    
};

#endif /* MyScene_hpp */
