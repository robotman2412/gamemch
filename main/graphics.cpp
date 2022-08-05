
#include "graphics.h"
#include "connection.h"
#include "player.h"
#include "string.h"



static const char *TAG = "graphics";

AttributeSet defaultSet ("defaultSet",  "",            Rarity::UNOBTAINABLE);

/* ==== eyes ==== */
AttributeSet eyeBlue    ("eyeBlue",     "Blue Eyes",   Rarity::UNCOMMON);
AttributeSet eyeGreen   ("eyeGreen",    "Green Eyes",  Rarity::COMMON);
AttributeSet eyeBrown   ("eyeBrown",    "Brown Eyes",  Rarity::COMMON);
AttributeSet eyePurple  ("eyePurple",   "Purple Eyes", Rarity::RARE);
AttributeSet eyeOrange  ("eyeOrange",   "Orange Eyes", Rarity::RARE);
AttributeSet eyeRed     ("eyeRed",      "Red Eyes",    Rarity::VERY_RARE);
AttributeSet eyeBlack   ("eyeBlack",    "Black Eyes",  Rarity::VERY_RARE);
AttributeSet eyeOne     ("eyeOne",      "Cyclops",     Rarity::RARE);
AttributeSet eyeThree   ("eyeThree",    "Three Eyes",  Rarity::RARE);

/* ==== body ==== */
AttributeSet darkSide   ("darkSide",    "Dark Side",   Rarity::RARE);
AttributeSet slime      ("slime",       "Slime",       Rarity::RARE);
AttributeSet bodyRed    ("bodyRed",     "Red",         Rarity::UNCOMMON);
AttributeSet bodyYellow ("bodyYellow",  "Yellow",      Rarity::COMMON);
AttributeSet pointy     ("pointy",      "Pointy",      Rarity::UNCOMMON);
AttributeSet smooth     ("smooth",      "Smooth",      Rarity::UNCOMMON);

