
#include "graphics.h"
#include "player.h"
#include "string.h"



static const char *TAG = "graphics";
AttributeSet defaultSet;
AttributeSet darkSide;
AttributeSet slime;
std::vector<AttributeSet> attributeSets;

static void initAttributeSets() {
    // Defaults:
        // Square or round body.
        defaultSet.add(Attribute::BODY_SHAPE, Attribute::SET, Shape::SQUARE, 1, 1);
        defaultSet.add(Attribute::BODY_SHAPE, Attribute::SET, Shape::CIRCLE, 1, 1);
        // Two traditional eyes.
        defaultSet.add(Attribute::EYE_COUNT,  Attribute::SET, 2, 1, 1);
        defaultSet.add(Attribute::EYE_TYPE,   Attribute::SET, EyeType::TRADITIONAL, 1, 1);
        
        // Basically any eye hue.
        defaultSet.add(Attribute::EYE_HUE,    Attribute::SET, 1, 1000, 1);
        // Mostly full eye saturation.
        defaultSet.add(Attribute::EYE_SAT,    Attribute::SET, 250, 10, 1);
        // Mostly full eye brightness.
        defaultSet.add(Attribute::EYE_BRI,    Attribute::SET, 190, 30, 1);
        
        // Blueish body hue.
        defaultSet.add(Attribute::BODY_HUE,   Attribute::SET, 165, 70, 1);
        // Full body saturation.
        defaultSet.add(Attribute::BODY_SAT,   Attribute::SET, 255, 50, 1);
        // Mostly full body brightness.
        defaultSet.add(Attribute::BODY_BRI,   Attribute::SET, 230, 50, 1);
        
        // Full alt saturation.
        defaultSet.add(Attribute::ALT_SAT,    Attribute::SET, 255, 50, 1);
        // Less alt brightness.
        defaultSet.add(Attribute::ALT_BRI,    Attribute::SET, 130, 30, 1);
    // The default set always applies.
    
    // Dark side.
    darkSide = AttributeSet("Dark Side");
    darkSide.initialWeight  = 0.1;
    darkSide.mutationWeight = 0.1;
        // Dark body.
        darkSide.add(Attribute::BODY_BRI,  Attribute::SET, 50, 16, 5);
        // Slightly brigher outline.
        darkSide.add(Attribute::ALT_BRI,   Attribute::SET, 70, 16, 5);
        // Less eye saturation.
        darkSide.add(Attribute::EYE_SAT,   Attribute::SET, 130, 20, 3);
        // More eye brightness.
        darkSide.add(Attribute::EYE_BRI,   Attribute::SET, 255, 50, 10);
        // Chance of dark eyes.
        darkSide.add(Attribute::EYE_TYPE,  Attribute::SET, EyeType::DARK, 1, 2);
    attributeSets.push_back(darkSide);
    
    // Slime.
    slime = AttributeSet("Slime");
}

void graphics_init() {
    // Init attribute sets before blob.
    initAttributeSets();
}

void graphics_task() {
    static bool hadCompanion = false;
    static bool wasLoaded = false;
    pax_background(&buf, 0);
    
    // Show number of nearby people.
    if (companion && !companionAgrees) {
        pax_draw_text(&buf, -1, pax_font_saira_regular, 18, 5, 5, "🅱 Cancel");
    } else if (companion) {
        pax_draw_text(&buf, -1, pax_font_saira_regular, 18, 5, 5, "🅱 Leave");
    } else if (!nearby) {
        pax_draw_text(&buf, -1, pax_font_saira_regular, 18, 5, 5, "Nobody nearby");
    } else if (nearby == 1) {
        pax_draw_text(&buf, -1, pax_font_saira_regular, 18, 5, 5, "🅴 1 person nearby");
    } else {
        char *tmp = new char[128];
        snprintf(tmp, 128, "🅴 %d people nearby", nearby);
        pax_draw_text(&buf, -1, pax_font_saira_regular, 18, 5, 5, tmp);
        delete tmp;
    }
    
    // Draw the AVATAARRRRR.
    if (companion) {
        if (!hadCompanion) {
            localPlayer->blob->pos.animateTo(buf.width/3, buf.height/2, 35, 1000);
            companion->blob->pos = Blob::Pos(buf.width*2/3, buf.height/2, 0);
            companion->blob->pos.animateTo(buf.width*2/3, buf.height/2, 35, 1000);
        }
        
        localPlayer->blob->draw();
        companion->blob->draw();
        hadCompanion = true;
    } else {
        if (hadCompanion) {
            localPlayer->blob->pos.animateTo(buf.width/2, buf.height/2, 50, 1000);
        }
        localPlayer->blob->draw();
        hadCompanion = false;
        wasLoaded = false;
    }
    
    disp_flush();
}



