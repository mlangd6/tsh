#ifndef MINUNIT_H
#define MINUNIT_H
/* Code taken from http://www.jera.com/techinfo/jtns/jtn002.html */
#define mu_assert(message, test) do {  if (!(test)) return message; } while(0)
#define m_run(test) do { char *message = test(); tests_run++; if (message) return message; } while(0)
extern int tests_run;

#endif