/* ==== resources ==== */
extern uint8_t pattern0_start[] asm("_binary_pattern0_png_start");
extern uint8_t pattern0_end[]   asm("_binary_pattern0_png_end");
pax_buf_t pattern0;

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
        // Mark exlusive with other colors.
        eyeBlue.markExclusive(eyeGreen);
        eyeBlue.markExclusive(eyeBrown);
        eyeBlue.markExclusive(eyePurple);
        eyeBlue.markExclusive(eyeOrange);
        eyeBlue.markExclusive(eyeRed);
        eyeBlue.markExclusive(eyeBlack);
    } // Uncommon set.

    { // Brown eyes.
        eyeBrown.add(Attribute::EYE_HUE, Attribute::SET, 28, 5, 5);
        eyeBrown.add(Attribute::EYE_BRI, Attribute::SET, 90, 10, 5);
        // Mark exlusive with other colors.
        eyeBrown.markExclusive(eyeBlue);
        eyeBrown.markExclusive(eyeGreen);
        eyeBrown.markExclusive(eyePurple);
        eyeBrown.markExclusive(eyeOrange);
        eyeBrown.markExclusive(eyeRed);
        eyeBrown.markExclusive(eyeBlack);
    } // Common set.

    { // Green eyes.
        // Blue to cyan hue.
        eyeGreen.add(Attribute::EYE_HUE, Attribute::SET, 75, 5, 5);
        // Mark exlusive with other colors.
        eyeGreen.markExclusive(eyeBlue);
        eyeGreen.markExclusive(eyeBrown);
        eyeGreen.markExclusive(eyePurple);
        eyeGreen.markExclusive(eyeOrange);
        eyeGreen.markExclusive(eyeRed);
        eyeGreen.markExclusive(eyeBlack);
    } // Uncommon set.

    { // Purple eyes.
        eyePurple.add(Attribute::EYE_HUE, Attribute::SET, 194, 5, 5);
        // Mark exlusive with other colors.
        eyePurple.markExclusive(eyeBlue);
        eyePurple.markExclusive(eyeGreen);
        eyePurple.markExclusive(eyeBrown);
        eyePurple.markExclusive(eyeOrange);
        eyePurple.markExclusive(eyeRed);
        eyePurple.markExclusive(eyeBlack);
    } // Very rare set.

    { // Orange eyes.
        eyeOrange.add(Attribute::EYE_HUE, Attribute::SET, 28, 5, 5);
        eyeOrange.add(Attribute::EYE_BRI, Attribute::SET, 255, 5, 5);
        eyeOrange.add(Attribute::EYE_SAT, Attribute::SET, 210, 5, 5);
        // Mark exlusive with other colors.
        eyeOrange.markExclusive(eyeBlue);
        eyeOrange.markExclusive(eyeGreen);
        eyeOrange.markExclusive(eyeBrown);
        eyeOrange.markExclusive(eyePurple);
        eyeOrange.markExclusive(eyeRed);
        eyeOrange.markExclusive(eyeBlack);
    } // Very rare set.

    { // Red eyes.
        eyeRed.add(Attribute::EYE_HUE, Attribute::SET, 0, 5, 5);
        // Mark exlusive with other colors.
        eyeRed.markExclusive(eyeBlue);
        eyeRed.markExclusive(eyeGreen);
        eyeRed.markExclusive(eyeBrown);
        eyeRed.markExclusive(eyePurple);
        eyeRed.markExclusive(eyeOrange);
        eyeRed.markExclusive(eyeBlack);
    } // Legendary set.

    { // Black eyes.
        eyeBlack.add(Attribute::EYE_BRI, Attribute::SET, 10, 5, 5);
        eyeBlack.add(Attribute::EYE_SAT, Attribute::SET, 10, 5, 5);
        // Mark exlusive with other colors.
        eyeBlack.markExclusive(eyeBlue);
        eyeBlack.markExclusive(eyeGreen);
        eyeBlack.markExclusive(eyeBrown);
        eyeBlack.markExclusive(eyePurple);
        eyeBlack.markExclusive(eyeOrange);
        eyeBlack.markExclusive(eyeRed);
    } // Legendary set.

    { // One eye.
        eyeOne.add(Attribute::EYE_COUNT, Attribute::SET, 1, 1, 1);
        eyeOne.markExclusive(eyeThree);
    } // Very rare set.

    { // Three eyes.
        eyeThree.add(Attribute::EYE_COUNT, Attribute::SET, 3, 1, 1);
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
        // Incompatible with black eyes.
        darkSide.markExclusive(eyeBlack);
    } // Rare set.

    { // Red body.
        // Red body.
        bodyRed.add(Attribute::BODY_HUE,  Attribute::SET, 3, 10, 5);
        // Incompatible with other body colors.
        bodyRed.markExclusive(bodyYellow);
        bodyRed.markExclusive(slime);
    } // Uncommon set.

    { // Yellow body.
        // Red body.
        bodyYellow.add(Attribute::BODY_HUE,  Attribute::SET, 41, 1, 5);
        // Incompatible with other body colors.
        bodyYellow.markExclusive(bodyRed);
        bodyYellow.markExclusive(slime);
    } // Common set.

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
        // Incompatible with other body colors.
        slime.markExclusive(bodyYellow);
        slime.markExclusive(bodyRed);
    } // Rare set.

    { // Pointy.
        // More square than not.
        pointy.add(Attribute::BODY_SHAPE, Attribute::SET, Shape::SQUARE, 2, 2);
        // Incompatible with smooth.
        pointy.markExclusive(smooth);
    } // Uncommon set.

    { // Smooth.
        // More round than not.
        smooth.add(Attribute::BODY_SHAPE, Attribute::SET, Shape::CIRCLE, 2, 2);
    } // Uncommon set.
    
    
    // I am very dumb.
    attributeSets.push_back(eyeBlue);
    attributeSets.push_back(eyeBrown);
    attributeSets.push_back(eyeGreen);
    attributeSets.push_back(eyePurple);
    attributeSets.push_back(eyeOrange);
    attributeSets.push_back(eyeRed);
    attributeSets.push_back(eyeBlack);
    attributeSets.push_back(eyeOne);
    attributeSets.push_back(eyeThree);
    attributeSets.push_back(darkSide);
    attributeSets.push_back(bodyRed);
    attributeSets.push_back(bodyYellow);
    attributeSets.push_back(slime);
    attributeSets.push_back(smooth);
    attributeSets.push_back(pointy);
}

