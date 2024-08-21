from pathlib import Path

MAKEFILE_STUB = r"""
EFI_HEADER_LOC = ../bootloader/include/efi

OBJECTSDIR = objects
LIBSDIR = lib

CCNOLINK = clang 						\
	-target x86_64-unknown-windows

AR = ar -cr

CCKER = $(CCNOLINK) 					\
	-fuse-ld=lld 						\
	-Wl,-subsystem:efi_application 		\
	-Wl,-entry:kmain 					\
	-Wl,-FIXED,-BASE:0xffff800000000000

CFLAGS = 								\
	-std=c23							\
	-Wall								\
	-Wextra								\
	-Wpedantic 							\
	-fms-extensions 					\
	-Wno-microsoft-anon-tag 			\
	-mno-red-zone 						\
	-ffreestanding 						\
	-nostdlib 							\
	-I$(EFI_HEADER_LOC) 				\
	-Iinclude							\
	-D__EFI_STANDALONE__

CFASTNOSSE = 							\
	-Ofast								\
	-mno-sse
"""

cwd = Path(r'.')
src = cwd / "src"

main_source = 'kernel.c'
target = 'kernel.exe'

objects_directory = 'objects'
libs_directory = 'lib'
exclude_directories = ['include', objects_directory, libs_directory]

libraries = []

def generateDirectory(output, path: Path):
    libname = path.name + '.lib'
    libraries.append(path.name)

    c_sources = list(path.glob('**/*.c'))

    if c_sources != []:
        output.write(f"{path.name}_csources = ");
        for s in c_sources:
            output.write(f"{s.as_posix()} ")
        output.write("\n")

        output.write(f"{path.name}_cobjects = $(patsubst src/{path.name}/%.c, {objects_directory}/{path.name}/%.o,$({path.name}_csources))\n");

        output.write(f"$({path.name}_cobjects): {objects_directory}/{path.name}/%.o: src/{path.name}/%.c | $(OBJECTSDIR)\n")
        output.write(f"\t@powershell -Command \"mkdir -Force $(@D)\" 1> nul\n")
        output.write("\t@echo Building $@\n")
        output.write("\t@$(CCNOLINK) $(CFLAGS) $(CFASTNOSSE) -o $@ -c $<\n")

    asm_sources = list(path.glob('**/*.asm'))
    
    if asm_sources != []:
        output.write(f"{path.name}_asmsources = ")
        for s in asm_sources:
            output.write(f"{s.as_posix()} ")
        output.write("\n")

        output.write(f"{path.name}_asmobjects = $(patsubst src/{path.name}/%.asm, {objects_directory}/{path.name}/%.o,$({path.name}_asmsources))\n");

        output.write(f"$({path.name}_asmobjects): {objects_directory}/{path.name}/%.o: src/{path.name}/%.asm | $(OBJECTSDIR)\n")
        output.write(f"\t@powershell -Command \"mkdir -Force $(@D)\" 1> nul\n")
        output.write("\t@echo Building $@\n")
        output.write("\t@nasm -fwin64 $< -o $@\n")
    
    output.write(f"{libs_directory}/{libname}: $({path.name}_cobjects) $({path.name}_asmobjects) | $(LIBSDIR)\n")
    output.write("\t@echo Creating $@\n")
    output.write("\t@$(AR) $@ $^\n")

MAKEFILE_STUB += rf"""
all: {target}
""" + f"$(OBJECTSDIR):\n\t@mkdir $(OBJECTSDIR)\n" + f"$(LIBSDIR):\n\t@mkdir $(LIBSDIR)\n"

with open('Makefile', 'w') as output:
    output.write(MAKEFILE_STUB)

    for d in src.iterdir():
        if d.is_dir() and d.name not in exclude_directories:
            libname = d.name + '.lib'
            generateDirectory(output, d)

    libraries_ser = ""
    for l in libraries:
        libraries_ser += f"{libs_directory}/{l}.lib "
    
    output.write(f"{target}: src/{main_source} {libraries_ser}\n")
    output.write("\t@echo Building $@\n");
    output.write(f"\t@$(CCKER) $(CFLAGS) -L{libs_directory} ")
    for l in libraries:
        output.write(f"-l{l} ")
    output.write("-o $@ $<\n")

    output.write(".PHONY: clean\n")
    output.write("clean:\n")
    output.write("\tif exist lib rmdir /Q /S lib\n")
    output.write("\tif exist objects rmdir /Q /S objects\n")
    output.write(f"\tif exist {target} del /Q /S {target}\n")