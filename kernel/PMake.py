from pathlib import Path

MAKEFILE_STUB = r"""
EFI_HEADER_LOC = ../bootloader/include/efi

OBJECTSDIR = objects
LIBSDIR = lib

CCNOLINK = gcc		                    \
	-mabi=ms

CXXNOLINK = g++ 	                    \
	-mabi=ms                			

CCKER = $(CCNOLINK)                     \
    -Wl,-e,kmain                        \
    -Wl,-T,linker.ld					\
	-Wl,--build-id=none

CXXKER = $(CXXNOLINK)                   \
    -Wl,-e,kmain                        \
    -Wl,-T,linker.ld					\
	-Wl,--build-id=none					\
	-z noexecstack

AR = ar                                 \
    -rcs

CFLAGS = 								\
	-std=c2x							\
	-Wall								\
	-Wextra								\
	-Wpedantic 							\
	-fms-extensions 					\
	-mno-red-zone 						\
	-ffreestanding 						\
	-nostdlib 							\
	-I$(EFI_HEADER_LOC) 				\
	-Iinclude							\
	-D__EFI_STANDALONE__                \
	-fno-PIE							\
	-mcmodel=large						\
	-static								\
	-fno-exceptions						

CXXFLAGS =							    \
	-std=c++23						    \
	-Wall							    \
	-Wextra							    \
	-Wpedantic						    \
	-fms-extensions					    \
	-mno-red-zone					    \
	-ffreestanding					    \
	-nostdlib						    \
    -I$(EFI_HEADER_LOC)                 \
	-Iinclude                           \
    -D__EFI_STANDALONE__				\
	-fno-PIE							\
	-mcmodel=large						\
	-static								\
	-fno-exceptions						\
	-fno-rtti

CFASTNOSSE = 							\
	-Ofast								\
	-mno-sse
"""

cwd = Path(r'.')
src = cwd / "src"

target = 'kernel.img'

objects_directory = 'objects'
libs_directory = 'lib'
exclude_directories = ['include', objects_directory, libs_directory]

libraries = []

def generateDirectory(output, path: Path):
    libname = path.name + '.lib'
    libraries.append(path.name)
    
    cxx_sources = list(path.glob('**/*.cpp'))

    if cxx_sources != []:
        output.write(f"{path.name}_cxxsources = ")
        for s in cxx_sources:
            output.write(f"{s.as_posix()} ")
        output.write("\n")

        output.write(f"{path.name}_cxxobjects = $(patsubst {src}/{path.name}/%.cpp, {objects_directory}/{path.name}/%.o,$({path.name}_cxxsources))\n")

        output.write(f"$({path.name}_cxxobjects): {objects_directory}/{path.name}/%.o: {src}/{path.name}/%.cpp | $(OBJECTSDIR)\n")
        output.write("\t@mkdir -p $(@D)\n")
        output.write("\t@echo Building $@\n")
        output.write("\t@$(CXXNOLINK) $(CXXFLAGS) $(CFASTNOSSE) -o $@ -c $<\n")

    c_sources = list(path.glob('**/*.c'))

    if c_sources != []:
        output.write(f"{path.name}_csources = ")
        for s in c_sources:
            output.write(f"{s.as_posix()} ")
        output.write("\n")

        output.write(f"{path.name}_cobjects = $(patsubst {src}/{path.name}/%.c, {objects_directory}/{path.name}/%.o,$({path.name}_csources))\n")

        output.write(f"$({path.name}_cobjects): {objects_directory}/{path.name}/%.o: {src}/{path.name}/%.c | $(OBJECTSDIR)\n")
        output.write("\t@mkdir -p $(@D)\n")
        output.write("\t@echo Building $@\n")
        output.write("\t@$(CCNOLINK) $(CFLAGS) $(CFASTNOSSE) -o $@ -c $<\n")

    asm_sources = list(path.glob('**/*.asm'))
    
    if asm_sources != []:
        output.write(f"{path.name}_asmsources = ")
        for s in asm_sources:
            output.write(f"{s.as_posix()} ")
        output.write("\n")

        output.write(f"{path.name}_asmobjects = $(patsubst {src}/{path.name}/%.asm, {objects_directory}/{path.name}/%.o,$({path.name}_asmsources))\n");

        output.write(f"$({path.name}_asmobjects): {objects_directory}/{path.name}/%.o: {src}/{path.name}/%.asm | $(OBJECTSDIR)\n")
        output.write(f"\t@mkdir -p $(@D)\n")
        output.write("\t@echo Building $@\n")
        output.write("\t@nasm -felf64 $< -o $@\n")
    
    output.write(f"{libs_directory}/{libname}: $({path.name}_cxxobjects) $({path.name}_cobjects) $({path.name}_asmobjects) | $(LIBSDIR)\n")
    output.write("\t@echo Creating $@\n")
    output.write("\t@$(AR) $@ $^\n")

