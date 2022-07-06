
#pragma once
#include "main.h"
#include <pax_gfx.h>

typedef enum {
    TRIANGLE,
    SQUARE,
    PENTAGON,
    HEXAGON,
    CIRCLE,
} Shape;

typedef enum {
    TRADITIONAL,
    CUTOUT,
} EyeType;

class Blob {
    public:
        class Pos {
            public:
                // Current position.
                float x, y;
                // Animating from.
                float x0, y0;
                // Animating to.
                float x1, y1;
                // Animation start time.
                uint64_t start;
                // Animation end time.
                uint64_t end;
                
                // Origin position.
                Pos();
                // Position.
                Pos(float x, float y);
                
                // Animate using coefficient.
                void coeffAnimate(float newCoeff);
                // Animate using time.
                void timeAnimate(uint64_t now);
                
                // Internal arm drawing method.
                void drawAsArm(Blob *blob);
                // Internal eye drawing method.
                void drawAsEye(Blob *blob);
        };
        
        // Position of all arms.
        std::vector<Pos> arms;
        // Position of all eyes.
        std::vector<Pos> eyes;
        // Position of the mouth.
        Pos   mouth;
        // Shape of the mouth, -1 to 1, positive is happy.
        float     mouthBias;
        // Next mouth bias to animate towards.
        float     targetBias;
        // Color of the body.
        pax_col_t bodyColor;
        // Color of the outline of the body.
        pax_col_t altColor;
        // Color of the eyes.
        pax_col_t eyeColor;
        // Shape of the blob.
        Shape     body;
        // Type of eye.
        EyeType   eyeType;
        // Looking at, never greater than 1 magnitude.
        Pos       lookingAt;
        
        Blob();
        
        void draw(float x, float y, float scale);
        float getEdgeX(float y, float angle);
};


void graphics_task();
