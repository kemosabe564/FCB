#include <stdio.h>
#include <stdlib.h>

#define SIGN_BIT 0x80000000
#define EXP_BIT  0x7f800000
#define TAIL_BIT 0x007fffff


int main()
{
    // 1.5707963 0.5708
    
    float a = (float)1/(1000000*0.0081);
    a = 1.0/(256*1250*0.0081);
    // a = 1.0/256;
    a = 0.0081;

    int64_t result = 0;
    
//  1100100100001111 // 51471
//   110010010000111 // 25735
    for(int i=0; i<18; i++)
    {
        if(a*2>=1)
        {
            a=a*2-1;
            result = ((result << 1) | 1);
        }
        else
        {
            a=a*2;
            result = ((result << 1) | 0);
        }
    }
    // printf("%x\n", (result));
    printf("%ld\n", (result));
    // printf("%x\n", (result&0x1fff));

    // printf("%d\n", (1<<18));
    // printf("%ld\n", (1<<18)|result);
    return 0;

}
