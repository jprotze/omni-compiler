static char rcsid[] = "$Id$";
/* 
 * $TSUKUBA_Release: Omni OpenMP Compiler 3 $
 * $TSUKUBA_Copyright:
 *  PLEASE DESCRIBE LICENSE AGREEMENT HERE
 *  $
 */
/* error case of netsting 028 :
 * for の中に master が存在する場合
 */

#include <omp.h>


int	errors = 0;
int	thds;
int	sum;


main ()
{
  thds = omp_get_max_threads ();
  if (thds == 1) {
    printf ("should be run this program on multi threads.\n");
    exit (0);
  }
  omp_set_dynamic (0);

  #pragma omp parallel
  {
    int	i;

    #pragma omp for
    for (i=0; i<thds; i++) {
      #pragma omp master
      {
	sum += 1;
      }
    }
  }


  printf ("err_nesting 028 : FAILED, can not compile this program.\n");
  return 1;
}
