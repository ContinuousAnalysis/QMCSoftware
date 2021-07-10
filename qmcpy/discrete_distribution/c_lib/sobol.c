/*
Digital Net Generator by alegresor 

References:
        
    [1] Paul Bratley and Bennett L. Fox. 1988. 
    Algorithm 659: Implementing Sobol's quasirandom sequence generator. 
    ACM Trans. Math. Softw. 14, 1 (March 1988), 88–100. 
    DOI:https://doi.org/10.1145/42288.214372
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "export_ctypes.h"
#include "MRG63k3a.h"

EXPORT int get_unsigned_long_size()
{
    return sizeof(unsigned long);
}

EXPORT int get_unsigned_long_long_size()
{
    return sizeof(unsigned long long);
}

EXPORT int set_lms_ds(
    unsigned long *dvec, /* vector of dimesions */
    unsigned int d, /* lenght(dvec) */
    unsigned int set_lms, /* linear matrix scrambling flag */
    unsigned int set_ds, /* digital shift flag */
    unsigned long long *seeds, /* seeds, one for each dimension in dvec */
    unsigned int d_max, /* maximum dimension */
    unsigned int m_max, /* 2^m_max is the maximum number of samples supported */
    unsigned int t_max, /* number of bits in each element of the generating vector */
    unsigned long long *zold, /* original generating vector with shape d_max x m_max*/
    unsigned int msb, /* most significant bit flag 
        1 = most significant bit (MSB) in 0^th column of directional numbers in j^th row
        0 = least significant bit (LSB) in 0^th column of direction numbers in j^th row
        Ex 
            if j^th row of directional numbers is
                [1 0 0]
                [0 1 0]
                [0 0 1]
            then MSB order uses int representation [4 2 1] 
            while LSB order has int representation [1 2 4]
        note that MSB order is faster as it does not require flipping bits */
    unsigned long long *znew, /* new generating vector with shape d x m_max to fill */ 
    unsigned long long *rshift){ /* random shift vector with shape d to fill */
    /*
    error codes:
        1) requires 32 bit precision but system has unsigned int with < 32 bit precision
        2) dimension in dvec greater than d_max
        3) t_max is too large for unsigned long long precision
    */
    /* parameter checks */
    if(sizeof(unsigned int)<4){ /* require 32 bit precision */
        return(1);}
    for(j=0;j<d;j++){
        if (dvec[j]>d_max){ /* dimension too large */
            return(2);}}
    if(t_max>sizeof(unsigned long long)){
        return(3);}
    /* variables */
    unsigned int j,m,t,t1;
    unsigned long long u,z;
    sm = (unsigned long long *) calloc(t_max, sizeof(unsigned long long)); /* scramble matrix */
    /* set randomizations */
    for(j=0;j<d;j++){
    	seed_MRG63k3a(seeds[j]); /* seed the IID RNG */
        /* set lms matrix*/
        if(set_lms==1){
            memset(sm,0,m_max*sizeof(unsigned long long));
            /* initialize the scrambling matrix */
            for(t=1;t<t_max;t++){
                u = (unsigned long long) (MRG63k3a() * (((unsigned long long) 1) << t)); /* get random int between 0 and 2^t */
                sm[t] = u << (t_max-t); /* lower triangular rectangular matrix */
                sm[t] |= ((unsigned long ) 1) << (t_max-t-1);}} /* set diagonal to 1 (ul to lr) */
        /* left multiply scrambling matrix to directional numbers */
        for(m=0;m<m_max;m++){
            z = zold[(dvec[j]*m_max+m)];
            u = 0;
            if(msb==0){ /* flip bits for lsb order */
                for(t=0;t<t_max;t++){
                    u |= ((z>>t)&1)<<(t_max-1-t);}}
            else{
                u = z;} /* use original element */
            v = 0;
            if(set_lms==1){
                /* left multiply scrambling matrix by direction number represeted as a column */
                for(t=0;t<t_max;t++){
                    s = 0;
                    b = sm[t]&u;
                    for(t1=0;t1<t_max;t1++){
                        s += (b>>t1)&1;}
                    s %= 2;
                    if(s){
                        v |= ((unsigned long long) 1) << (t_max-1-t);}}}
            else{
                v = u;}}
            znew[j*m_max+k] = v;
            /* initialize DS (will also be applied to LMS) */
            if(set_ds){
                rshift[j] = (unsigned long long) (MRG63k3a()*ldexp(1,t_max));}}
    free(sm);
    return(0);}

