#!/bin/bash

#-------------------------------------------------------
#  generator for module xmp_coarray_get
#-------------------------------------------------------

#----------------------------------------
#  sub
#----------------------------------------
echo72 () {
    str="$1                                                                        "
    str=`echo "$str" | cut -c -72`"&"
    echo "$str"
}

print_decl() {
    tk=$1
    typekind=$2
    echo72     "      function xmpf_coarray_get${DIM}d_${tk}(descptr, baseaddr, element,"
    echo72     "     &   coindex, mold, rank"
    for i in `seq 1 ${DIM}`; do
        echo72 "     &   , nextaddr${i}, count${i}"
    done
    echo '     &   ) result(val)'
    echo '      integer(8), intent(in) :: descptr'
    echo '      integer, intent(in) :: element, coindex, rank'
    for i in `seq 1 ${DIM}`; do
        echo "      integer, intent(in) :: count${i}"
    done
    echo '      integer(8), intent(in) :: baseaddr'
    for i in `seq 1 ${DIM}`; do
        echo "      integer(8), intent(in) :: nextaddr${i}"
    done
    case ${DIM} in
        0)  echo "      ${typekind} :: mold" ;;
        1)  echo "      ${typekind} :: mold(count1)" ;;
        *)  echo72 "      ${typekind} :: mold(count1"
            echo -n "         "
            for i in `seq 2 ${DIM}`; do
                echo -n ",count${i}"
            done
            echo ')' ;;
    esac
    case ${DIM} in
        0)  echo "      ${typekind} :: val" ;;
        1)  echo "      ${typekind} :: val(count1)" ;;
        *)  echo72 "      ${typekind} :: val(count1"
            echo -n "         "
            for i in `seq 2 ${DIM}`; do
                echo -n ",count${i}"
            done
            echo ')' ;;
    esac
    echo
}

print_body() {
    tk=$1
    typekind=$2

    case ${DIM} in
        0)  echo72 "      call xmpf_coarray_get_scalar(descptr, baseaddr, element,"
            echo   "     &   coindex, val)"
            ;;
        *)  echo72 "      call xmpf_coarray_get_array(descptr, baseaddr, element,"
            echo72 "     &   coindex, val, rank"
            for i in `seq 1 ${DIM}`; do
                echo72 "     &   , nextaddr${i}, count${i}"
            done
            echo '     &   )'
            ;;
    esac
    echo
}

print_function() {
    print_decl $1 $2
    print_body $1 $2
    echo "      end function"
    echo
}

print_interface() {
    print_decl $1 $2
    echo "      end function"
    echo
}


#----------------------------------------
#  main
#----------------------------------------
echo "!! This file is automatically generated by xmpf_coarray_get.sh"
echo

if [ "$1" == "-xmp" ]; then
#====================
# to make .xmod
# for F_FRONT
#====================

echo "      module xmp_coarray_get"
echo ""

for DIM in `seq 0 7`
do
    echo "      interface xmpf_coarray_get${DIM}d"
    echo ""
    print_interface i2  "integer(2)"      
    print_interface i4  "integer(4)"      
    print_interface i8  "integer(8)"      
    print_interface l2  "logical(2)"      
    print_interface l4  "logical(4)"      
    print_interface l8  "logical(8)"      
    print_interface r4  "real(4)"         
    print_interface r8  "real(8)"         
    print_interface z8  "complex(4)"      
    print_interface z16 "complex(8)"      
    print_interface cn  "character(element)" 
    echo "      end interface xmpf_coarray_get${DIM}d"
    echo ""
done

echo "      end module xmp_coarray_get"
echo

else
#====================
# to make .xmod
# for F_FRONT
#====================

echo "      module xmp_coarray_get"
echo ""

for DIM in `seq 0 7`
do
    echo "      interface xmpf_coarray_get${DIM}d"
    for tk in i2 i4 i8 l2 l4 l8 r4 r8 z8 z16 cn; do
        echo "        module procedure xmpf_coarray_get${DIM}d_${tk}"
    done
    echo "      end interface xmpf_coarray_get${DIM}d"
    echo ""
done

echo "      contains"
echo ""

for DIM in `seq 0 7`
do
    print_function i2  "integer(2)"      
    print_function i4  "integer(4)"      
    print_function i8  "integer(8)"      
    print_function l2  "logical(2)"      
    print_function l4  "logical(4)"      
    print_function l8  "logical(8)"      
    print_function r4  "real(4)"         
    print_function r8  "real(8)"         
    print_function z8  "complex(4)"      
    print_function z16 "complex(8)"      
    print_function cn  "character(element)" 
done

echo "      end module xmp_coarray_get"
echo ""

fi
exit