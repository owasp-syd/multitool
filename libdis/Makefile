export CC     := gcc
export CFLAGS := -m32 -g -fpic

X86  := imm operand_list disasm insn format misc
IA32 := implicit invariant opcode_tables reg insn modrm operand settings

.PHONY: libdis.so clean
libdis.so: $(IA32:%=ia32_%.o) $(X86:%=x86_%.o)
	$(CC) $(CFLAGS) -shared -o $@ $^

clean:; rm -f *.so *.o *.gch
