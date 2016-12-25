//
//  MyOpenGLView.m
//  PolyVoxTest
//
//  Created by Daniel Parnell on 24/12/16.
//  Copyright © 2016 Automagic Software. All rights reserved.
//

#import "MyOpenGLView.h"


#include "MyScene.hpp"

//Use the PolyVox namespace
using namespace PolyVox;

void createSphereInVolume(RawVolume<uint32_t>& volData, float fRadius, uint32_t colour)
{
    //This vector hold the position of the center of the volume
    Vector3DFloat v3dVolCenter(volData.getWidth() / 2, volData.getHeight() / 2, volData.getDepth() / 2);
    
    //This three-level for loop iterates over every voxel in the volume
    for (int z = 0; z < volData.getDepth(); z++)
    {
        for (int y = 0; y < volData.getHeight(); y++)
        {
            for (int x = 0; x < volData.getWidth(); x++)
            {
                //Store our current position as a vector...
                Vector3DFloat v3dCurrentPos(x, y, z);
                //And compute how far the current position is from the center of the volume
                float fDistToCenter = (v3dCurrentPos - v3dVolCenter).length();
                
                uint32_t uVoxelValue = 0;
                
                //If the current voxel is less than 'radius' units from the center then we make it solid.
                if (fDistToCenter <= fRadius)
                {
                    //Our new voxel value
                    uVoxelValue =  colour;
                }
                
                //Wrte the voxel value into the volume	
                volData.setVoxel(x, y, z, uVoxelValue);
            }
        }
    }
}

@implementation MyOpenGLView {
    MyScene scene;
}

- (void) awakeFromNib {
    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        NSOpenGLPFAColorSize    , 24                           ,
        NSOpenGLPFAAlphaSize    , 8                            ,
        NSOpenGLPFADepthSize    , 16                           ,
        NSOpenGLPFADoubleBuffer ,
        NSOpenGLPFAAccelerated  ,
        NSOpenGLPFANoRecovery   ,
        0
    };
    
    NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes: attrs];
    self.pixelFormat = pf;
    self.openGLContext = [[NSOpenGLContext alloc] initWithFormat:self.pixelFormat shareContext:nil];
    CGLContextObj c = self.openGLContext.CGLContextObj;
    CGLEnable(c, kCGLCECrashOnRemovedFunctions);
    CGLSetCurrentContext(c);
    
    [[self openGLContext] makeCurrentContext];
    
    glEnable(GL_CULL_FACE);
    
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glDepthRange(0.0, 1.0);

    glClearColor(0, 0, 0, 0);
    glClearDepth(1.0);
    
    NSString *vertexShader = [NSString stringWithContentsOfFile: [[NSBundle mainBundle] pathForResource: @"vertex" ofType: @"glsl"] encoding: NSUTF8StringEncoding error: nil];
    NSString *fragmentShader = [NSString stringWithContentsOfFile: [[NSBundle mainBundle] pathForResource: @"fragment" ofType: @"glsl"] encoding: NSUTF8StringEncoding error: nil];
    
    scene.setupShaders(vertexShader.UTF8String, fragmentShader.UTF8String);
    
    // Create an empty volume and then place a sphere in it
    RawVolume<uint32_t> volData(PolyVox::Region(Vector3DInt32(0, 0, 0), Vector3DInt32(63, 63, 63)));
    createSphereInVolume(volData, 30, 0xffffffff);
    volData.setVoxel(0, 32, 0, 0x0000ffff);
    volData.setVoxel(0,  0, 0, 0x00ff00ff);
    volData.setVoxel(0, 62, 0, 0xff0000ff);
    
    for(int x=0; x<62; x++) {
        for(int y=0; y<62; y++) {
            volData.setVoxel(x, y, 62, rand() << 8);
        }
    }
    // Extract the surface for the specified region of the volume. Uncomment the line for the kind of surface extraction you want to see.
    auto mesh = extractCubicMesh(&volData, volData.getEnclosingRegion());
    
    // The surface extractor outputs the mesh in an efficient compressed format which is not directly suitable for rendering. The easiest approach is to
    // decode this on the CPU as shown below, though more advanced applications can upload the compressed mesh to the GPU and decompress in shader code.
    auto decodedMesh = decodeMesh(mesh);
    
    //Pass the surface to the OpenGL window
    scene.addMesh(decodedMesh);
    
    [NSTimer scheduledTimerWithTimeInterval: 1.0/50.0 repeats: YES block:^(NSTimer * _Nonnull timer) {
        [self setNeedsDisplay: YES];
    }];
}

- (void) setFrame:(NSRect)frame {
    [super setFrame: frame];
    
    GLfloat w = frame.size.width;
    GLfloat h = frame.size.height;
    
    float ratio;
    // Prevent a divide by zero, when window is too short
    // (you cant make a window of zero width).
    if(h == 0)
            h = 1;
 
    // Set the viewport to be the entire window
    // glViewport(0, 0, w, h);
 
    ratio = (1.0f * w) / h;
    scene.buildProjectionMatrix(53.13f, ratio, 1.0f, 300.0f);
}

- (void)drawRect:(NSRect)dirtyRect {
    scene.mMeshData[0].rotationAngle += 0.01;
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene.setCamera(100.0, 100.0, 100.0, 0.0, 0.0, 0.0);
    
    scene.render();

    glFlush();
    [[self openGLContext] flushBuffer];
}

@end
