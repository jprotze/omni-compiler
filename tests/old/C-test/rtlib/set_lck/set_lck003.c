static char rcsid[] = "$Id$";
/* 
 * $TSUKUBA_Release: Omni OpenMP Compiler 3 $
 * $TSUKUBA_Copyright:
 *  PLEASE DESCRIBE LICENSE AGREEMENT HERE
 *  $
 */
/* omp_set_lock 003:
 * lock変数毎に、個別に制御ができる事を確認。
 */

#include <omp.h>
#include "omni.h"


main ()
{
  omp_lock_t	lck, lck2;
  int		thds;

  int		errors = 0;


  thds = omp_get_max_threads ();
  if (thds == 1) {
    printf ("should be run this program on multi thread.\n");
    exit (0);
  }

  omp_init_lock(&lck);
  omp_init_lock(&lck2);

  #pragma omp parallel sections
  {
    #pragma omp section
    {
      barrier (2);

      omp_set_lock (&lck);
      barrier (2);
      omp_unset_lock (&lck);

      omp_set_lock (&lck2);
      barrier (2);
      omp_unset_lock (&lck2);
    }

    #pragma omp section
    {
      barrier (2);

      omp_set_lock (&lck2);
      barrier (2);
      omp_unset_lock (&lck2);

      omp_set_lock (&lck);
      barrier (2);
      omp_unset_lock (&lck);
    }
  }


  if (errors == 0) {
    printf ("omp_set_lock 003 : SUCCESS\n");
    return 0;
  } else {
    printf ("omp_set_lock 003 : FAILED\n");
    return 1;
  }
}