Attribute::Attribute() {}

// Constructor for the handy.
Attribute::Attribute(Affects affects, Mode mode, float value, float range, float power) {
    this->affects = affects;
    this->mode    = mode;
    this->value   = value;
    this->range   = range;
    this->power   = power;
}

// Empty nameless set.
AttributeSet::AttributeSet() {
    this->name = strdup("");
}

// Empty named set.
AttributeSet::AttributeSet(const char *name) {
    this->name = strdup(name);
}

// Deconstructor.
AttributeSet::~AttributeSet() {
    delete name;
}

// Add an attribute to the set.
void AttributeSet::add(Attribute toAdd) {
    attributes.push_back(toAdd);
}

// Add an attribute to the set.
void AttributeSet::add(Attribute::Affects affects, Attribute::Mode mode, float value, float range, float power) {
    attributes.push_back(Attribute(affects, mode, value, range, power));
}


// Make a new blob with default attributes.
// Does not apply any attributes.
Blob::Blob() {
    // Defaults in case attributes break.
    body      = Shape::PENTAGON;
    bodyColor = 0xffd07000;
    altColor  = 0xff804000;
    eyeColor  = 0xffe09030;
    eyeType   = EyeType::DARK;
    eyes.push_back(Pos(-0.4, -0.5));
    eyes.push_back(Pos( 0.4, -0.5));
    mouth     = Pos(0.0, 0.4);
    mouthBias = Pos();
    lookingAt = Pos();
    pos       = Pos(buf.width/2.0, buf.height/2.0, 50);
    blinkTime = 0;
    
    // Attributes.
    attributes.push_back(defaultSet);
    // attributes.push_back(darkSide);
}

// Draws an n-gon based on a circle.
static void draw_ngon(size_t num, pax_col_t bodyColor, pax_col_t altColor, float edge) {
    // Rotate such that the shape is flat at the bottom.
    pax_push_2d(&buf);
    pax_apply_2d(&buf, matrix_2d_rotate(M_PI/-2 + M_PI/num));
    
    // Fix parameter range.
    num  ++;
    edge ++;
    // Start with a circle.
    pax_vec1_t points[num];
    pax_vectorise_circle(points, num, 0, 0, 1);
    
    // Draw the outline.
    for (size_t i = 0; i < num; i++) {
        pax_vec1_t cur  = points[i];
        pax_vec1_t next = points[(i + 1) % num];
        pax_draw_tri(
            &buf,        altColor,
            cur.x,       cur.y,
            cur.x*edge,  cur.y*edge,
            next.x*edge, next.y*edge
        );
        pax_draw_tri(
            &buf,        altColor,
            cur.x,       cur.y,
            next.x,      next.y,
            next.x*edge, next.y*edge
        );
    }
    // Draw the infill.
    for (size_t i = 0; i < num; i++) {
        pax_vec1_t cur  = points[i];
        pax_vec1_t next = points[(i + 1) % num];
        pax_draw_tri(
            &buf,   bodyColor,
            cur.x,  cur.y,
            next.x, next.y,
            0,      0
        );
    }
    
    // Restore transform.
    pax_pop_2d(&buf);
}

