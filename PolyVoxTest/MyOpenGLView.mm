//
//  MyOpenGLView.m
//  PolyVoxTest
//
//  Created by Daniel Parnell on 24/12/16.
//  Copyright © 2016 Automagic Software. All rights reserved.
//

#import "MyOpenGLView.h"
#import "pico_png.h"

#include "MyScene.hpp"

//Use the PolyVox namespace
using namespace PolyVox;

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

    NSData *tiles_png = [NSData dataWithContentsOfFile: [[NSBundle mainBundle] pathForResource: @"Tiles16x16" ofType: @"png"]];
    unsigned char decoded_tiles[640 * 592 * 3];
    unsigned long image_width;
    unsigned long image_height;
    decodePNG(decoded_tiles, sizeof(decoded_tiles), image_width, image_height, (const unsigned char*)[tiles_png bytes], [tiles_png length]);
    

    NSLog(@"Building meshes...");
    for(uint32_t i=0; i < image_width; i += 16) {
        for(uint32_t j=0; j < image_height; j += 16) {
            uint32_t base_address = (i*3) + (j*(uint32_t)image_width*3);
            
            RawVolume<uint32_t> volData(PolyVox::Region(Vector3DInt32(0, 0, 0), Vector3DInt32(17, 17, 17)));
            for(uint8_t x = 0; x<16; x++) {
                for(uint8_t y = 0; y<16; y++) {
                    uint32_t pixel_address = base_address + x*3 + y*(uint32_t)image_width*3;
                    unsigned char r = decoded_tiles[pixel_address + 0];
                    unsigned char g = decoded_tiles[pixel_address + 1];
                    unsigned char b = decoded_tiles[pixel_address + 2];
                    if(r != 0x47 || g != 0x6c || b != 0x6c) {
                        if(r == 0 && g == 0 && b == 0) {
                            r = 33;
                            g = 33;
                            b = 33;
                        }
                        
                        for(uint8_t z = 0; z<16; z++) {
                            volData.setVoxel(x, z, y, r << 24 | g << 16 | b << 8 | 0xff);
                        }
                    }
                }
            }
            
            // Extract the surface for the specified region of the volume. Uncomment the line for the kind of surface extraction you want to see.
            auto mesh = extractCubicMesh(&volData, volData.getEnclosingRegion());
            
            // The surface extractor outputs the mesh in an efficient compressed format which is not directly suitable for rendering. The easiest approach is to
            // decode this on the CPU as shown below, though more advanced applications can upload the compressed mesh to the GPU and decompress in shader code.
            auto decodedMesh = decodeMesh(mesh);
            
            //Pass the surface to the OpenGL window
            scene.addMesh(decodedMesh, Vector3DInt32(i - 320, 0, j - 296));
        }
    }
    
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
    if(h == 0) {
        h = 1;
    }
    
    // Set the viewport to be the entire window
    // glViewport(0, 0, w, h);
 
    ratio = (1.0f * w) / h;
    scene.buildProjectionMatrix(53.13f, ratio, 1.0f, 1200.0f);
}

float th = 0;

- (void)drawRect:(NSRect)dirtyRect {
    // scene.mMeshData[0].rotationAngle += 0.01;
    th += 0.01;
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene.setCamera(300.0 * sin(th), 100.0, 600.0 * cos(th), 0.0, 0.0, 0.0);
    
    scene.render();

    glFlush();
    [[self openGLContext] flushBuffer];
}

@end
