#ifndef MEM_H
#define MEM_H	1

void *malloc(unsigned nbytes);
void *calloc(unsigned n, unsigned nbytes);
void free(void *ap);
void bfree(void *ap, unsigned n);

#endif /* MEM_H */
