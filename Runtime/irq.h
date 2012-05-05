#ifndef __IRQ_H_
#define __IRQ_H_

// Note: Uses optimize("O2") to work around this bug:
// http://gcc.gnu.org/bugzilla/show_bug.cgi?id=39585
#define __irq__ __attribute__((interrupt("IRQ"), optimize("O2")))

#endif