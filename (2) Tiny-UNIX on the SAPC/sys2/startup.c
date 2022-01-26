/* C startup file, called from startup0.s-- */

extern void clr_bss(void);
extern void init_devio(void);
extern void main(void);
extern void init_kernel(void);

void startupc()
{
  clr_bss();			/* clear BSS area (uninitialized data) */
  init_devio();			/* latch onto Tutor-supplied info, code */
 // (void)main();			
 // /* execute user-supplied main */
 /* replace main with init_kernel */
  init_kernel();
}