static const pax_vec1_t square_points[4] = {
    {1,1}, {1,-1}, {-1,-1}, {-1,1},
};

static pax_vec1_t pentagon_points[5];
static pax_vec1_t hexagon_points[6];
static pax_vec1_t circle_points[32];

static void vectorise_ngon(pax_vec1_t *out, int n_points) {
    pax_vectorise_arc(
        out, n_points, 0, 0, 1,
        M_PI * -0.5 + M_PI / n_points,
        M_PI * -0.5 + M_PI / n_points + M_PI * 2
    );
}

static void precalculate() {
    // The body shapes.
    vectorise_ngon(pentagon_points, 5);
    vectorise_ngon(hexagon_points, 6);
    vectorise_ngon(circle_points, 32);
}

void graphics_init() {
    // Allocate the text temporary buffer.
    temp = new char[temp_len];

    // Precaculate various things.
    precalculate();

    // Init attribute sets before blob.
    initAttributeSets();
    
    // Load resources.
    if (!pax_decode_png_buf(&pattern0, pattern0_start, pattern0_end-pattern0_start, PAX_BUF_16_4444ARGB, CODEC_FLAG_OPTIMAL)) {
        ESP_LOGE(TAG, "Failed to load pattern0.png");
        exit_to_launcher();
    }
}

