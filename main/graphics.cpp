
#include "graphics.h"
#include "player.h"
#include "string.h"



static const char *TAG = "graphics";

AttributeSet defaultSet ("defaultSet",  "", Rarity::UNOBTAINABLE);

/* ==== eyes ==== */
AttributeSet eyeBlue    ("eyeBlue",     "Blue Eyes",   Rarity::UNCOMMON);
AttributeSet eyeGreen   ("eyeGreen",    "Green Eyes",  Rarity::COMMON);
AttributeSet eyeBrown   ("eyeBrown",    "Brown Eyes",  Rarity::COMMON);
AttributeSet eyePurple  ("eyePurple",   "Purple Eyes", Rarity::VERY_RARE);
AttributeSet eyeOrange  ("eyeOrange",   "Orange Eyes", Rarity::VERY_RARE);
AttributeSet eyeRed     ("eyeRed",      "Red Eyes",    Rarity::LEGENDARY);
AttributeSet eyeBlack   ("eyeBlack",    "Black Eyes",  Rarity::LEGENDARY);

/* ==== body ==== */
AttributeSet darkSide   ("darkSide",    "Dark Side",   Rarity::RARE);
AttributeSet slime      ("slime",       "Slime",       Rarity::RARE);

std::vector<AttributeSet> attributeSets;
int debugAttrIndex = 0;
static const size_t temp_len = 1024;
static char *temp;

static void initAttributeSets() {
    attributeSets = std::vector<AttributeSet>();
    
    { // Defaults:
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
        defaultSet.add(Attribute::BODY_HUE,   Attribute::SET, 165, 30, 1);
        // Full body saturation.
        defaultSet.add(Attribute::BODY_SAT,   Attribute::SET, 255, 50, 1);
        // Mostly full body brightness.
        defaultSet.add(Attribute::BODY_BRI,   Attribute::SET, 230, 50, 1);
        
        // Full alt saturation.
        defaultSet.add(Attribute::ALT_SAT,    Attribute::SET, 255, 50, 1);
        // Less alt brightness.
        defaultSet.add(Attribute::ALT_BRI,    Attribute::SET, 130, 30, 1);
    } // The default set always applies.
    
    /* ==== eyes ==== */
    
    { // Blue eyes.
        // Blue to cyan hue.
        eyeBlue.add(Attribute::EYE_HUE, Attribute::SET, 133, 5, 5);
    attributeSets.push_back(eyeBlue);
    } // Uncommon set.
    
    { // Brown eyes.
        eyeBrown.add(Attribute::EYE_HUE, Attribute::SET, 28, 5, 5);
        eyeBrown.add(Attribute::EYE_BRI, Attribute::SET, 90, 10, 5);
    attributeSets.push_back(eyeBrown);
    } // Common set.
    
    { // Green eyes.
        // Blue to cyan hue.
        eyeGreen.add(Attribute::EYE_HUE, Attribute::SET, 75, 5, 5);
    attributeSets.push_back(eyeGreen);
    } // Uncommon set.
    
    { // Purple eyes.
        eyePurple.add(Attribute::EYE_HUE, Attribute::SET, 194, 5, 5);
    attributeSets.push_back(eyePurple);
    } // Very rare set.
    
    { // Orange eyes.
        eyeOrange.add(Attribute::EYE_HUE, Attribute::SET, 28, 5, 5);
        eyeBrown.add(Attribute::EYE_BRI, Attribute::SET, 255, 5, 5);
        eyeBrown.add(Attribute::EYE_SAT, Attribute::SET, 210, 5, 5);
    attributeSets.push_back(eyeOrange);
    } // Very rare set.
    
    /* ==== body ==== */
    
    { // Dark side.
        // Dark body.
        darkSide.add(Attribute::BODY_BRI,  Attribute::SET, 90, 7, 5);
        // Slightly brigher outline.
        darkSide.add(Attribute::ALT_BRI,   Attribute::SET, 130, 7, 5);
        // Less eye saturation.
        darkSide.add(Attribute::EYE_SAT,   Attribute::SET, 130, 20, 3);
        // More eye brightness.
        darkSide.add(Attribute::EYE_BRI,   Attribute::SET, 240, 50, 10);
        // Chance of dark eyes.
        darkSide.add(Attribute::EYE_TYPE,  Attribute::SET, EyeType::DARK, 1, 2);
    attributeSets.push_back(darkSide);
    } // Rare set.
    
    { // Slime.
        // Tends to be green.
        slime.add(Attribute::BODY_HUE, Attribute::SET, 96, 5, 10);
        // Adds cutout eyes.
        slime.add(Attribute::EYE_TYPE, Attribute::SET, EyeType::CUTOUT, 1, 5);
        slime.add(Attribute::EYE_TYPE, Attribute::FORBID, EyeType::TRADITIONAL, 1, 1);
        slime.add(Attribute::EYE_TYPE, Attribute::FORBID, EyeType::DARK, 1, 1);
        // Incompatible with eye colors.
        slime.markExclusive(eyeBlue);
        slime.markExclusive(eyeGreen);
        slime.markExclusive(eyeBrown);
        slime.markExclusive(eyePurple);
        slime.markExclusive(eyeOrange);
        slime.markExclusive(eyeRed);
        slime.markExclusive(eyeBlack);
    attributeSets.push_back(slime);
    } // Rare set.
    
}

