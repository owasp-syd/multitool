X86  := imm operand_list disasm insn format misc
IA32 := implicit invariant opcode_tables reg insn modrm operand settings

.PHONY: build clean
build: $(IA32:%=ia32_%.o) $(X86:%=x86_%.o)
clean:; rm -f *.o *.gch
