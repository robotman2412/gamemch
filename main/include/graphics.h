
#pragma once

class Attribute;
class AttributeSet;
class Blob;

#include "main.h"
#include <pax_gfx.h>
#include <vector>

extern std::vector<AttributeSet> attributeSets;

typedef enum {
    TRIANGLE,
    SQUARE,
    PENTAGON,
    HEXAGON,
    CIRCLE,
} Shape;

typedef enum {
    TRADITIONAL,
    DARK,
    CUTOUT,
} EyeType;

// An attribute that changes the way the blob looks or behaves.
// Always used in an attribute set.
class Attribute {
    public:
        // Attribute to affect.
        typedef enum {
            // Shape of body (1 to 4).
            BODY_SHAPE,
            // Body color: Hue (0 to 255).
            BODY_HUE,
            // Body color: Saturation (0 to 255).
            BODY_SAT,
            // Body color: Brightness (0 to 255).
            BODY_BRI,
            // Outline color: Saturation (0 to 255).
            ALT_SAT,
            // Outline color: Brightness (0 to 255).
            ALT_BRI,
            
            // Type of eyes (0 to 2).
            EYE_TYPE,
            // Number of eyes (1 to 8).
            EYE_COUNT,
            // Eye color: Hue (0 to 255).
            EYE_HUE,
            // Eye color: Saturation (0 to 255).
            EYE_SAT,
            // Eye color: Brightness (0 to 255).
            EYE_BRI,
        } Affects;
        // Ways in which attributes are affected.
        typedef enum {
            // Make the values much less likely.
            FORBID,
            // Add a to a value.
            ADD,
            // Subtract from a value.
            SET,
        } Mode;
        
        Attribute();
        // Constructor for the handy.
        Attribute(Affects affects, Mode mode, float value, float range, float power);
        // Attribute to affect.
        Affects affects;
        // Mode in which to affect.
        Mode    mode;
        // Value:
        // When adding, this is the average amount added.
        // When setting, this is the average to set.
        // When forbidding, this is the value to forbid.
        float   value;
        // Range: the distribution to affect.
        float   range;
        // Power: the strength by which to affect the range.
        float   power;
};

// An collection of attributes that are applied simultaneously.
class AttributeSet {
    private:
        // The name of this set.
        char *name;
    public:
        // All attributes affected.
        std::vector<Attribute> attributes;
        
        // Empty nameless set.
        AttributeSet();
        // Empty named set.
        AttributeSet(const char *name);
        // Deconstructor.
        ~AttributeSet();
        // Get the name of the set.
        const char *getName();
        // Add an attribute to the set.
        void add(Attribute toAdd);
        // Add an attribute to the set.
        void add(Attribute::Affects affects, Attribute::Mode mode, float value, float range, float power);
};

#include "connection.h"

// How a blob is shown.
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
                // Position from data.
                Pos(const char *data);
                
                // Animate using coefficient.
                void coeffAnimate(float newCoeff);
                // Animate using time.
                void timeAnimate(int64_t now);
                // Set animation's target.
                void animateTo(float newX, float newY, int64_t duration);
                
                // Send data.
                void send(Connection *to, const char *topic);
                
                // Internal arm drawing method.
                void drawAsArm(Blob *blob);
                // Internal eye drawing method.
                void drawAsEye(Blob *blob);
        };
        // Attributes that affected the current look.
        std::vector<AttributeSet> attributes;
        
        // Position of all arms.
        std::vector<Pos> arms;
        // Position of all eyes.
        std::vector<Pos> eyes;
        
        // Position of the mouth.
        Pos       mouth;
        // Shape of the mouth, x=-1 to x=1, positive is happy.
        // Pos       mouthBias;
        float mouthBias, targetBias;
        
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
        // Looking at, never greater than 1 in magnitude.
        Pos       lookingAt;
        
        // Time at which to blink.
        int64_t  blinkTime;
        
        // Make a new blob with default attributes.
        // Does not apply any attributes.
        Blob();
        
        // Draw the blob.
        void draw(float x, float y, float scale);
        // Gets X counterpart for a given Y on the edge of the blob.
        float getEdgeX(float y, float angle);
        // Apply the blob's attributes.
        // Even for the same attributes, the blob will look slightly different.
        void applyAttributes();
        // Send the blob to a given connection.
        void send(Connection *to);
        // Receive blob data from a given connection.
        void receive(Connection *from, const char *topic, const char *data);
    private:
        // Calculate the value given by the attributes for a given affected variable.
        int calculateAttribute(Attribute::Affects affects, int min, int max);
};

void graphics_init();
void graphics_task();