EXPORT int digitalseq2(
    unsigned long n, /* n: sample indicies include n0:(n0+n). Must be a power of 2 if not using Graycode ordering */
    unsigned int d, /* dimension supported by generating vector */
    unsigned long n0, /* starting index in sequence. Must be a power of 2 if not using Graycode ordering */
    unsigned int graycode, /* Graycode flag */
    unsigned int m_max, /* 2^m_max is the maximum number of samples supported */
    unsigned int t_max, /* number of bits in each element of the generating vector */
    unsigned long long *znew, /* new generating vector with shape d x m_max */
    double *x)}{ /* sample points with shape n x d to fill */
    /*
    Error Codes:
        1) using natural ordering (graycode=0) and n0 and/or (n0+n) is not 0 or a power of 2
        2) n0+n exceeds 2^m_max
    */
    /* parameter checks */
    if( (n==0) || (d==0) ){
        return(0);}
    if( (graycode==0) && ( ((n0!=0)&&fmod(log(n0)/log(2),1)!=0) || (fmod(log(n0+n)/log(2),1)!=0) ) ){
        /* for natural ordering, require n0 and (n0+n) be either 0 or powers of 2 */
        return(1);}
    if( ((n0+n)>ldexp(1,m_max)) ||  ){
        /* too many samples or dimensions */
        return(2);}
    /* variables */
    double scale = ldexp(1,-1*m_max);
    unsigned int j, m, k, k1, k2, s;
    unsigned long long i, im, u, rshift, xc, xr, z1, b, *sm, *zcp;
    /* generate points */
    for(j=0;j<d;j++){
        /* set an initial point */ 
        xc = 0; /* current point */
        z1 = 0; /* next directional vector */
        if(n0>0){
            im = n0-1;
            b = im; 
            im ^= im>>1;
            m = 0;
            while((im!=0) && (m<m_max)){
                if(im&1){
                    xc ^= zcp[m];}
                im >>= 1;
                m += 1;}
            s = 0;
            while(b&1){
                b >>= 1;
                s += 1;}
            z1 = zcp[s];}
        /* set the rest of the points */
        for(i=n0;i<(n0+n);i++){
            xc ^= z1;
            xr = xc;     
            /* set point */
            im = i;
            if(!graycode){
                im = i^(i>>1);}
            if((randomize==1)&&set_xjlms){
                xjlms[(im-n0)*d+j] = ((double) xr)*scale;}
            if((randomize==1) || (randomize==2)){
                xr ^= rshift;}
            x[(im-n0)*d+j] = ((double) xr)*scale;
            /* get the index of the rightmost 0 bit in i */
            b = i; 
            s = 0;
            while(b&1){
                b >>= 1;
                s += 1;}
            /* get the vector used for the next index */
            z1 = znew[s];}}
    return(0);}

/*
int main(){
    unsigned long n = 8;
    unsigned int d = 2;
    unsigned long n0 = 0; 
    unsigned int d0 = 0;
    unsigned int randomize = 1;
    unsigned int graycode = 0;
    unsigned long long seeds[2] = {7,17};
    double *x = (double*) calloc(n*d,sizeof(double));
    unsigned int d_max = 3;
    unsigned int m_max = 32;
    
    unsigned long long z[3][32] = {
        {2147483648     ,1073741824     ,536870912      ,268435456      ,134217728      ,67108864       ,33554432       ,16777216       ,8388608        ,4194304        ,2097152        ,1048576        ,524288         ,262144         ,131072         ,65536          ,32768          ,16384          ,8192           ,4096           ,2048           ,1024           ,512            ,256            ,128            ,64             ,32             ,16             ,8              ,4              ,2              ,1              },
        {2147483648     ,3221225472     ,2684354560     ,4026531840     ,2281701376     ,3422552064     ,2852126720     ,4278190080     ,2155872256     ,3233808384     ,2694840320     ,4042260480     ,2290614272     ,3435921408     ,2863267840     ,4294901760     ,2147516416     ,3221274624     ,2684395520     ,4026593280     ,2281736192     ,3422604288     ,2852170240     ,4278255360     ,2155905152     ,3233857728     ,2694881440     ,4042322160     ,2290649224     ,3435973836     ,2863311530     ,4294967295     },
        {2147483648     ,3221225472     ,1610612736     ,2415919104     ,3892314112     ,1543503872     ,2382364672     ,3305111552     ,1753219072     ,2629828608     ,3999268864     ,1435500544     ,2154299392     ,3231449088     ,1626210304     ,2421489664     ,3900735488     ,1556135936     ,2388680704     ,3314585600     ,1751705600     ,2627492864     ,4008611328     ,1431684352     ,2147543168     ,3221249216     ,1610649184     ,2415969680     ,3892340840     ,1543543964     ,2382425838     ,3305133397     }};
    unsigned int msb = 1;
    
    unsigned long long z[3][32] = {
            {1              ,2              ,4              ,8              ,16             ,32             ,64             ,128            ,256            ,512            ,1024           ,2048           ,4096           ,8192           ,16384          ,32768          ,65536          ,131072         ,262144         ,524288         ,1048576        ,2097152        ,4194304        ,8388608        ,16777216       ,33554432       ,67108864       ,134217728      ,268435456      ,536870912      ,1073741824     ,2147483648     },
	        {1              ,3              ,5              ,15             ,17             ,51             ,85             ,255            ,257            ,771            ,1285           ,3855           ,4369           ,13107          ,21845          ,65535          ,65537          ,196611         ,327685         ,983055         ,1114129        ,3342387        ,5570645        ,16711935       ,16843009       ,50529027       ,84215045       ,252645135      ,286331153      ,858993459      ,1431655765     ,4294967295     },
	        {1              ,3              ,6              ,9              ,23             ,58             ,113            ,163            ,278            ,825            ,1655           ,2474           ,5633           ,14595          ,30470          ,43529          ,65815          ,197434         ,394865         ,592291         ,1512982        ,3815737        ,7436151        ,10726058       ,18284545       ,54132739       ,108068870      ,161677321      ,370540567      ,960036922      ,2004287601     ,2863268003     }};
    
    unsigned int msb = 0;
    unsigned long long z[0][0];
    unsigned int msb = 2;
    double *xjlms = (double*) calloc(n*d,sizeof(double));
    unsigned int set_xjlms = 1;
    int rc; 
    rc = sobol(n, d, n0, d0, randomize, graycode, seeds, x, d_max, m_max, *z, msb, xjlms, set_xjlms);
    printf("Return code: %d\n\n",rc);
    printf("x\n");
    for(unsigned long i=0; i<n; i++){
        for(int j=0; j<d; j++){
            printf("%.3f\t",x[i*d+j]);}
        printf("\n");}
    printf("\nxjlms\n");
    for(unsigned long i=0; i<n; i++){
        for(int j=0; j<d; j++){
            printf("%.3f\t",xjlms[i*d+j]);}
        printf("\n");}   
    return(0);}
*/
