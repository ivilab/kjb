#define MUL_RE(a,b) (a.even*b.even - a.odd*b.odd)

#define MUL_IM(a,b) (a.even*b.odd + a.odd*b.even)

__kernel 
void ow_multiply_complex(__global float2* op1, __global float2* op2, unsigned int N)

{
    const unsigned int i = get_global_id(0);
    if(i >= N) return;
    float2 tmp;
    tmp.even = MUL_RE(op1[i],op2[i]); 
    tmp.odd = MUL_IM(op1[i],op2[i]);
    op1[i] = tmp;
}

__kernel 
void ow_multiply_constant(__global float* op1, float N)

{
    const unsigned int i = get_global_id(0);
    op1[i] = op1[i] * N;
}

