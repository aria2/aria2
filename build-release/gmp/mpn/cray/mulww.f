c     Helper for mpn_mul_1, mpn_addmul_1, and mpn_submul_1 for Cray PVP.

c     Copyright 1996, 2000 Free Software Foundation, Inc.

c     This file is part of the GNU MP Library.

c     The GNU MP Library is free software; you can redistribute it and/or
c     modify it under the terms of the GNU Lesser General Public License as
c     published by the Free Software Foundation; either version 3 of the
c     License, or (at your option) any later version.

c     The GNU MP Library is distributed in the hope that it will be useful,
c     but WITHOUT ANY WARRANTY; without even the implied warranty of
c     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
c     Lesser General Public License for more details.

c     You should have received a copy of the GNU Lesser General Public License
c     along with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.

c     p1[] = hi(a[]*s); the upper limbs of each product
c     p0[] = low(a[]*s); the corresponding lower limbs
c     n is number of limbs in the vectors

      subroutine gmpn_mulww(p1,p0,a,n,s)
      integer*8 p1(0:*),p0(0:*),a(0:*),s
      integer n

      integer*8 a0,a1,a2,s0,s1,s2,c
      integer*8 ai,t0,t1,t2,t3,t4

      s0 = shiftl(and(s,4194303),24)
      s1 = shiftl(and(shiftr(s,22),4194303),24)
      s2 = shiftl(and(shiftr(s,44),4194303),24)

      do i = 0,n-1
         ai = a(i)
         a0 = shiftl(and(ai,4194303),24)
         a1 = shiftl(and(shiftr(ai,22),4194303),24)
         a2 = shiftl(and(shiftr(ai,44),4194303),24)

         t0 = i24mult(a0,s0)
         t1 = i24mult(a0,s1)+i24mult(a1,s0)
         t2 = i24mult(a0,s2)+i24mult(a1,s1)+i24mult(a2,s0)
         t3 = i24mult(a1,s2)+i24mult(a2,s1)
         t4 = i24mult(a2,s2)

         p0(i)=shiftl(t2,44)+shiftl(t1,22)+t0
         c=shiftr(shiftr(t0,22)+and(t1,4398046511103)+
     $        shiftl(and(t2,1048575),22),42)
         p1(i)=shiftl(t4,24)+shiftl(t3,2)+shiftr(t2,20)+shiftr(t1,42)+c
      end do
      end
