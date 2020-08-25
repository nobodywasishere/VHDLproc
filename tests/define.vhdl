#define TEST "hello"

TEST


#define TEST2

#ifdef TEST2

TEST TEST TEST

#else

TEST TEST no TEST

#endif

#ifndef TEST3

TEST TEST TEST

#endif
