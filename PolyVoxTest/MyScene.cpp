//
//  MyScene.cpp
//  PolyVoxTest
//
//  Created by Daniel Parnell on 24/12/16.
//  Copyright © 2016 Automagic Software. All rights reserved.
//

#include "MyScene.hpp"


// ----------------------------------------------------
// VECTOR STUFF
//

// res = a cross b;
void crossProduct( GLfloat *a, GLfloat *b, GLfloat *res) {
    
    res[0] = a[1] * b[2]  -  b[1] * a[2];
    res[1] = a[2] * b[0]  -  b[2] * a[0];
    res[2] = a[0] * b[1]  -  b[0] * a[1];
}

// Normalize a vec3
void normalize(GLfloat *a) {
    
    GLfloat mag = sqrt(a[0] * a[0]  +  a[1] * a[1]  +  a[2] * a[2]);
    
    a[0] /= mag;
    a[1] /= mag;
    a[2] /= mag;
}

// ----------------------------------------------------
// MATRIX STUFF
//

// sets the square matrix mat to the identity matrix,
// size refers to the number of rows (or columns)
void setIdentityMatrix( GLfloat *mat, int size) {
    
    // fill matrix with 0s
    for (int i = 0; i < size * size; ++i)
        mat[i] = 0.0f;
    
    // fill diagonal with 1s
    for (int i = 0; i < size; ++i)
        mat[i + i * size] = 1.0f;
}

//
// a = a * b;
//
void multMatrix(GLfloat *a, GLfloat *b) {
    
    GLfloat res[16];
    
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            res[j*4 + i] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                res[j*4 + i] += a[k*4 + i] * b[j*4 + k];
            }
        }
    }
    memcpy(a, res, 16 * sizeof(GLfloat));
    
}

// Defines a transformation matrix mat with a translation
void setTranslationMatrix(GLfloat *mat, GLfloat x, GLfloat y, GLfloat z) {
     
        setIdentityMatrix(mat,4);
        mat[12] = x;
        mat[13] = y;
        mat[14] = z;
}

void rotationMatrix(GLfloat *mat, GLfloat *axis, GLfloat angle)
{
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    GLfloat scratch[] = {
        oc *  axis[0] *  axis[0] + c,             oc *  axis[0] *  axis[1] -  axis[2] * s,  oc *  axis[2] *  axis[0] +  axis[1] * s,  0.0,
        oc *  axis[0] *  axis[1] +  axis[2] * s,  oc *  axis[1] *  axis[1] + c,             oc *  axis[1] *  axis[2] -  axis[0] * s,  0.0,
        oc *  axis[2] *  axis[0] -  axis[1] * s,  oc *  axis[1] *  axis[2] +  axis[0] * s,  oc *  axis[2] *  axis[2] + c,             0.0,
        0.0,                                      0.0,                                      0.0,                                      1.0};
    
    memcpy(mat, scratch, sizeof(scratch));
}
