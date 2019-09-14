//
// The Computer Image
//
// This file implements a Fast Fourier Transform of a 1D function
//

#include <math.h>
#include "../fft.h"

#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr

void four1(float *data,int nn,int isign)
//
// Replaces data by its discrete Fourier transform, if isign is input as 1;
// or replaces data by nn times its inverse discrete Fourier transform, if
// isign is input as -1. data is a complex array of length nn, input as a 
// real array data[1..2*nn]. nn must be an integer power of 2.
//
{
     int n,mmax,m,j,istep,i;
     double wtemp,wr,wpr,wpi,wi,theta;
     float tempr,tempi;

     n=nn<<1;
     j=1;
     for( i=1;i<n;i+=2 )
          {
           if (j>i)
              {
               SWAP(data[j],data[i]);
               SWAP(data[j+1],data[i+1]);
              }
           m=n>>1;
           while( m>=2 && j>m )
                  {
                   j-=m;
                   m>>=1;
                  }
           j+=m;
          }
     mmax=2;
     while( n>mmax )
            {
             istep=mmax<<1;
             theta=6.28318530717959/(isign*mmax);
             wtemp=sin(0.5*theta);
             wpr=-2.0*wtemp*wtemp;
             wpi=sin(theta);
             wr=1.0;
             wi=0.0;
             for( m=1;m<mmax;m+=2 )
                 {
                  for( i=m;i<=n;i+=istep )
                       {
                        j=i+mmax;
                        tempr=wr*data[j]-wi*data[j+1];
                        tempi=wr*data[j+1]+wi*data[j];
                        data[j]=data[i]-tempr;
                        data[j+1]=data[i+1]-tempi;
                        data[i]+=tempr;
                        data[i+1]+=tempi;
                       }
                  wr=(wtemp=wr)*wpr-wi*wpi+wr;
                  wi=wi*wpr+wtemp*wpi+wi;
                 }
             mmax=istep;
            }

}

void twofft(float *data1,float *data2,float *fft1,float *fft2,int n)
//
// Given two real input arrays data[1..n] and data2[1..n], this routine
// calls four1 and returns two complex output arrays, fft1 and fft2,
// each of complex length n (real dimensions of [1..2n]), which contain
// the discrete Fourier transforms of the respective datas. n must be 
// a integer power of two.
//
{
     int nn3,nn2,jj,j;
     float rep,rem,aip,aim;

     nn3=1+(nn2=2+n+n);
     for( j=1,jj=2;j<=n;j++,jj+=2 )
          {
           fft1[jj-1]=data1[j];
           fft1[jj]=data2[j];
          }
     four1(fft1,n,1);
     fft2[1]=fft1[2];
     fft1[2]=fft2[2]=0.0;
     for( j=3;j<=n+1;j+=2 )
          {
           rep=0.5*(fft1[j]+fft1[nn2-j]);
           rem=0.5*(fft1[j]-fft1[nn2-j]);
           aip=0.5*(fft1[j+1]+fft1[nn3-j]);
           aim=0.5*(fft1[j+1]-fft1[nn3-j]);
           fft1[j]=rep;
           fft1[j+1]=aim;
           fft1[nn2-j]=rep;
           fft1[nn3-j]=-aim;
           fft2[j]=aip;
           fft2[j+1]=-rem;
           fft2[nn2-j]=aip;
           fft2[nn3-j]=rem;
          }
}

void realft(float *data,int n,int isign)
//
// Calculates the Fourier transform of a set of 2n real-valued data points.
// Replace this data (which is stored in array data[1..2n]) by the positive
// frequency half of its complex Fourier transform. The real-valued first
// and last components of the complex transform are returned as elements
// data[1] and data[2] recpectively. n must be a power of 2. This routine 
// also calculates the inverse transform of a complex data array if it is
// the transform of real data (in this case the result sould be multiplied
// by 1/n).
//
{
     int i,i1,i2,i3,i4,n2p3;
     float c1=0.5,c2,h1r,h1i,h2r,h2i;
     double wr,wi,wpr,wpi,wtemp,theta;

     theta=3.141592653589793/(double)n;
     if (isign==1)
        {
         c2=-0.5;
         four1(data,n,1);
        }
     else {
           c2=0.5;
           theta=-theta;
          }
     wtemp=sin(0.5*theta);
     wpr=-2.0*wtemp*wtemp;
     wpi=sin(theta);
     wr=1.0+wpr;
     wi=wpi;
     n2p3=2*n+3;
     for( i=2;i<=n/2;i++ )
          {
           i4=1+(i3=n2p3-(i2=1+(i1=i+i-1)));
           h1r=c1*(data[i1]+data[i3]);
           h1i=c1*(data[i2]-data[i4]);
           h2r=-c2*(data[i2]+data[i4]);
           h2i=c2*(data[i1]-data[i3]);
           data[i1]=h1r+wr*h2r-wi*h2i;
           data[i2]=h1i+wr*h2i+wi*h2r;
           data[i3]=h1r-wr*h2r+wi*h2i;
           data[i4]=-h1i+wr*h2i+wi*h2r;
           wr=(wtemp=wr)*wpr-wi*wpi-wr;
           wi=wi*wpr+wtemp*wpi+wi;
          }
     if (isign==1)
        {
         data[1]=(h1r=data[1])+data[2];
         data[2]=h1r-data[2];
        }
     else {
           data[1]=c1*((h1r=data[1])+data[2]);
           data[2]=c1*(h1r-data[2]);
           four1(data,n,-1);
          }
}

