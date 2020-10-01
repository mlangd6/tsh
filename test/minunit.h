#ifndef MINUNIT_H
#define MINUNIT_H

#define mu_assert(message, test) do {  if (!(test)) return message; } while(0)
#define m_run(test) do { char *message = test(); test_run++; if (message) return message; } while(0)
extern int tes_run;

#endif
