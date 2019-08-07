#ifndef __AD9833_H
#define __AD9833_H


void AD9833_AmpSet(unsigned char amp);
void setWave(int waveform, long frequency);

// AD9833 Control Register helpers
#define CR_B28_COMBINED      0x2000
#define CR_FSELECT_0         0x0000
#define CR_PSELECT_0         0x0000
#define CR_RESET             0x0100
#define CR_SLEEP1            0x0080
#define CR_SLEEP12           0x0040
#define CR_OPBITEN           0x0020
#define CR_DIV2              0x0008
#define CR_MODE_D1_TRIANGLE  0x0002
#define CR_MODE_D1_SINE      0x0000

// Mnemonics for wave forms
#define SINE                 (CR_B28_COMBINED | CR_MODE_D1_SINE)
#define SQUARE               (CR_B28_COMBINED | CR_OPBITEN)
#define FAST_SQUARE          (SQUARE | CR_DIV2)
#define TRIANGLE             (CR_B28_COMBINED | CR_MODE_D1_TRIANGLE)

#define FREQ0                0x4000
#define PHASE0               0xC000
#define REF_FREQ             25000000.0


#endif
/*****************************END OF FILE***************************/