void sinft(float *y,int n)
//
// Calculates the sine transform of a set of n real-valued data points
// stored in array y[1..n]. The number n must be a power of 2. On exit
// y is replaced by its transform. This program, without changes, also
// calculates the inverse sine transform, but in this case the output
// arry should be multiplied by 2/n
//
{
    int j,m=n/2,n2=n+1;
    float sum,y1,y2;
    double theta,wi=0.0,wr=1.0,wpi,wpr,wtemp;
    
    theta=3.14159265358979/(double)n;

    wtemp=sin(0.5*theta);
    wpr=-2.0*wtemp*wtemp;
    wpi=sin(theta);
    y[1]=0.0;
    for( j=2;j<=m+1;j++ )
         {
          wr=(wtemp=wr)*wpr-wi*wpi+wr;
          wi=wi*wpr+wtemp*wpi+wi;
          y1=wi*(y[j]+y[n2-j]);
          y2=0.5*(y[j]-y[n2-j]);
          y[j]=y1+y2;
          y[n2-j]=y1-y2;
         }
    realft(y,m,1);
    y[1]*=0.5;
    sum=y[2]=0.0;
    for( j=1;j<=n-1;j+=2 )
         {
          sum+=y[j];
          y[j]=y[j+1];
          y[j+1]=sum;
         }

}

void cosft(float *y,int n,int isign)
//
// Calculates the cosine transform of a set y[1..0] of real-valued data
// points. The transformed data replace the original data in array y.
// n must be a power of 2. Set isign to +1 for a transform and to -1 for
// an inverse transform. For an inverse transform, the output array should
// be multiplied by 2/n
//
{
    int j,m,n2;
    float enf0,even,odd,sum,sume,sumo,y1,y2;
    double theta,wi=0.0,wr=1.0,wpi,wpr,wtemp;
    
    theta=3.14159265358979/(double)n;

    wtemp=sin(0.5*theta);
    wpr=-2.0*wtemp*wtemp;
    wpi=sin(theta);
    sum=y[1];
    m=n>>1;
    n2=n+2;
    for( j=2;j<=m;j++ )
         {
          wr=(wtemp=wr)*wpr-wi*wpi+wr;
          wi=wi*wpr+wtemp*wpi+wi;
          y1=0.5*(y[j]+y[n2-j]);
          y2=(y[j]-y[n2-j]);
          y[j]=y1-wi*y2;
          y[n2-j]=y1+wi*y2;
          sum+=wr*y2;
         }
    realft(y,m,1);
    y[2]=sum;
    for( j=4;j<=n;j+=2 )
         {
          sum+=y[j];
          y[j]=sum;
         }
    if (isign==-1)
       {
        even=y[1];
        odd=y[2];
        for( j=3;j<=n-1;j+=2 )
             { 
              even+=y[j];
              odd+=y[j+1];
             }
        enf0=2.0*(even-odd);
        sumo=y[1]-enf0;
        sume=(2.0*odd/n)-sumo;
        y[1]=0.5*enf0;
        y[2]-=sume;
        for( j=3;j<=n-1;j+=2 )
             {
              y[j]-=sumo;
              y[j+1]-=sume;
             }
       }
}

void fourn(float *data,int *nn,int ndim,int isign)
{
     int i1,i2,i3,i2rev,i3rev,ip1,ip2,ip3,ifp1,ifp2;
     int ibit,idim,k1,k2,n,nprev,nrem,ntot;
     float tempi,tempr;
     double theta,wi,wpi,wpr,wr,wtemp;

     ntot=1;
     for( idim=1;idim<=ndim;idim++ )
          ntot*=nn[idim];
     nprev=1;
     for( idim=ndim;idim>=1;idim-- )
          {
           n=nn[idim];
           nrem=ntot/(n*nprev);
           ip1=nprev<<1;
           ip2=ip1*n;
           ip3=ip2*nrem;
           i2rev=1;
           for( i2=1;i2<=ip2;i2+=ip1 )
                {
                 if (i2<i2rev)
                    {
                     for( i1=i2;i1<=i2+ip1-2;i1+=2 )
                          {
                           for( i3=i1;i3<=ip3;i3+=ip2 )
                                {
                                 i3rev=i2rev+i3-i2;
                                 SWAP(data[i3],data[i3rev]);
                                 SWAP(data[i3+1],data[i3rev+1]);
                                }
                          }
                    }
                 ibit=ip2>>1;
                 while( ibit>=ip1 && i2rev>ibit)
                        {
                         i2rev-=ibit;
                         ibit>>=1;
                        }
                 i2rev+=ibit;
                }
           ifp1=ip1;
           while( ifp1<ip2 )
                  {
                   ifp2=ifp1<<1;
                   theta=isign*6.28318530717959/(ifp2/ip1);
                   wtemp=sin(0.5*theta);
                   wpr=-2.0*wtemp*wtemp;
                   wpi=sin(theta);
                   wr=1.0;
                   wi=0.0;
                   for( i3=1;i3<=ifp1;i3+=ip1 )
                        {
                         for( i1=i3;i1<=i3+ip1-2;i1+=2 )
                              {
                               for( i2=i1;i2<=ip3;i2+=ifp2 )
                                    {
                                     k1=i2;
                                     k2=k1+ifp1;
                                     tempr=wr*data[k2]-wi*data[k2+1];
                                     tempi=wr*data[k2+1]+wi*data[k2];
                                     data[k2]=data[k1]-tempr;
                                     data[k2+1]=data[k1+1]-tempi;
                                     data[k1]+=tempr;
                                     data[k1+1]+=tempi;
                                    }
                              }
                         wr=(wtemp=wr)*wpr-wi*wpi+wr;
                         wi=wi*wpr+wtemp*wpi+wi;
                        }
                   ifp1=ifp2;
                  }
           nprev*=n;
          }
}
