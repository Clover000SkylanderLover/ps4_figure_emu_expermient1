#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "portal_protocol.h"
#include "font.h"

#define MAX_FILES 32

bool g_MenuVisible = false;
int g_SelectedIndex = 0;
int g_TotalFiles = 0;
char g_FileList[MAX_FILES][128];

void RefreshFileList(void) {
    g_TotalFiles = 0;
    DIR *dir = opendir("/data/figures");
    if (!dir) return;

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL && g_TotalFiles < MAX_FILES) {
        if (ent->d_name[0] != '.') {
            strncpy(g_FileList[g_TotalFiles], ent->d_name, 127);
            g_TotalFiles++;
        }
    }
    closedir(dir);
}

void DrawPixel(uint32_t *fb, int pitch, int x, int y, uint32_t color) {
    fb[y * (pitch / 4) + x] = color;
}

void DrawText(uint32_t *fb, int pitch, int x, int y, const char *text, uint32_t color) {
    while (*text) {
        char c = *text++;
        if (c < 0 || c > 127) c = ' ';
        for (int row = 0; row < 8; row++) {
            uint8_t row_bits = font8x8_basic[(int)c][row];
            for (int col = 0; col < 8; col++) {
                if (row_bits & (1 << col)) {
                    DrawPixel(fb, pitch, x + col, y + row, color);
                }
            }
        }
        x += 8;
    }
}

void RenderOverlay(uint32_t *framebuffer, int pitch, int width, int height) {
    if (!g_MenuVisible || !framebuffer) return;

    for (int y = 40; y < 400; y++) {
        for (int x = 40; x < 440; x++) {
            DrawPixel(framebuffer, pitch, x, y, 0xCC111111);
        }
    }

    DrawText(framebuffer, pitch, 50, 55, "=== PS4 FIGURE EMULATOR ===", 0xFF00FFFF);
    
    char status[64];
    snprintf(status, sizeof(status), "Portal Status: %s", g_Portal.figure_present ? "[TAG MOUNTED]" : "[EMPTY]");
    DrawText(framebuffer, pitch, 50, 75, status, g_Portal.figure_present ? 0xFF00FF00 : 0xFF888888);

    for (int i = 0; i < g_TotalFiles; i++) {
        char line[140];
        uint32_t col = (i == g_SelectedIndex) ? 0xFFFFFF00 : 0xFFFFFFFF;
        snprintf(line, sizeof(line), "%s %s", (i == g_SelectedIndex) ? ">" : " ", g_FileList[i]);
        DrawText(framebuffer, pitch, 50, 105 + (i * 12), line, col);
    }

    DrawText(framebuffer, pitch, 50, 360, "[X] Place Tag | [O] Remove Tag | [L3+R3] Exit", 0xFFAAAAAA);
}
