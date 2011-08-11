template<>
inline __declspec(naked) boost::uint64_t atomic_read<8>(const volatile void *)
{
    __asm {
	    ALIGN 4
	    mov ecx, 4[esp]
	    test ecx, 7
	    jne slow
	    sub esp, 12
	    fild qword ptr [ecx]
	    fistp qword ptr [esp]
	    mov eax, [esp]
	    mov edx, 4[esp]
	    add esp, 12
	    ret
    slow:
	    push ebx
	    push edi
	    mov edi, ecx
	    xor eax, eax
	    xor ebx, ebx
	    xor ecx, ecx
	    xor edx, edx
	    lock cmpxchg8b qword ptr [edi]
	    pop edi
	    pop ebx
	    ret
	}
}

template<>
inline __declspec(naked) boost::uint64_t atomic_add<8>(volatile void *, boost::uint64_t)
{
    __asm {
        ALIGN 4
	    push ebx
	    push edi
	    mov edi, 12[esp]
	    mov eax, [edi]
	    mov edx, 4[edi]
    mloop:
	    mov ebx, 16[esp]
	    mov ecx, 20[esp]
	    add ebx, eax
	    adc ecx, edx
	    lock cmpxchg8b qword ptr [edi]
	    jnz mloop
	    pop edi
	    pop ebx
	    ret
    }
}

template<>
inline __declspec(naked) void atomic_write<8>(volatile void *, boost::uint64_t)
{
    __asm {
	    push ebx
	    push edi
	    mov edi, 12[esp]
	    mov ebx, 16[esp]
	    mov ecx, 20[esp]
	    mov eax, [edi]
	    mov edx, 4[edi]
    mloop:
	    lock cmpxchg8b qword ptr [edi]
	    jnz mloop
	    pop edi
	    pop ebx
	    ret
	}
}

template<>
inline __declspec(naked) boost::uint64_t atomic_cas<8>(volatile void *, boost::uint64_t, boost::uint64_t)
{
    __asm {
	    push ebx
	    push edi
	    mov edi, 12[esp]
	    mov ebx, 16[esp]
	    mov ecx, 20[esp]
	    mov eax, 24[esp]
	    mov edx, 28[esp]
	    lock cmpxchg8b qword ptr [edi]
	    pop edi
	    pop ebx
	    ret
    }
}

template<>
inline __declspec(naked) boost::uint64_t atomic_read_write<8>(volatile void *, boost::uint64_t)
{
    __asm {
	    push ebx
	    push edi
	    mov edi, 12[esp]
	    mov ebx, 16[esp]
	    mov ecx, 20[esp]
	    mov eax, [edi]
	    mov edx, 4[edi]
    mloop:
	    lock cmpxchg8b qword ptr [edi]
	    jnz mloop
	    pop edi
	    pop ebx
	    ret
	}
}
