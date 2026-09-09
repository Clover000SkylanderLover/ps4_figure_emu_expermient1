#ifndef PORTAL_PROTOCOL_H
#define PORTAL_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    bool figure_present;
    uint8_t uid[4];
    uint8_t data[1024];
} PortalState;

extern PortalState g_Portal;

void Portal_Init(void);
void Portal_LoadFigure(const char *path);
void Portal_RemoveFigure(void);
void Portal_ProcessOutCommand(const uint8_t *cmd, uint32_t len);
void Portal_GetInReport(uint8_t *report, uint32_t len);

#endif
