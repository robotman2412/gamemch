
#pragma once

class Attribute;
class AttributeSet;
class Blob;

#include "main.h"
#include <pax_gfx.h>
#include <vector>
#include <set>

extern std::vector<AttributeSet> attributeSets;
extern int debugAttrIndex;

typedef enum {
    // Commonplace.
    COMMON,
    // Slightly less common but still common.
    UNCOMMON,
    // Hard to obtain.
    RARE,
    // Very hard to obtain, only obtained by trading.
    VERY_RARE,
    // Hardest to obtain, only obtained by trading.
    LEGENDARY,
    // Can only be obtained by trading.
    UNOBTAINABLE,
} Rarity;

typedef enum {
    UNDEFINED,
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
            NUMBER,
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
        // The ID of this set.
        uint32_t id;
        // Mutually exclusive with.
        std::set<uint32_t> exclusive;
    public:
        // The name of this set.
        const char *name;
        // The network ID of this set.
        const char *netId;
        // All attributes affected.
        std::vector<Attribute> attributes;
        // The weight of this set in first time blob.
        float initialWeight;
        // The weight of this set in mutations.
        float mutationWeight;
        
        // Empty nameless set.
        AttributeSet();
        // Empty named set.
        // The name must not be deallocated.
        // Rarity is used to preset weights.
        AttributeSet(const char *netId, const char *name, Rarity rarity);
        // Deconstructor.
        ~AttributeSet();
        // Get the unique ID of the set.
        uint32_t getId();
        // Test whether a set is mutually exclusive with this one.
        bool isExclusive(AttributeSet &other);
        // Make thie set mutually exclusive with another set.
        void markExclusive(AttributeSet &other);
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
                // Current scale.
                float scale;
                // Animating from.
                float x0, y0, s0;
                // Animating to.
                float x1, y1, s1;
                // Animation start time.
                uint64_t start;
                // Animation end time.
                uint64_t end;
                
                // Origin position.
                Pos();
                // Position.
                Pos(float x, float y);
                // Position.
                Pos(float x, float y, float scale);
                // Position from data.
                Pos(const char *data);
                
                // Animate using coefficient.
                void coeffAnimate(float newCoeff);
                // Animate using time.
                void timeAnimate(int64_t now);
                // Set animation's target.
                void animateTo(float newX, float newY, int64_t duration);
                // Set animation's target.
                void animateTo(float newX, float newY, float newScale, int64_t duration);
                
                // Send data.
                void send(Connection *to, const char *topic);
                
                // Internal arm drawing method.
                void drawAsArm(Blob *blob);
                // Internal eye drawing method.
                void drawAsEye(Blob *blob);
        };
        // Attributes that affected the current look.
        std::vector<AttributeSet> attributes;
        // All attributes that have changed.
        std::set<Attribute::Affects> changes;
        
        // Position of all arms.
        std::vector<Pos> arms;
        // Position of all eyes.
        std::vector<Pos> eyes;
        
        // Position of the mouth.
        Pos       mouth;
        // Shape of the mouth, x=-1 to x=1, positive is happy.
        Pos       mouthBias;
        
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
        // On-screen position.
        Pos       pos;
        
        // Time at which to blink.
        int64_t  blinkTime;
        
        // Make a new blob with default attributes.
        // Does not apply any attributes.
        Blob();
        
        // Draw the blob.
        void draw();
        // Gets X counterpart for a given Y on the edge of the blob.
        float getEdgeX(float y, float angle);
        
        // Tests whether this blob has a given attribute set.
        // If so, returns the index.
        // If not, returns -1.
        int findSet(AttributeSet &set);
        // Toggle an attribute set.
        void toggleSet(AttributeSet &set);
        // Add an attribute set.
        void addSet(AttributeSet &set);
        // Remove an attribute set.
        void removeSet(AttributeSet &set);
        // Apply the blob's attributes from scratch.
        void redoAttributes();
        // Apply the blob's attributes, but only for changed affected variables.
        void applyAttributes();
        // Tests whether an attribute has changed.
        bool hasChanged(Attribute::Affects affected);
        // Mutate with another blob.
        void mutate(Blob *with);
        
        // Send the blob to a given connection.
        void send(Connection *to);
        // Receive blob data from a given connection.
        void receive(Connection *from, const char *topic, const char *data);
    private:
        // Calculate the value given by the attributes for a given affected variable.
        int calculateAttribute(Attribute::Affects affects, int min, int max);
};

AttributeSet *getSetById(const char *id);
void graphics_init();
void graphics_task();
