
subroutine c(i,j)
  integer i,j

  if(i .ne. j) then
     continue
  end if
  continue
end subroutine

program main
call c(1,2)
write(*,*) "OK"
end
