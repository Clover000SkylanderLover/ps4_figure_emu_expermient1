#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "portal_protocol.h"

extern bool g_MenuVisible;
extern int g_SelectedIndex;
extern int g_TotalFiles;
extern char g_FileList[32][128];
void RefreshFileList(void);
void RenderOverlay(uint32_t *framebuffer, int pitch, int width, int height);

#define PAD_L3     0x00000002
#define PAD_R3     0x00000004
#define PAD_UP     0x00000010
#define PAD_DOWN   0x00000040
#define PAD_CROSS  0x00004000
#define PAD_CIRCLE 0x00002000

typedef struct {
    uint32_t buttons;
    uint8_t  lx, ly, rx, ry;
    uint8_t  padding[16];
} ScePadData;

int (*orig_scePadReadState)(int handle, ScePadData *data);
int (*orig_sceUsbdInterruptTransfer)(int handle, uint8_t endpoint, void *data, uint32_t length, uint32_t *transferred, uint32_t timeout);

int hooked_scePadReadState(int handle, ScePadData *data) {
    int ret = orig_scePadReadState(handle, data);
    if (ret < 0 || !data) return ret;

    static bool combo_held = false;
    if ((data->buttons & PAD_L3) && (data->buttons & PAD_R3)) {
        if (!combo_held) {
            g_MenuVisible = !g_MenuVisible;
            if (g_MenuVisible) RefreshFileList();
            combo_held = true;
        }
    } else {
        combo_held = false;
    }

    if (g_MenuVisible) {
        static uint32_t last_buttons = 0;
        uint32_t pressed = data->buttons & ~last_buttons;
        last_buttons = data->buttons;

        if ((pressed & PAD_UP) && g_SelectedIndex > 0) g_SelectedIndex--;
        if ((pressed & PAD_DOWN) && g_SelectedIndex < g_TotalFiles - 1) g_SelectedIndex++;

        if (pressed & PAD_CROSS && g_TotalFiles > 0) {
            char path[256];
            snprintf(path, sizeof(path), "/data/figures/%s", g_FileList[g_SelectedIndex]);
            Portal_LoadFigure(path);
        }

        if (pressed & PAD_CIRCLE) {
            Portal_RemoveFigure();
        }

        data->buttons = 0;
        data->lx = 128; data->ly = 128;
        data->rx = 128; data->ry = 128;
    }

    return ret;
}

int hooked_sceUsbdInterruptTransfer(int handle, uint8_t endpoint, void *data, uint32_t length, uint32_t *transferred, uint32_t timeout) {
    if (endpoint & 0x80) {
        Portal_GetInReport((uint8_t *)data, length);
        if (transferred) *transferred = length;
        return 0;
    }
    
    if (!(endpoint & 0x80)) {
        Portal_ProcessOutCommand((const uint8_t *)data, length);
        if (transferred) *transferred = length;
        return 0;
    }

    return orig_sceUsbdInterruptTransfer(handle, endpoint, data, length, transferred, timeout);
}

int module_start(size_t args, const void *argp) {
    Portal_Init();
    return 0;
}

int module_stop(size_t args, const void *argp) {
    return 0;
}