// Draw the blob.
void Blob::draw() {
    float edge = 0.2;
    static int64_t nextRandom = 0;
    int64_t now = esp_timer_get_time() / 1000;
    if (nextRandom <= now) {
        nextRandom      = now + 3000;
        float a         = esp_random() / (float) INT32_MAX * M_PI;
        lookingAt.animateTo(cosf(a), sinf(a), 500);
        if (nearby) {
            mouthBias.animateTo(esp_random() / (float) UINT32_MAX, 0.0, 1000);
        } else {
            mouthBias.animateTo((int) esp_random() / (float) INT32_MAX, 0.0, 1000);
        }
    }
    if (now >= blinkTime + 300) {
        blinkTime = now + 4000 + (esp_random() / (float) UINT32_MAX * 3000.0);
    }
    
    // Apply position and scale.
    pax_push_2d(&buf);
    pax_apply_2d(&buf, matrix_2d_translate(pos.x, pos.y));
    pax_apply_2d(&buf, matrix_2d_scale(pos.scale, pos.scale));
    
    
    switch (body) {
        default:
        case(SQUARE):
            // Square body.
            pax_draw_rect(&buf, altColor,  -1, -1, 2, 2);
            pax_draw_rect(&buf, bodyColor, edge-1, edge-1, 2-2*edge, 2-2*edge);
            break;
        case(PENTAGON):
            // Pentagonal body.
            draw_ngon(5, bodyColor, altColor, edge);
            break;
        case(HEXAGON):
            // Hexagonal body.
            draw_ngon(6, bodyColor, altColor, edge);
            break;
        case(CIRCLE):
            // Circular body.
            pax_draw_circle(&buf, altColor,  0, 0, 1.1+edge);
            pax_draw_circle(&buf, bodyColor, 0, 0, 1.1);
            break;
    }
    
    
    // Draw all arms.
    for (size_t i = 0; i < arms.size(); i++) {
        arms[i].timeAnimate(now);
        arms[i].drawAsArm(this);
    }
    // Draw all eyes.
    for (size_t i = 0; i < eyes.size(); i++) {
        eyes[i].timeAnimate(now);
        eyes[i].drawAsEye(this);
    }
    
    // Animate misc.
    mouthBias.timeAnimate(now);
    lookingAt.timeAnimate(now);
    pos.timeAnimate(now);
    
    
    // Position of mouth.
    float mouthScaleY = 0.10;
    float mouthScaleX = mouthScaleY * (1 + 0.5 * fabsf(mouthBias.x));
    pax_apply_2d(&buf, matrix_2d_translate(mouth.x, mouth.y));
    pax_apply_2d(&buf, matrix_2d_scale(mouthScaleX, mouthScaleY));
    
    // Draw mouth, but keep it convex.
    pax_vec1_t mouth[16];
    pax_vectorise_circle(mouth, 16, 0, 0, 1);
    // Nudge the edges.
    for (int i = 0; i < 16; i++) {
        float effect = (1 - fabsf(mouth[i].y));
        mouth[i].y -= effect * mouthBias.x;
    }
    // Draw it like normal.
    for (int i = 1; i < 15; i++) {
        pax_draw_tri(
            &buf, altColor,
            mouth[0].x,   mouth[0].y,
            mouth[i].x,   mouth[i].y,
            mouth[i+1].x, mouth[i+1].y
        );
    }
    
    // Restore transformation.
    pax_pop_2d(&buf);
}

// Gets X counterpart for a given Y on the edge of the blob.
float Blob::getEdgeX(float y, float angle) {
    return 1;
}

// Apply the blob's attributes.
// Even for the same attributes, the blob will look slightly different.
void Blob::applyAttributes() {
    int hue, sat, bri;
    // Body shape.
    body = (Shape) calculateAttribute(Attribute::BODY_SHAPE, Shape::SQUARE, Shape::CIRCLE);
    
    // Body color.
    hue = calculateAttribute(Attribute::BODY_HUE, 0, 255);
    sat = calculateAttribute(Attribute::BODY_SAT, 0, 255);
    bri = calculateAttribute(Attribute::BODY_BRI, 0, 255);
    bodyColor = pax_col_hsv(hue, sat, bri);
    
    // Alt color.
    sat = calculateAttribute(Attribute::ALT_SAT, 0, 255);
    bri = calculateAttribute(Attribute::ALT_BRI, 0, 255);
    altColor = pax_col_hsv(hue, sat, bri);
    
    // Eye color.
    hue = calculateAttribute(Attribute::EYE_HUE, 0, 255);
    sat = calculateAttribute(Attribute::EYE_SAT, 0, 255);
    bri = calculateAttribute(Attribute::EYE_BRI, 0, 255);
    eyeColor = pax_col_hsv(hue, sat, bri);
    
    // Eye type.
    eyeType = (EyeType) calculateAttribute(Attribute::EYE_TYPE, EyeType::TRADITIONAL, EyeType::CUTOUT);
}

// Adds a curve to the probabilities list.
static void addCurve(float *probabilities, int min, int max, float addCenter, float range, float power, bool isForbid) {
    // Iterate over the array.
    for (int i = min; i <= max; i++) {
        // If it is in range...
        float dist = fabsf(i - addCenter);
        if (dist < range) {
            float relativePower = (range - dist) / range * power;
            if (isForbid) {
                // Forbidding decreases the THING.
                probabilities[i-min] *= (1-relativePower);
            } else {
                // Otherwise, just add the constant.
                probabilities[i-min] += relativePower;
            }
        }
    }
}

