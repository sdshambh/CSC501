.global zfunction
.text
	zfunction: #prologue
		push %ebp
		movl %esp , %ebp
		#saving live registers
		push %esi
		push %edi
		push %ebx
		#put params in register/stack
		movl 8(%ebp) , %ebx
		#moving 0xfffc01ff as maskbit
		movl $0xfffc01ff , %eax
		andl %eax , %ebx
		shrl $8 , %ebx
		movl $0xff000000 , %edi
		orl %edi , %ebx
		movl %ebx , %eax
		#clean stack and restore
		pop %ebx
		pop %edi
		pop %esi
		#epilogue
		movl %ebp , %esp
		pop %ebp
		ret
		
		