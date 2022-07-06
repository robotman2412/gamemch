
#pragma once
#include "main.h"


void grpahics_task() {
    pax_background(&buf, 0);
    
    // Show number of nearby people.
    char *tmp = new char[128];
    snprintf(tmp, 128, "%d people nearby", nearby);
    pax_draw_text(&buf, -1, pax_font_saira_regular, 18, 5, 5, tmp);
    delete tmp;
    
    disp_flush();
}
