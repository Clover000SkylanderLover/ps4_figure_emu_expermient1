#include "portal_protocol.h"
#include <stdio.h>
#include <string.h>

PortalState g_Portal = {0};

static uint8_t query_block_index = 0;
static bool query_pending = false;

void Portal_Init(void) {
    memset(&g_Portal, 0, sizeof(PortalState));
}

void Portal_LoadFigure(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return;
    
    fread(g_Portal.data, 1, 1024, f);
    fclose(f);

    memcpy(g_Portal.uid, &g_Portal.data[0], 4);
    g_Portal.figure_present = true;
}

void Portal_RemoveFigure(void) {
    g_Portal.figure_present = false;
    memset(g_Portal.uid, 0, 4);
}

void Portal_ProcessOutCommand(const uint8_t *cmd, uint32_t len) {
    if (len < 1) return;

    switch (cmd[0]) {
        case 'R':
        case 'A':
        case 'M':
            break;
            
        case 'Q':
            if (len >= 3) {
                query_block_index = cmd[2];
                query_pending = true;
            }
            break;

        case 'W':
            if (len >= 19 && g_Portal.figure_present) {
                uint8_t block = cmd[2];
                if (block < 64) {
                    memcpy(&g_Portal.data[block * 16], &cmd[3], 16);
                }
            }
            break;
    }
}

void Portal_GetInReport(uint8_t *report, uint32_t len) {
    memset(report, 0, len);

    if (query_pending && g_Portal.figure_present) {
        report[0] = 'Q';
        report[1] = 0x00;
        report[2] = query_block_index;
        
        if (query_block_index < 64) {
            memcpy(&report[3], &g_Portal.data[query_block_index * 16], 16);
        }
        query_pending = false;
        return;
    }

    report[0] = 'S';
    if (g_Portal.figure_present) {
        report[1] = 0x01;
        report[2] = 0x00;
        report[3] = 0x00;
        memcpy(&report[4], g_Portal.uid, 4);
    } else {
        report[1] = 0x00;
    }
}
