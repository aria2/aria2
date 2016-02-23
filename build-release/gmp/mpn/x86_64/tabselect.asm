dnl  AMD64 mpn_tabselect.

dnl  Copyright 2011, 2012 Free Software Foundation, Inc.

dnl  This file is part of the GNU MP Library.

dnl  The GNU MP Library is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU Lesser General Public License as published
dnl  by the Free Software Foundation; either version 3 of the License, or (at
dnl  your option) any later version.

dnl  The GNU MP Library is distributed in the hope that it will be useful, but
dnl  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
dnl  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
dnl  License for more details.

dnl  You should have received a copy of the GNU Lesser General Public License
dnl  along with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.

include(`../config.m4')


C	     cycles/limb
C AMD K8,K9	 2.5
C AMD K10	 2.5
C AMD bobcat	 3.5
C Intel P4	 4
C Intel core2	 2.33
C Intel NHM	 2.5
C Intel SBR	 2.2
C Intel atom	 5
C VIA nano	 3.5

C NOTES
C  * This has not been tuned for any specific processor.  Its speed should not
C    be too bad, though.
C  * Using SSE2/AVX2 could result in many-fold speedup.

C mpn_tabselect (mp_limb_t *rp, mp_limb_t *tp, mp_size_t n, mp_size_t nents, mp_size_t which)
define(`rp',     `%rdi')
define(`tp',     `%rsi')
define(`n',      `%rdx')
define(`nents',  `%rcx')
define(`which',  `%r8')

define(`i',      `%rbp')
define(`maskp',  `%r11')
define(`maskn',  `%r12')

C rax rbx  rcx  rdx rdi rsi rbp (rsp)  r8   r9 r10 r11 r12 r13 r14 r15
C         nents  n  rp  tab           which

ABI_SUPPORT(DOS64)
ABI_SUPPORT(STD64)

ASM_START()
	TEXT
	ALIGN(16)
PROLOGUE(mpn_tabselect)
	FUNC_ENTRY(4)
IFDOS(`	mov	56(%rsp), %r8d	')
	push	%rbx
	push	%rbp
	push	%r12

	lea	(rp,n,8), rp
	lea	(tp,n,8), tp
	sub	nents, which
L(outer):
	lea	(which,nents), %rax
	neg	%rax			C set CF iff 'which' != k
	sbb	maskn, maskn
	mov	maskn, maskp
	not	maskp

	mov	n, i
	neg	i
	test	$1, R32(n)
	je	L(top)
	mov	(tp,i,8), %rax
	and	maskp, %rax
	mov	(rp,i,8), %r9
	and	maskn, %r9
	or	%r9, %rax
	mov	%rax, (rp,i,8)
	add	$1, i
	jns	L(end)

	ALIGN(16)
L(top):	mov	(tp,i,8), %rax
	mov	8(tp,i,8), %rbx
	and	maskp, %rax
	and	maskp, %rbx
	mov	(rp,i,8), %r9
	mov	8(rp,i,8), %r10
	and	maskn, %r9
	and	maskn, %r10
	or	%r9, %rax
	or	%r10, %rbx
	mov	%rax, (rp,i,8)
	mov	%rbx, 8(rp,i,8)
	add	$2, i
	js	L(top)

L(end):	lea	(tp,n,8), tp
	dec	nents
	jne	L(outer)

L(outer_end):
	pop	%r12
	pop	%rbp
	pop	%rbx
	FUNC_EXIT()
	ret
EPILOGUE()
