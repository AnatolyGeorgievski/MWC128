	.file	"fletcher.c"
	.text
	.p2align 4
	.globl	fletcher4
	.def	fletcher4;	.scl	2;	.type	32;	.endef
	.seh_proc	fletcher4
fletcher4:
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	.seh_endprologue
	movq	%rcx, %rbx
	movq	%rdx, %rdi
	movq	%r9, %rsi
	cmpq	$3, %rdx
	jbe	.L6
	shrq	$2, %rdx
	movq	%rcx, %rax
	xorl	%r10d, %r10d
	leaq	(%rcx,%rdx,4), %r9
	leaq	-1(%rdx), %rbp
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L3:
	movl	(%rax), %r11d
	addq	$4, %rax
	addq	%r11, %r8
	addq	%r8, %rdx
	addq	%rdx, %rcx
	addq	%rcx, %r10
	cmpq	%r9, %rax
	jne	.L3
	leaq	4(%rbx,%rbp,4), %rbx
	andl	$3, %edi
.L2:
	testq	%rdi, %rdi
	je	.L4
	movzbl	(%rbx), %eax
	addq	%rax, %r8
	movq	%rdi, %rax
	addq	%r8, %rdx
	addq	%rdx, %rcx
	addq	%rcx, %r10
	decq	%rax
	je	.L4
	movzbl	1(%rbx), %r9d
	addq	%r9, %r8
	addq	%r8, %rdx
	addq	%rdx, %rcx
	addq	%rcx, %r10
	cmpq	$1, %rax
	je	.L4
	movzbl	2(%rbx), %eax
	addq	%rax, %r8
	addq	%r8, %rdx
	addq	%rdx, %rcx
	addq	%rcx, %r10
.L4:
	vmovq	%rdx, %xmm2
	vmovq	%r10, %xmm3
	vpinsrq	$1, %rcx, %xmm2, %xmm0
	vpinsrq	$1, %r8, %xmm3, %xmm1
	vinserti128	$0x1, %xmm0, %ymm1, %ymm0
	vmovdqu	%ymm0, (%rsi)
	vzeroupper
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	ret
	.p2align 4,,10
	.p2align 3
.L6:
	xorl	%r10d, %r10d
	xorl	%edx, %edx
	xorl	%ecx, %ecx
	jmp	.L2
	.seh_endproc
	.p2align 4
	.globl	fletcher64
	.def	fletcher64;	.scl	2;	.type	32;	.endef
	.seh_proc	fletcher64
fletcher64:
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	.seh_endprologue
	movq	%rcx, %rbx
	leaq	(%r8,%rdx), %rcx
	movq	%rdx, %r11
	movq	%rcx, %r8
	cmpq	$3, %rdx
	jbe	.L19
	movabsq	$-9223372034707292159, %rsi
	movl	$370724, %edi
	.p2align 4,,10
	.p2align 3
.L23:
	movq	%r11, %r10
	andq	$-4, %r10
	cmpq	$370725, %r11
	cmovnb	%rdi, %r10
	leaq	(%rbx,%r10), %r9
	cmpq	%r9, %rbx
	jnb	.L21
	movq	%rbx, %rax
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L22:
	movl	(%rax), %edx
	addq	$4, %rax
	addq	%rdx, %rcx
	addq	%rcx, %r8
	cmpq	%r9, %rax
	jb	.L22
	addq	%r10, %rbx
.L21:
	movq	%rcx, %rax
	subq	%r10, %r11
	mulq	%rsi
	shrq	$31, %rdx
	movq	%rdx, %rax
	salq	$32, %rax
	subq	%rdx, %rax
	subq	%rax, %rcx
	movq	%r8, %rax
	mulq	%rsi
	shrq	$31, %rdx
	movq	%rdx, %rax
	salq	$32, %rax
	subq	%rdx, %rax
	subq	%rax, %r8
	cmpq	$3, %r11
	ja	.L23
.L19:
	testq	%r11, %r11
	je	.L24
	movzbl	(%rbx), %r9d
	addq	%rcx, %r9
	leaq	(%r9,%r8), %r10
	cmpq	$1, %r11
	je	.L25
	movzbl	1(%rbx), %eax
	addq	%rax, %r9
	addq	%r9, %r10
	cmpq	$2, %r11
	je	.L25
	movzbl	2(%rbx), %eax
	addq	%rax, %r9
	addq	%r9, %r10
.L25:
	movabsq	$-9223372034707292159, %r8
	movq	%r9, %rax
	mulq	%r8
	movq	%rdx, %rcx
	shrq	$31, %rcx
	movq	%rcx, %rax
	salq	$32, %rax
	subq	%rcx, %rax
	movq	%r9, %rcx
	subq	%rax, %rcx
	movq	%r10, %rax
	mulq	%r8
	movq	%rdx, %r8
	shrq	$31, %r8
	movq	%r8, %rax
	salq	$32, %rax
	subq	%r8, %rax
	subq	%rax, %r10
	movq	%r10, %r8
.L24:
	movq	%r8, %rax
	salq	$32, %rax
	orq	%rcx, %rax
	popq	%rbx
	popq	%rsi
	popq	%rdi
	ret
	.seh_endproc
	.p2align 4
	.globl	zfs_fletcher2
	.def	zfs_fletcher2;	.scl	2;	.type	32;	.endef
	.seh_proc	zfs_fletcher2
zfs_fletcher2:
	.seh_endprologue
	cmpq	$15, %rdx
	jbe	.L42
	andq	$-16, %rdx
	vpxor	%xmm1, %xmm1, %xmm1
	addq	%rcx, %rdx
	vmovdqa	%xmm1, %xmm0
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L41:
	vpaddq	(%rcx), %xmm0, %xmm0
	addq	$16, %rcx
	vpaddq	%xmm0, %xmm1, %xmm1
	cmpq	%rcx, %rdx
	jne	.L41
	vinserti128	$0x1, %xmm1, %ymm0, %ymm0
	vmovdqu	%ymm0, (%r9)
	vzeroupper
	ret
	.p2align 4,,10
	.p2align 3
.L42:
	vpxor	%xmm0, %xmm0, %xmm0
	vmovdqu	%ymm0, (%r9)
	vzeroupper
	ret
	.seh_endproc
	.ident	"GCC: (Rev11, Built by MSYS2 project) 15.2.0"