MAKEFILE_STUB += rf"""
all: {target}
""" + f"$(OBJECTSDIR):\n\t@mkdir $(OBJECTSDIR)\n" + f"$(LIBSDIR):\n\t@mkdir $(LIBSDIR)\n"

with open('Makefile', 'w') as output:
    output.write(MAKEFILE_STUB)

    for d in src.iterdir():
        if d.is_dir() and d.name not in exclude_directories:
            generateDirectory(output, d)

    libraries_ser = ""
    for l in libraries:
        libraries_ser += f"{libs_directory}/{l}.lib "

    root_objects = ""

    root_cxx_sources = list(src.glob('*.cpp'))
    root_c_sources = list(src.glob('*.c'))
    root_asm_sources = list(src.glob('*.asm'))

    if root_cxx_sources != []:
        output.write("_cxx_sources = ");
        for s in root_cxx_sources:
            output.write(f"{s.as_posix()} ")
        output.write('\n')

        output.write(f"_cxx_objects = $(patsubst {src}/%.cpp,{objects_directory}/%.o,$(_cxx_sources))\n")

        output.write(f"$(_cxx_objects): {objects_directory}/%.o: {src}/%.cpp | $(OBJECTSDIR)\n")
        output.write("\t@echo Building $@\n")
        output.write("\t@$(CXXNOLINK) $(CXXFLAGS) $(CFASTNOSSE) -o $@ -c $<\n")
        root_objects += '$(_cxx_objects) '

    if root_c_sources != []:
        output.write("_c_sources = ")
        for s in root_c_sources:
            output.write(f"{s.as_posix()} ")
        output.write('\n')

        output.write(f"_c_objects = $(patsubst {src}/%.c, {objects_directory}/%.o,$(_c_sources))\n")

        output.write(f"$(_c_objects): {objects_directory}/%.o: {src}/%.c | $(OBJECTSDIR)\n")
        output.write("\t@echo Building $@\n")
        output.write("\t@$(CCNOLINK) $(CFLAGS) $(CFASTNOSSE) -o $@ -c $<\n")
        root_objects += '$(_c_objects) '

    if root_asm_sources != []:
        output.write("_asm_sources = ")
        for s in root_asm_sources:
            output.write(f"{f.as_posix()}")
        output.write('\n')

        output.write(f"_asm_objects = $(patsubst) {src}/%.asm, {objects_directory}/%.o,$(_asm_sources))\n")

        output.write(f"$(_asm_objects): {objects_directory}/%.o: {src}/%.asm | $(OBJECTSDIR)")
        output.write(f"\t@echo Building $@\n")
        output.write(f"\t@nasm -felf64 $< -o $@")
        root_objects += '$(_asm_objects) '

    
    output.write(f"{target}: {root_objects}{libraries_ser}\n")
    output.write("\t@echo Building $@\n");
    output.write(f"\t@$(CXXKER) $(CXXFLAGS) -Wl,--start-group -L{libs_directory} ")
    for l in libraries:
        output.write(f"-l:{l}.lib ")
    output.write("-o $@ $< -Wl,--end-group\n")

    output.write(".PHONY: clean\n")
    output.write("clean:\n")
    output.write("\t@rm -rf lib\n")
    output.write("\t@rm -rf objects\n")
    output.write(f"\t@rm -f {target}\n")
    output.write("\t@echo Done.");