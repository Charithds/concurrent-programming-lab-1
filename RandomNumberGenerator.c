#include <stdio.h>

#define POL_MASK_32 0xB4BCD35C
#define POL_MASK_31 0x7A5BC2E3

typedef unsigned int uint;
uint lfsr32, lfsr31;

int shift_lsfr(uint *lfsr, uint polynominal_mask)
{
    int feedback;
    feedback = *lfsr & 1;
    *lfsr >>= 1;
    if(feedback == 1)
        *lfsr ^= polynominal_mask;
    return *lfsr;
}

void init_lsfrs(void)
{
    lfsr32 = 0xABCDE;       /*seed values*/
    lfsr31 = 0x23456789;
}

int get_random(void)
{
    shift_lsfr(&lfsr32, POL_MASK_32);
    return (shift_lsfr(&lfsr32, POL_MASK_32) ^ shift_lsfr(&lfsr31, POL_MASK_31)) & 0xffff;
}

int main(void)
{
    int random_value;
    init_lsfrs();
    for(int i = 0; i < 100; i++)
    {
        random_value = get_random();
        printf("%d ", random_value);
    }
}