// Calculate a value for the affected variable given attribute sets.
int Blob::calculateAttribute(Attribute::Affects affects, int min, int max) {
    // Number of possible values.
    int possible      = (max - min) + 1;
    // Start all probabilities at 0.
    float *probabilities = new float[possible];
    for (int i = 0; i < possible; i++) probabilities[i] = 0;
    
    // Set pass.
    for (int x = 0; x < attributes.size(); x++) {
        AttributeSet *set = &attributes[x];
        for (int y = 0; y < set->attributes.size(); y++) {
            Attribute *attr = &set->attributes[y];
            if (attr->affects == affects && attr->mode == Attribute::SET) {
                // Add it's value to the probs.
                addCurve(probabilities, min, max, attr->value, attr->range, attr->power, false);
            }
        }
    }
    
    // TODO: Add pass.
    
    // Forbid pass.
    for (int x = 0; x < attributes.size(); x++) {
        AttributeSet *set = &attributes[x];
        for (int y = 0; y < set->attributes.size(); y++) {
            Attribute *attr = &set->attributes[y];
            if (attr->affects == affects && attr->mode == Attribute::FORBID) {
                // Add it's value to the probs.
                addCurve(probabilities, min, max, attr->value, attr->range, attr->power, true);
            }
        }
    }
    
    // Find total probability.
    float total = 0;
    for (int i = 0; i < possible; i++) {
        total += probabilities[i];
    }
    // Pick the outcome.
    float pick  = esp_random() / (float) UINT32_MAX * total;
    int   value = min;
    for (int i = 0; i < possible; i++) {
        pick -= probabilities[i];
        if (pick <= 0) {
            value = min + i;
            break;
        }
    }
    
    // Clean up.
    delete probabilities;
    return value;
}


// Send the blob to a given connection.
void Blob::send(Connection *to) {
    char *temp = new char[32];
    // Send colors.
    to->sendNum("blob_body_col", bodyColor);
    to->sendNum("blob_alt_col",  altColor);
    to->sendNum("blob_eye_col",  eyeColor);
    // Send eyes.
    to->sendNum("blob_eye_type", (int) eyeType);
    to->sendNum("blob_eye_count", eyes.size());
    for (int i = 0; i < eyes.size(); i++) {
        snprintf(temp, 32, "blob_eye/%d", i);
        eyes[i].send(to, temp);
    }
    // Send looking direction.
    lookingAt.send(to, "blob_looking");
    // Send body type.
    to->sendNum("blob_body", (int) body);
    // Send mouth.
    mouth.send(to, "blob_mouth");
    mouthBias.send(to, "blob_mouth_bias");
    
    delete temp;
}

// Receive blob data from a given connection.
void Blob::receive(Connection *from, const char *tmpTopic, const char *tmpData) {
    ESP_LOGI(TAG, "I was ACK! (%s: %s)", tmpTopic, tmpData);
    if (!strcmp(tmpTopic, "blob_body")) {
        body = (Shape) atoi(tmpData);
    } else if (!strcmp(tmpTopic, "blob_body_col")) {
        bodyColor = atoi(tmpData);
    } else if (!strcmp(tmpTopic, "blob_alt_col")) {
        altColor = atoi(tmpData);
    } else if (!strcmp(tmpTopic, "blob_looking")) {
        lookingAt = Pos(tmpData);
    } else if (!strcmp(tmpTopic, "blob_mouth")) {
        mouth = Pos(tmpData);
    } else if (!strcmp(tmpTopic, "blob_mouth_bias")) {
        mouthBias = Pos(tmpData);
    } else if (!strcmp(tmpTopic, "blob_eye_col")) {
        eyeColor = atoi(tmpData);
    } else if (!strcmp(tmpTopic, "blob_eye_type")) {
        eyeType = (EyeType) atoi(tmpData);
    } else if (!strcmp(tmpTopic, "blob_eye_count")) {
        // eyes.clear();
    }
}


// Origin position.
Blob::Pos::Pos() {
    x=x0=x1=y=y0=y1=0;
    scale=s0=s1=1;
}

