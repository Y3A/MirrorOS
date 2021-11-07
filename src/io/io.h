#ifndef IO_H
#define IO_H

unsigned char insb(unsigned short port);
unsigned short insw(unsigned short port);

void outsb(unsigned short port, unsigned char val);
void outsw(unsigned short port, unsigned short val);

#endif