
#include "graphics.h"
#include "player.h"



Blob fancy;

void graphics_task() {
    pax_background(&buf, 0);
    
    // Show number of nearby people.
    char *tmp = new char[128];
    snprintf(tmp, 128, "%d people nearby", nearby);
    pax_draw_text(&buf, -1, pax_font_saira_regular, 18, 5, 5, tmp);
    delete tmp;
    
    // Draw the AVATAARRRRR.
    fancy.draw(160, 120, 50);
    
    disp_flush();
}



Blob::Blob() {
    body      = Shape::SQUARE;
    bodyColor = 0xffd07000;
    altColor  = 0x40d07000;
    eyeColor  = 0xffd07000;
    eyeType   = EyeType::TRADITIONAL;
    eyes.push_back(Pos(-0.4, -0.5));
    eyes.push_back(Pos( 0.4, -0.5));
    mouth     = Pos(0.0, 0.4);
    mouthBias = 0;
}

void Blob::draw(float x, float y, float scale) {
    float edge = 0.2;
    static uint64_t nextRandom = 0;
    uint64_t now = esp_timer_get_time() / 1000;
    if (nextRandom <= now) {
        nextRandom      = now + 3000;
        float a         = esp_random() / (float) INT32_MAX * M_PI;
        lookingAt.x0    = lookingAt.x;
        lookingAt.y0    = lookingAt.y;
        lookingAt.x1    = sinf(a);
        lookingAt.y1    = cosf(a);
        lookingAt.start = now;
        lookingAt.end   = now + 500;
        targetBias      = (int) esp_random() / (float) INT32_MAX;
    }
    
    // Apply position and scale.
    pax_push_2d(&buf);
    pax_apply_2d(&buf, matrix_2d_translate(x, y));
    pax_apply_2d(&buf, matrix_2d_scale(scale, scale));
    
    // Square body for now.
    pax_col_t altColor = pax_col_merge(bodyColor, 0x40404040);
    pax_draw_rect(&buf, altColor,  -1, -1, 2, 2);
    pax_draw_rect(&buf, bodyColor, edge-1, edge-1, 2-2*edge, 2-2*edge);
    
    
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
    mouthBias += (targetBias - mouthBias) * 0.3;
    lookingAt.timeAnimate(now);
    
    
    // Position of mouth.
    float mouthScaleY = 0.10;
    float mouthScaleX = mouthScaleY * (1 + 0.5 * fabsf(mouthBias));
    pax_apply_2d(&buf, matrix_2d_translate(mouth.x, mouth.y));
    pax_apply_2d(&buf, matrix_2d_scale(mouthScaleX, mouthScaleY));
    
    // Draw mouth, but keep it convex.
    pax_vec1_t mouth[16];
    pax_vectorise_circle(mouth, 16, 0, 0, 1);
    // Nudge the edges.
    for (int i = 0; i < 16; i++) {
        float effect = (1 - fabsf(mouth[i].y));
        mouth[i].y -= effect * mouthBias;
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

float Blob::getEdgeX(float y, float angle) {
    return 1;
}



Blob::Pos::Pos() {
    x = y = 0;
}

Blob::Pos::Pos(float sx, float sy) {
    x  = sx; y  = sy;
    x0 = sx; y0 = sy;
    x1 = sx; y1 = sy;
}

void Blob::Pos::coeffAnimate(float newCoeff) {
    // -2x³+3x²
    float coeff = -2*newCoeff*newCoeff*newCoeff + 3*newCoeff*newCoeff;
    x = x0 + (x1 - x0) * coeff;
    y = y0 + (y1 - y0) * coeff;
}

void Blob::Pos::timeAnimate(uint64_t now) {
    float coeff;
    if (now <= start) coeff = 0;
    else if (now >= end) coeff = 1;
    else coeff = (float) (now - start) / (float) (end - start);
    coeffAnimate(coeff);
}



void Blob::Pos::drawAsArm(Blob *blob) {
    
}

void Blob::Pos::drawAsEye(Blob *blob) {
    float scale = 0.2;
    float lookX = blob->lookingAt.x;
    float lookY = blob->lookingAt.y;
    
    // Apply position and scale.
    pax_push_2d(&buf);
    pax_apply_2d(&buf, matrix_2d_translate(x, y));
    pax_apply_2d(&buf, matrix_2d_scale(scale, scale));
    
    switch (blob->eyeType) {
        default:
        case(EyeType::TRADITIONAL):
            // More traditional eye.
            pax_draw_circle(&buf, 0xffffffff, 0, 0, 1);
            pax_draw_circle(&buf, blob->eyeColor, lookX*0.2, lookY*0.2, 0.7);
            break;
        case(EyeType::CUTOUT):
            // Alt colored eye.
            pax_draw_circle(&buf, blob->eyeColor, lookX*0.3, lookY*0.3, 1);
            break;
    }
    
    // Restore transformation.
    pax_pop_2d(&buf);
}