// Position.
Blob::Pos::Pos(float sx, float sy) {
    x  = sx; y  = sy; scale = 1;
    x0 = sx; y0 = sy; s0 = 1;
    x1 = sx; y1 = sy; s1 = 1;
}

// Position.
Blob::Pos::Pos(float sx, float sy, float ss) {
    x  = sx; y  = sy; scale = ss;
    x0 = sx; y0 = sy; s0 = ss;
    x1 = sx; y1 = sy; s1 = ss;
}

// Position from data.
Blob::Pos::Pos(const char *tmpData) {
    char *data = strdup(tmpData);
    char *pos  = strchr(data, ',');
    if (pos) {
        *pos = 0;
        float sx = atoff(data);
        float sy = atoff(pos+1);
        x  = sx; y  = sy;
        x0 = sx; y0 = sy;
        x1 = sx; y1 = sy;
    }
    free(data);
}

// Animate using coefficient.
void Blob::Pos::coeffAnimate(float newCoeff) {
    // -2x³+3x²
    float coeff = -2*newCoeff*newCoeff*newCoeff + 3*newCoeff*newCoeff;
    x     = x0 + (x1 - x0) * coeff;
    y     = y0 + (y1 - y0) * coeff;
    scale = s0 + (s1 - s0) * coeff;
}

// Animate using time.
void Blob::Pos::timeAnimate(int64_t now) {
    float coeff;
    if (now <= start) coeff = 0;
    else if (now >= end) coeff = 1;
    else coeff = (float) (now - start) / (float) (end - start);
    coeffAnimate(coeff);
}

// Set animation's target.
void Blob::Pos::animateTo(float newX, float newY, int64_t duration) {
    x0 = x;
    y0 = y;
    s0 = scale;
    x1 = newX;
    y1 = newY;
    s1 = scale;
    start = esp_timer_get_time() / 1000;
    end   = start + duration;
}

// Set animation's target.
void Blob::Pos::animateTo(float newX, float newY, float newScale, int64_t duration) {
    x0 = x;
    y0 = y;
    s0 = scale;
    x1 = newX;
    y1 = newY;
    s1 = newScale;
    start = esp_timer_get_time() / 1000;
    end   = start + duration;
}


// Send data.
void Blob::Pos::send(Connection *to, const char *topic) {
    // Allocate some memory.
    char *temp       = new char[Connection_BUF_LEN];
    // Adjust animation times.
    int64_t now      = esp_timer_get_time() / 1000;
    int64_t startAlt = start - now;
    int64_t endAlt   = end   - now;
    // Format and send it!
    snprintf(temp, 32, "%f,%f", x1, y1);
    to->send(topic, temp);
    
    delete temp;
}


// Internal arm drawing method.
void Blob::Pos::drawAsArm(Blob *blob) {
    
}

// Internal eye drawing method.
void Blob::Pos::drawAsEye(Blob *blob) {
    int64_t now = esp_timer_get_time() / 1000;
    int64_t blinkLength = 150;
    float scale  = 0.2;
    float lookX  = blob->lookingAt.x;
    float lookY  = blob->lookingAt.y;
    
    // Apply position and scale.
    pax_push_2d(&buf);
    pax_apply_2d(&buf, matrix_2d_translate(x, y));
    pax_apply_2d(&buf, matrix_2d_scale(scale, scale));
    
    // Apply blinking.
    if (now >= blob->blinkTime - blinkLength && now <= blob->blinkTime + blinkLength) {
        float blink = fabsf((blob->blinkTime - now) / (float) blinkLength);
        blink *= blink * blink;
        pax_apply_2d(&buf, matrix_2d_scale(1, blink));
    }
    
    switch (blob->eyeType) {
        default:
        case(EyeType::TRADITIONAL):
            // More traditional eye.
            pax_draw_circle(&buf, 0xffffffff, 0, 0, 1);
            pax_draw_circle(&buf, blob->eyeColor, lookX*0.2, lookY*0.2, 0.7);
            break;
        case(EyeType::DARK):
            // Dark colored traditional eye.
            pax_draw_circle(&buf, 0xff000000, 0, 0, 1);
            pax_draw_circle(&buf, blob->eyeColor, lookX*0.2, lookY*0.2, 0.7);
            break;
        case(EyeType::CUTOUT):
            // Eye is like a cutout.
            pax_draw_circle(&buf, blob->altColor, lookX*0.3, lookY*0.3, 1);
            break;
    }
    
    // Restore transformation.
    pax_pop_2d(&buf);
}