void graphics_task() {
    static bool hadCompanion = false;
    pax_background(&buf, 0);
    
    if (currentScreen == Screen::MUTATE_PICK) {
        // Draw the mutation pick screen thing.
        for (auto iter = candidates.begin(); iter != candidates.end(); iter++) {
            iter->draw("");
        }
    }
    
    // Draw the AVATAARRRRR.
    if (hasCompanion && companionAgrees) {
        if (!hadCompanion) {
            localPlayer->blob->pos.animateTo(buf.width/3, buf.height/2, 35, 1000);
            companion->blob->pos = Blob::Pos(buf.width*2/3, buf.height/2, 0);
            companion->blob->pos.animateTo(buf.width*2/3, buf.height/2, 35, 1000);
        }
        
        localPlayer->blob->draw(localPlayer->getNick());
        companion->blob->draw(companion->getNick());
        hadCompanion = true;
    } else {
        if (hadCompanion) {
            localPlayer->blob->pos.animateTo(buf.width/2, buf.height/2, 50, 1000);
        }
        localPlayer->blob->draw(localPlayer->getNick());
        hadCompanion = false;
    }
    
    if (currentScreen == Screen::INTRO) {
        pax_draw_text(&buf, -1, pax_font_saira_regular, 18, 5, 5, "↑↓Read 🆂Done");
        
        pax_clip(&buf, 0, 25, 320, 240-25);
        float scrollFac = pax_text_size(pax_font_saira_regular, 18, introText).y - 240 + 55;
        scrollFac *= introScroll;
        
        pax_center_text(&buf, -1, pax_font_saira_condensed, 30, 320*0.5, 25-scrollFac, introTitle);
        pax_draw_text(&buf, -1, pax_font_saira_regular, 18, 5, 55-scrollFac, introText);
        
        pax_noclip(&buf);
    } else if (currentScreen == Screen::COMP_AWAIT) {
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
            pax_draw_text(&buf, -1, pax_font_saira_regular, 18, 5, 5, "🆂Cross 🅱Leave");
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
    } else if (currentScreen == Screen::COMP_SELECT) {
        // Draw a kinda crappy selection list.
        pax_draw_text(&buf, -1, pax_font_saira_regular, 18, 5, 5, "↑↓Navigate 🅰Select 🅱Cancel");
        for (int i = 0; i < companionList.size(); i++) {
            pax_col_t col = i == companionListIndex ? 0xffffffff : 0xff9f9f9f;
            const char *str = connections[companionList[i]]->player->getNick();
            pax_draw_text(&buf, col, pax_font_saira_regular, 18, 28, 25+18*i, str);
            col = connections[companionList[i]]->player->blob->bodyColor;
            pax_draw_circle(&buf, col, 10, 34+18*i, i==companionListIndex ? 9 : 3);
        }
        // pax_draw_rect(&buf, 0xffffffff, 0, 31+18*companionListIndex, 6, 6);
    } else if (currentScreen == Screen::MUTATE_PICK) {
        pax_draw_text(&buf, -1, pax_font_saira_regular, 18, 5, 5, "←→Select 🅰Accept");
    } else if (currentScreen == Screen::MUTATE_AWAIT) {
        snprintf(temp, temp_len, "Waiting for %s...", companion->getNick());
        pax_draw_text(&buf, -1, pax_font_saira_regular, 18, 5, 5, temp);
    }
    
    // Draw nearby players.
    if (currentScreen != Screen::COMP_SELECT && currentScreen != Screen::INTRO) {
        float x = 10;
        for (auto iter = connections.begin(); iter != connections.end(); iter++) {
            Connection *conn = *iter;
            if (conn && conn->player && conn != broadcaster && conn->status == Connection::OPEN && (conn->player != companion || !companionAgrees)) {
                pax_draw_circle(&buf, conn->player->blob->bodyColor, x, 30, 9);
                x += 20;
            }
        }
    }
    
    #ifdef ENABLE_DEBUG
    // Debug menu.
    if (currentScreen == Screen::HOME) {
        for (int i = 0; i < attributeSets.size(); i++) {
            pax_col_t col = localPlayer->blob->findSet(attributeSets[i]) != -1
                        ? 0xff00ff00
                        : 0xff9f9f9f;
            pax_draw_text(&buf, col, pax_font_sky, 9, 8, 25+9*i, attributeSets[i].name);
        }
        pax_draw_rect(&buf, 0xffffffff, 0, 28+9*debugAttrIndex, 4, 4);
    }
    #endif
    
    // Draw version warnings.
    #ifdef ENABLE_DEBUG
    if (hasCompanion && (companion->version != GAME_VERSION || !companion->versionDebug)) {
        bool compNewer = companion->version > GAME_VERSION || (companion->version == GAME_VERSION && !companion->versionDebug);
    #else
    if (hasCompanion && (companion->version != GAME_VERSION || companion->versionDebug)) {
        bool compNewer = companion->version > GAME_VERSION;
    #endif
        if (compNewer) {
            snprintf(temp, temp_len, "%s has newer version %s\n(you are on %s)", companion->getNick(), companion->verStr, localPlayer->verStr);
        } else {
            snprintf(temp, temp_len, "%s has older version %s\n(you are on %s)", companion->getNick(), companion->verStr, localPlayer->verStr);
        }
        pax_vec1_t dims = pax_text_size(pax_font_sky, 9, temp);
        pax_draw_text(&buf, 0xffff7f00, pax_font_sky, 9, 320-5-dims.x, 240-5-dims.y, temp);
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
            this->mutationWeight = 1;
            this->initialWeight  = 50;
            break;

        case UNCOMMON:
            this->mutationWeight = 1;
            this->initialWeight  = 10;
            break;

        case RARE:
            this->mutationWeight = 5;
            this->initialWeight  = 5;
            break;

        case VERY_RARE:
            this->mutationWeight = 5;
            this->initialWeight  = 1;
            break;

        case LEGENDARY:
            this->mutationWeight = 3;
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
    for (auto iter = exclusive.begin(); iter != exclusive.end(); iter++) {
        printf(" %d", *iter);
        if (*iter == other.id) {
            return true;
        }
    }
    // return exclusive.find(other.id) != exclusive.end();
    return false;
}

// Make this set mutually exclusive with another set.
void AttributeSet::markExclusive(AttributeSet &other) {
    if (other.id == id) return;
    ESP_LOGW(TAG, "Mark %s(%d) excl %s(%d)", name, id, other.name, other.id);
    exclusive.emplace(other.id);
    other.exclusive.emplace(id);
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
    sickness  = 0;
    redoAll   = false;
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
void Blob::draw(const char *name) {
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
    
    // Apply sickness.
    pax_push_2d(&buf);
    pax_apply_2d(&buf, matrix_2d_shear(0, sickness*0.3));
    pax_apply_2d(&buf, matrix_2d_scale(1+sickness*0.2, 1));
    
    
    // pax_quad_t uvs = {
    //     .x0 = 0, .y0 = 0,
    //     .x1 = 9, .y1 = 0,
    //     .x2 = 9, .y2 = 9,
    //     .x3 = 0, .y3 = 9,
    // };
    // pax_shader_t shader = PAX_SHADER_TEXTURE(&pattern0);
    switch (body) {
        default:
        case(SQUARE):
            // Square body.
            pax_draw_rect(&buf, altColor,  -1, -1, 2, 2);
            // pax_shade_rect(
            //     &buf, bodyColor,
            //     &shader, &uvs,
            //     edge-1, edge-1, 2-2*edge, 2-2*edge
            // );
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
    pax_push_2d(&buf);
    float mouthScaleY = 0.10;
    float mouthScaleX = mouthScaleY * (1 + 0.5 * fabsf(mouthBias.x));
    pax_apply_2d(&buf, matrix_2d_translate(mouth.x, mouth.y));
    pax_apply_2d(&buf, matrix_2d_scale(mouthScaleX, mouthScaleY));
    // Apply sickness.
    pax_apply_2d(&buf, matrix_2d_translate(sickness, sickness*0.2));

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
    pax_pop_2d(&buf);
    
    // Draw the name.
    pax_pop_2d(&buf);
    pax_center_text(&buf, -1, pax_font_saira_regular, 18.0/50.0, 0, -1.5, name);

    // Restore transformation.
    pax_pop_2d(&buf);
}

// Interpolates X given Y on a line.
static float linelerp(pax_vec1_t *line, float y) {
    if (line[0].y < line[1].y) {
        return line[0].x + (line[1].x-line[0].x)*(y-line[0].y)/(line[1].y-line[0].y);
    } else {
        return line[1].x + (line[0].x-line[1].x)*(y-line[1].y)/(line[0].y-line[1].y);
    }
}

// Gets X counterpart for a given Y on the edge of the blob.
float Blob::getEdgeX(float y) {
    int numToEval;
    pax_vec1_t *points;
    switch (body) {
        default:
        case(SQUARE):
            // Square body.
            return 1;
        case(PENTAGON):
            // Pentagonal body.
            numToEval = 2;
            points = pentagon_points;
            break;
        case(HEXAGON):
            // Hexagonal body.
            numToEval = 2;
            points = hexagon_points;
            break;
        case(CIRCLE):
            // Circular body.
            numToEval = 16;
            points = circle_points;
            break;
    }

    // Find the right subdivision.
    for (int i = 0; i < numToEval; i++) {
        if (y >= points[i+1].y && y <= points[i].y) {
            // And linearly interpolate the rest.
            return linelerp(points+i, y);
        }
    }
    return 0;
}


// Give initial attribute sets and apply them.
void Blob::initialRandomise() {
    // Pick a number of sets.
    int numSets = 2 + esp_random() % 2U;
    std::vector<AttributeSet> pool = attributeSets;
    
    // Continually pick a random set.
    for (; pool.size() && numSets > 0; numSets --) {
        
        // Measure the pool.
        float total = 0;
        for (auto iter = pool.begin(); iter != pool.end(); iter++) {
            total += iter->initialWeight;
        }
        
        // Pick one set from the pool.
        float coeff = esp_random() / (float) UINT32_MAX * total;
        for (auto iter = pool.begin(); iter != pool.end(); iter++) {
            coeff -= iter->initialWeight;
            if (coeff <= 0) {
                AttributeSet set = *iter;
                
                // Remove from the pool.
                pool.erase(iter);
                
                // Eliminate incompatible sets from the pool.
                for (auto y = pool.begin(); y != pool.end();) {
                    if (set.isExclusive(*y)) {
                        y = pool.erase(y);
                    } else {
                        y ++;
                    }
                }
                
                // Add it to the current sets.
                attributes.push_back(set);
                break;
            }
        }
    }
    
    // Apply all attributes.
    redoAttributes();
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
    // ESP_LOGW(TAG, "rm find %d", i);
    if (i != -1) {
        ESP_LOGW(TAG, "rm name %s", set.name);
        // ESP_LOGW(TAG, "pre len %zu", attributes.size());
        attributes.erase(attributes.begin() + i);
        // ESP_LOGW(TAG, "rm len %zu", attributes.size());
        
        for (int i = 0; i < set.attributes.size(); i++) {
            changes.emplace(set.attributes[i].affects);
            // ESP_LOGW(TAG, "+affect %d", set.attributes[i].affects);
        }
    }
}

// Apply the blob's attributes from scratch.
void Blob::redoAttributes() {
    redoAll = true;
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
    
    // Eye count.
    if (hasChanged(Attribute::EYE_COUNT)) {
        int eyeCount = calculateAttribute(Attribute::EYE_COUNT, 1, 3);
        
        eyes.clear();
        switch (eyeCount) {
            case 1:
                eyes.push_back(Pos(0, -0.5, 1.5));
                break;

            default:
            case 2:
                eyes.push_back(Pos(-0.4, -0.5));
                eyes.push_back(Pos( 0.4, -0.5));
                break;

            case 3:
                eyes.push_back(Pos(-0.45, -0.45));
                eyes.push_back(Pos( 0,    -0.55));
                eyes.push_back(Pos( 0.45, -0.45));
                break;
        }
    }
    
    // Mark all as not changed.
    changes.clear();
    redoAll = false;
}

// Tests whether an attribute has changed.
bool Blob::hasChanged(Attribute::Affects affected) {
    return redoAll || changes.find(affected) != changes.end();
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
    // Determine sickness.
    sickness = fabsf(sickness);
    int attrCommon = 0, attrLocal = 0, attrRemote = 0;
    // Determine common and local attributes.
    for (auto iter = attributes.begin(); iter != attributes.end();) {
        for (auto y = with->attributes.begin(); y != with->attributes.end(); y++) {
            if (!strcmp(iter->netId, y->netId)) {
                attrCommon ++;
                goto yeems0;
            }
        }
        attrLocal++;
        yeems0:
        iter++;
    }
    // Determine remote attributes.
    for (auto iter = with->attributes.begin(); iter != with->attributes.end();) {
        for (auto y = attributes.begin(); y != attributes.end(); y++) {
            if (!strcmp(iter->netId, y->netId)) {
                goto yeems1;
            }
        }
        attrRemote++;
        yeems1:
        iter++;
    }
    // Sickness DELTA.
    int attrDiff = attrLocal + attrRemote;
    float sicknessChance = (float) attrCommon / (float) (attrDiff + 0.3);
    if (esp_random() / (float) UINT32_MAX < sicknessChance) {
        sickness += 0.1;
        if (sickness > 1.5) sickness = 1;
    } else if (esp_random() / (float) UINT32_MAX < 0.3) {
        sickness -= 0.1;
        if (sickness < 0) sickness = 0;
    }
    if (esp_random() & 1) {
        sickness = -sickness;
    }
    
    // Pick a number to remove.
    float remFrac = 0.3 + esp_random() / (float) UINT32_MAX * 0.3;
    int   rem     = ceilf(remFrac * attributes.size());
    // Randomly remove attributes but never remove all.
    while (rem && attributes.size() > 1) {
        auto iter = attributes.begin() + (int) (esp_random() / (float) UINT32_MAX * attributes.size());
        removeSet(*iter);
        rem --;
    }
    
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
    
    // Eliminate incompatible sets from the pool.
    for (auto iter = attributes.begin(); iter != attributes.end(); iter++) {
        AttributeSet set = *iter;
        for (auto y = list.begin(); y != list.end();) {
            if (set.isExclusive(*y) || !strcmp(set.netId, y->netId)) {
                y = list.erase(y);
            } else {
                y ++;
            }
        }
    }
    
    // Randomly pick a fraction of it's attributes.
    float maxFrac = 0.2 + esp_random() / (float) UINT32_MAX * 0.2;
    int   max = ceilf(list.size() * maxFrac);
    ESP_LOGI(TAG, "Mutating against %d of %zu attributes", max, list.size());
    while (max && list.size()) {
        // Pick a random one.
        auto iter = list.begin() + (int) (esp_random() / (float) UINT32_MAX * list.size());
        AttributeSet set = *iter;
        addSet(set);
        list.erase(iter);
        
        // Eliminate incompatible sets from the pool.
        for (auto y = list.begin(); y != list.end();) {
            if (set.isExclusive(*y)) {
                y = list.erase(y);
            } else {
                y ++;
            }
        }
        max --;
    }
    
    // Chance to mutate in another new gene.
    float extraChance = fabsf(sickness);
    bool c0 = esp_random() / (float) UINT32_MAX < extraChance;
    if (attributes.size() <= 2) c0 = true;
    bool c1 = list.size() > 0;
    if (c0 && c1) {
        // Measure the total weight before weighted random.
        float total = 0;
        for (auto iter = list.begin(); iter != list.end(); iter++) {
            total += iter->mutationWeight;
        }
        // Pick a weighted random one.
        float coeff = esp_random() / (float) UINT32_MAX * total;
        for (auto iter = list.begin(); iter != list.end(); iter++) {
            coeff -= iter->mutationWeight;
            if (coeff <= 0) {
                // The result was picked, add it and finish.
                addSet(*iter);
                break;
            }
        }
    }
    
    redoAttributes();
}


// Whether this blob has the same attribute sets as another.
bool Blob::setsEquals(Blob &other) {
    if (attributes.size() != other.attributes.size()) return false;
    
    for (auto iter = attributes.begin(); iter != attributes.end();) {
        for (auto y = other.attributes.begin(); y != other.attributes.end(); y++) {
            if (!strcmp(iter->netId, y->netId)) goto yesh;
        }
        return false;
        yesh:
        iter++;
    }
    
    return true;
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
    // Send sickness.
    to->sendNum("blob_sickness", sickness*65536);

    delete temp;
}

// Receive blob data from a given connection.
void Blob::receive(Connection *from, const char *tmpTopic, const char *tmpData) {
    if (!strncmp(tmpTopic, "blob", 4)) ESP_LOGI(tmpTopic, "%s", tmpData);
        
    if (!strcmp(tmpTopic, "blob_sickness")) {
        // Sickness.
        sickness = atoi(tmpData) / (float) 65536;
        
    } else if (!strcmp(tmpTopic, "blob_body")) {
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

    } else if (!strncmp(tmpTopic, "blob_eye/", 9)) {
        // Decode index.
        int index = atoi(tmpTopic + 9);
        if (index < 0 || index >= eyes.size()) {
            ESP_LOGW(TAG, "Invalid index %d", index);
        } else {
            // Enforce index constraints.
            eyes[index] = Pos(tmpData);
        }
        
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
                ESP_LOGW(TAG, "Invalid index %d", index);
            } else {
                // Enforce index constraints.
                attributes[index] = *set;
            }
        } else {
            // Ignore it if it's unknown.
            ESP_LOGW(TAG, "Unknown Set '%s'", tmpData);
        }
    }
}

// Stores the blob to NVS.
void Blob::storeToNvs() {
    char *temp = new char[32+32*attributes.size()];
    
    // Open NVS.
    nvs_handle_t handle = 0;
    esp_err_t res = nvs_open(GAME_NVS_NAME, NVS_READWRITE, &handle);
    if (res) ESP_LOGE(TAG, "Error storing blob: %s", esp_err_to_name(res));
    
    // Save colors.
    nvs_set_u32(handle, "blob_body_col", bodyColor);
    nvs_set_u32(handle, "blob_alt_col",  altColor);
    nvs_set_u32(handle, "blob_eye_col",  eyeColor);
    // Save eyes.
    nvs_set_u32(handle, "blob_eye_type", (int) eyeType);
    nvs_set_u32(handle, "blob_eye_count", eyes.size());
    // for (int i = 0; i < eyes.size(); i++) {
    //     snprintf(temp, 32, "blob_eye/%d", i);
    //     eyes[i].send(to, temp);
    // }
    // Save body type.
    nvs_set_u32(handle, "blob_body", (int) body);
    // Save attributes.
    *temp = 0;
    for (int i = 0; i < attributes.size(); i++) {
        if (i) strcat(temp, ",");
        strcat(temp, attributes[i].netId);
    }
    nvs_set_str(handle, "blob_attrs", temp);
    // Save sickness.
    nvs_set_u32(handle, "blob_sickness", sickness*65536);
    
    // Close NVS.
    nvs_close(handle);
    
    delete temp;
}

// Loads the blob from NVS.
void Blob::loadFromNvs() {
    esp_err_t res = 0;
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
    char *pos0 = strchr(data, ',');
    char *pos1 = strchr(pos0+1, ',');
    if (pos0 && !pos1) {
        *pos0 = 0;
        float sx = atoff(data);
        float sy = atoff(pos0+1);
        x  = sx; y  = sy;
        x0 = sx; y0 = sy;
        x1 = sx; y1 = sy;
    } else if (pos0 && pos1) {
        *pos0 = 0;
        *pos1 = 0;
        float sx = atoff(data);
        float sy = atoff(pos0+1);
        float ss = atoff(pos1+1);
        scale = ss;
        s0 = ss; s1 = ss;
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
    // int64_t now      = esp_timer_get_time() / 1000;
    // int64_t startAlt = start - now;
    // int64_t endAlt   = end   - now;
    // Format and send it!
    snprintf(temp, 32, "%f,%f,%f", x1, y1, s1);
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
    float scale  = 0.2 * this->scale;
    float lookX  = blob->lookingAt.x;
    float lookY  = blob->lookingAt.y;

    // Apply position and scale.
    pax_push_2d(&buf);
    pax_apply_2d(&buf, matrix_2d_translate(x, y));
    pax_apply_2d(&buf, matrix_2d_scale(scale, scale));
    
    // Apply sickness.
    pax_apply_2d(&buf, matrix_2d_translate(y*-0.5*blob->sickness, x*0.5*blob->sickness));
    pax_apply_2d(&buf, matrix_2d_rotate(M_PI*0.25*blob->sickness));

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