void graphics_init() {
    temp = new char[temp_len];
    
    // Init attribute sets before blob.
    initAttributeSets();
}

void graphics_task() {
    static bool hadCompanion = false;
    static bool wasLoaded = false;
    pax_background(&buf, 0);
    
    // Draw the AVATAARRRRR.
    if (hasCompanion && companionAgrees) {
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
    
    if (currentScreen == Screen::COMP_AWAIT) {
        // Asking companion: Show cancel option.
        pax_draw_text(&buf, -1, pax_font_saira_regular, 18, 5, 5, "🅱 Cancel");
        // Also show a little overlay.
        snprintf(temp, temp_len, "Asking %s...", companion->getNick());
        const pax_font_t *font = pax_font_saira_condensed;
        pax_draw_rect(&buf, 0x7f000000, 0, (buf.height-font->default_size)/2, buf.width, font->default_size);
        pax_center_text(&buf, 0xffffffff, font, font->default_size, buf.width/2, (buf.height-font->default_size)/2, temp);
    } else if (currentScreen == Screen::HOME) {
        if (hasCompanion) {
            // Have companion; show leave option.
            pax_draw_text(&buf, -1, pax_font_saira_regular, 18, 5, 5, "🅱 Leave");
        } else if (!nearby) {
            // Nobody detected.
            pax_draw_text(&buf, -1, pax_font_saira_regular, 18, 5, 5, "Nobody nearby");
        } else if (nearby == 1) {
            // 1 person detected.
            pax_draw_text(&buf, -1, pax_font_saira_regular, 18, 5, 5, "🅴 1 person nearby");
        } else {
            // Multiple people detected.
            snprintf(temp, temp_len, "🅴 %d people nearby", nearby);
            pax_draw_text(&buf, -1, pax_font_saira_regular, 18, 5, 5, temp);
        }
    } else if (currentScreen == COMP_SELECT) {
        // Draw a kinda crappy selection list.
        pax_draw_text(&buf, -1, pax_font_saira_regular, 18, 5, 5, "↑↓Navigate 🅰Select 🅱Cancel");
        for (int i = 0; i < companionList.size(); i++) {
            pax_col_t col = i == companionListIndex ? 0xffffffff : 0xff9f9f9f;
            const char *str = connections[companionList[i]]->player->getNick();
            pax_draw_text(&buf, col, pax_font_saira_regular, 18, 8, 25+18*i, str);
        }
        pax_draw_rect(&buf, 0xffffffff, 0, 31+18*companionListIndex, 6, 6);
    }
    
    // Debug menu.
    if (currentScreen == Screen::HOME) {
        for (int i = 0; i < attributeSets.size(); i++) {
            pax_col_t col = localPlayer->blob->findSet(attributeSets[i]) != -1
                        ? 0xff00ff00
                        : 0xff9f9f9f;
            pax_draw_text(&buf, col, pax_font_saira_regular, 18, 8, 25+18*i, attributeSets[i].name);
        }
        pax_draw_rect(&buf, 0xffffffff, 0, 31+18*debugAttrIndex, 6, 6);
    }
    
    disp_flush();
}



static int lastSetId = 0;

Attribute::Attribute() {}

// Constructor for the handy.
Attribute::Attribute(Affects affects, Mode mode, float value, float range, float power) {
    this->affects = affects;
    this->mode    = mode;
    this->value   = value;
    this->range   = range;
    this->power   = power;
}

// Get an attribute set by id.
AttributeSet *getSetById(const char *id) {
    // Iterate attribute sets.
    for (auto iter = attributeSets.begin(); iter != attributeSets.end(); iter ++) {
        if (!strcmp((*iter).netId, id)) {
            return &*iter;
        }
    }
    
    // Not found.
    return NULL;
}

// Empty nameless set.
AttributeSet::AttributeSet() {
    this->id    = lastSetId++;
    this->name  = "";
    this->netId = "";
}

// Empty named set.
AttributeSet::AttributeSet(const char *netId, const char *name, Rarity Rarity) {
    this->id    = lastSetId++;
    this->netId = netId;
    this->name  = name;
    this->id    = id;
    
    switch (Rarity) {
        case COMMON:
            this->mutationWeight = 10;
            this->initialWeight  = 50;
            break;
            
        case UNCOMMON:
            this->mutationWeight = 5;
            this->initialWeight  = 10;
            break;
            
        case RARE:
            this->mutationWeight = 3;
            this->initialWeight  = 1;
            break;
            
        case VERY_RARE:
            this->mutationWeight = 2;
            this->initialWeight  = 0;
            break;
            
        case LEGENDARY:
            this->mutationWeight = 1;
            this->initialWeight  = 0;
            break;
            
        default:
        case UNOBTAINABLE:
            this->mutationWeight = 0;
            this->initialWeight  = 0;
            break;
    }
}

// Deconstructor.
AttributeSet::~AttributeSet() {
    
}

// Get the unique ID of the set.
uint32_t AttributeSet::getId() {
    return id;
}

// Add an attribute to the set.
void AttributeSet::add(Attribute toAdd) {
    attributes.push_back(toAdd);
}

// Add an attribute to the set.
void AttributeSet::add(Attribute::Affects affects, Attribute::Mode mode, float value, float range, float power) {
    attributes.push_back(Attribute(affects, mode, value, range, power));
}

// Test whether a set is mutually exclusive with this one.
bool AttributeSet::isExclusive(AttributeSet &other) {
    return exclusive.find(other.id) != exclusive.end();
}

// Make thie set mutually exclusive with another set.
void AttributeSet::markExclusive(AttributeSet &other) {
    exclusive.insert(other.id);
    other.exclusive.insert(this->id);
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

// Tests whether this blob has a given attribute set.
// If so, returns the index.
// If not, returns -1.
int Blob::findSet(AttributeSet &set) {
    for (int i = 0; i < attributes.size(); i++) {
        if (attributes[i].getId() == set.getId()) {
            return i;
        }
    }
    return -1;
}

// Toggle an attribute set.
void Blob::toggleSet(AttributeSet &set) {
    int i = findSet(set);
    if (i != -1) {
        attributes.erase(attributes.begin() + i);
    } else {
        attributes.push_back(set);
    }
    
    for (int i = 0; i < set.attributes.size(); i++) {
        changes.emplace(set.attributes[i].affects);
    }
}

// Add an attribute set.
void Blob::addSet(AttributeSet &set) {
    int i = findSet(set);
    if (i == -1) {
        attributes.push_back(set);
    }
    
    for (int i = 0; i < set.attributes.size(); i++) {
        changes.emplace(set.attributes[i].affects);
    }
}

// Remove an attribute set.
void Blob::removeSet(AttributeSet &set) {
    int i = findSet(set);
    if (i != -1) {
        attributes.erase(attributes.begin() + i);
    }
    
    for (int i = 0; i < set.attributes.size(); i++) {
        changes.emplace(set.attributes[i].affects);
    }
}

// Apply the blob's attributes from scratch.
void Blob::redoAttributes() {
    for (int i = 0; i < Attribute::Affects::NUMBER; i++) {
        changes.emplace((Attribute::Affects) i);
    }
    applyAttributes();
}

// Apply the blob's attributes, but only for changed affected variables.
void Blob::applyAttributes() {
    int hue, sat, bri;
    
    // Body shape.
    if (hasChanged(Attribute::BODY_SHAPE))
        body = (Shape) calculateAttribute(Attribute::BODY_SHAPE, Shape::SQUARE, Shape::CIRCLE);
    
    // Body color.
    hue = calculateAttribute(Attribute::BODY_HUE, 0, 255);
    if (hasChanged(Attribute::BODY_HUE) || hasChanged(Attribute::BODY_SAT) || hasChanged(Attribute::BODY_BRI)) {
        sat = calculateAttribute(Attribute::BODY_SAT, 0, 255);
        bri = calculateAttribute(Attribute::BODY_BRI, 0, 255);
        bodyColor = pax_col_hsv(hue, sat, bri);
    }
    
    // Alt color.
    if (hasChanged(Attribute::BODY_HUE) || hasChanged(Attribute::ALT_SAT) || hasChanged(Attribute::ALT_BRI)) {
        sat = calculateAttribute(Attribute::ALT_SAT, 0, 255);
        bri = calculateAttribute(Attribute::ALT_BRI, 0, 255);
        altColor = pax_col_hsv(hue, sat, bri);
    }
    
    // Eye color.
    if (hasChanged(Attribute::EYE_HUE) || hasChanged(Attribute::EYE_SAT) || hasChanged(Attribute::EYE_BRI)) {
        hue = calculateAttribute(Attribute::EYE_HUE, 0, 255);
        sat = calculateAttribute(Attribute::EYE_SAT, 0, 255);
        bri = calculateAttribute(Attribute::EYE_BRI, 0, 255);
        eyeColor = pax_col_hsv(hue, sat, bri);
    }
    
    // Eye type.
    if (hasChanged(Attribute::EYE_TYPE))
        eyeType = (EyeType) calculateAttribute(Attribute::EYE_TYPE, EyeType::TRADITIONAL, EyeType::CUTOUT);
    
    // Mark all as not changed.
    changes.clear();
}

// Tests whether an attribute has changed.
bool Blob::hasChanged(Attribute::Affects affected) {
    return changes.find(affected) != changes.end();
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
    bool needDefault = true;
    for (int x = 0; x < attributes.size(); x++) {
        AttributeSet *set = &attributes[x];
        for (int y = 0; y < set->attributes.size(); y++) {
            Attribute *attr = &set->attributes[y];
            if (attr->affects == affects && attr->mode == Attribute::SET) {
                // Add it's value to the probs.
                addCurve(probabilities, min, max, attr->value, attr->range, attr->power, false);
                needDefault = false;
            }
        }
    }
    
    // Defaults pass.
    if (needDefault) {
        for (int i = 0; i < defaultSet.attributes.size(); i++) {
            Attribute *attr = &defaultSet.attributes[i];
            if (attr->affects == affects && attr->mode == Attribute::SET) {
                // Add it's value to the probs.
                addCurve(probabilities, min, max, attr->value, attr->range, attr->power, false);
                needDefault = false;
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

// Mutate with another blob.
void Blob::mutate(Blob *with) {
    // TODO: Remove own attr.
    
    // Copy the attr. list.
    std::vector<AttributeSet> list = with->attributes;
    
    // Eliminate unknown attribute sets.
    for (auto iter = list.begin(); iter != list.end();) {
        if (!*iter->name) {
            // Empty name is considered unknown set.
            iter = list.erase(iter);
        } else {
            // Don't increment iter when erasing.
            iter ++;
        }
    }
    
    // Randomly pick a fraction of it's attributes.
    float  maxFrac = 0.4;
    size_t max = ceilf(list.size() * maxFrac);
    while (max && list.size()) {
        auto iter = list.begin() + (esp_random() / (float) UINT32_MAX * list.size());
        list.erase(iter);
        addSet(*iter);
        max --;
    }
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
    // Send attributes.
    to->sendNum("blob_attrs_count", attributes.size());
    for (int i = 0; i < attributes.size(); i++) {
        snprintf(temp, 32, "blob_attrs/%d", i);
        to->send(temp, attributes[i].netId);
    }
    
    delete temp;
}

// Receive blob data from a given connection.
void Blob::receive(Connection *from, const char *tmpTopic, const char *tmpData) {
    if (!strcmp(tmpTopic, "blob_body")) {
        // Body shape thingy.
        body = (Shape) atoi(tmpData);
        
    } else if (!strcmp(tmpTopic, "blob_body_col")) {
        // Body color thingy.
        bodyColor = atoi(tmpData);
        
    } else if (!strcmp(tmpTopic, "blob_alt_col")) {
        // Edge color thingy.
        altColor = atoi(tmpData);
        
    } else if (!strcmp(tmpTopic, "blob_looking")) {
        // Looking at direction.
        lookingAt = Pos(tmpData);
        
    } else if (!strcmp(tmpTopic, "blob_mouth")) {
        // Mouth position.
        mouth = Pos(tmpData);
        
    } else if (!strcmp(tmpTopic, "blob_mouth_bias")) {
        // Mouth bias.
        mouthBias = Pos(tmpData);
        
    } else if (!strcmp(tmpTopic, "blob_eye_col")) {
        // Eye color.
        eyeColor = atoi(tmpData);
        
    } else if (!strcmp(tmpTopic, "blob_eye_type")) {
        // Eye type.
        eyeType = (EyeType) atoi(tmpData);
        
    } else if (!strcmp(tmpTopic, "blob_eye_count")) {
        // Double check count.
        int newSize = atoi(tmpData);
        if (newSize <= 0 || newSize > 10) ESP_LOGW(TAG, "Invalid eye count %d", newSize);
        // Resize array.
        while (eyes.size() > newSize) eyes.erase(eyes.begin() + newSize);
        while (eyes.size() < newSize) eyes.push_back(Pos());
        
    } else if (!strcmp(tmpTopic, "blob_attrs_count")) {
        // Double check count.
        int newSize = atoi(tmpData);
        if (newSize <= 0 || newSize > 20) ESP_LOGW(TAG, "Invalid attribute count %d", newSize);
        // Resize array.
        while (attributes.size() > newSize) attributes.erase(attributes.begin() + newSize);
        while (attributes.size() < newSize) attributes.push_back(AttributeSet());
        
    } else if (!strncmp(tmpTopic, "blob_attrs/", 11)) {
        // Decode ID.
        AttributeSet *set = getSetById(tmpData);
        if (set) {
            // Decode index.
            int index = atoi(tmpTopic + 11);
            if (index < 0 || index >= attributes.size()) {
                // Enforce index constraints.
                attributes[index] = *set;
                ESP_LOGI(TAG, "Recv set %s", set->name, attributes.size());
            }
        } else {
            // Ignore it if it's unknown.
            ESP_LOGW(TAG, "Unknown Set '%s'", tmpData);
        }
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
