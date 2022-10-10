#ifndef IO_H
#define IO_H

BYTE insb(WORD port);
WORD insw(WORD port);

VOID outsb(WORD port, BYTE val);
VOID outsw(WORD port, WORD val);

#endif