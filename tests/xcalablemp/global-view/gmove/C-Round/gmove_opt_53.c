#include <xmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define N 20
#define NB 4
#define NPROW 4
#define NPCOL 2
int x[N], b[NPCOL][N];
#pragma xmp nodes p(NPROW,NPCOL)
#pragma xmp template t_b(0:N-1,0:NPCOL-1)
#pragma xmp distribute t_b(cyclic(NB),block) onto p
#pragma xmp align b[i][j] with t_b(j,i)

#pragma xmp template t_x(0:N-1)
#pragma xmp nodes row(NPCOL) = p(*,1:NPCOL)
#pragma xmp distribute t_x(cyclic(NB)) onto row
#pragma xmp align x[j] with t_x(j)

int main()
{
#pragma xmp loop (j,i) on t_b(j,i)
  for(int i=0;i<NPCOL;i++)
    for(int j=0;j<N;j++)
      b[i][j] = i*N+j;

#pragma xmp loop on t_x(j)
  for(int j=0;j<N;j++)
    x[j] = 99;

  int s = NB * 4;
  int c = s / NB % NPCOL;
#pragma xmp gmove
  x[s:NB] = b[c][s:NB];

  int flag = true;
  if(1 <= xmp_node_num() && xmp_node_num() <= 4)
    if(x[s] == s && x[s+1] == s+1 && x[s+2] == s+2 && x[s+3] == s+3)
      flag = true;
    else
      flag = false;

#pragma xmp reduction (MIN:flag)
#pragma xmp task on p(1,1)
  if(flag == false){
    printf("ERROR in gmove_opt_53\n");
    exit(1);
  }
  else
    printf("PASS gmove_opt_53\n");

  return 0;
}
