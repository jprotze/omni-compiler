#include <stdio.h>
#include <math.h>
#include <xmp.h>

int n=9;
double a[n][n],b[n][n];
#pragma xmp nodes p(2,2)
#pragma xmp template tx(0:n-1,0:n-1)
#pragma xmp distribute tx(block,block) onto p
#pragma xmp align a[i][j] with tx(i,j)
#pragma xmp align b[i][j] with tx(i,j)

int main(){

  int i,j,myrank,ierr;
  double err ;

  myrank=xmp_all_node_num();

#pragma xmp loop (i,j) on tx(i,j)
  for(i=0;i<n;i++){
    for(j=0;j<n;j++){
      a[i][j]=i+j+2;
    }
  }

#pragma xmp loop (i,j) on tx(i,j)
  for(i=0;i<n;i++){
    for(j=0;j<n;j++){
      b[i][j]=0;
    }
  }

#pragma xmp gmove
  b[0:n][0:n]=a[0:n][0:n];

  err=0.0;
#pragma xmp loop (i,j) on tx(i,j)
  for(i=0;i<n;i++){
    for(j=0;j<n;j++){
      err=err+fabs(b[i][j]-a[i][j]);
    }
  }

#pragma xmp reduction (MAX:err)
  if (myrank ==1){
    printf("max error=%f\n",err);
  }
  ierr=err;

  return ierr;

}
