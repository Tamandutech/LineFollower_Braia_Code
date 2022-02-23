from SCons.Script import COMMAND_LINE_TARGETS
Import("env", "projenv")

# sourceFile = open('saida.txt', 'w')

# print(env.Dump(), file=sourceFile)
# print(projenv.Dump(), file=sourceFile)

# sourceFile.close()

#
# (Optional) Do not run extra script when IDE fetches C/C++ project metadata
#

if "idedata" in COMMAND_LINE_TARGETS:
    print("Saindo do script...")
    env.Exit(0)

def BuildMergeCommands(source, target, env):
    print("Construindo comandos de merge...")

    # Comandos comentados abaixo utilizados para debug#
    ###################################################
    # sourceFile = open('saida.txt', 'w')   
    # print(env.Dump(), file=sourceFile)
    # sourceFile.close()

    # print(env['FLASH_EXTRA_IMAGES'])
    # print(len(env['FLASH_EXTRA_IMAGES']))
    ###################################################

    JOINCOMMANDS = [
        "$OBJCOPY",
        "--chip esp32",
        "merge_bin",
        "-o $BUILD_DIR/merged_qemu.bin",
        "--flash_mode dout",
        "--flash_size 4MB",
        "--fill-flash-size 4MB",
        "$ESP32_APP_OFFSET $BUILD_DIR/${PROGNAME}.bin",
    ]

    for i in range(len(env['FLASH_EXTRA_IMAGES'])):
        for j in range(len(env['FLASH_EXTRA_IMAGES'][i])):
            JOINCOMMANDS.append(env['FLASH_EXTRA_IMAGES'][i][j])

    env.Execute(" ".join(JOINCOMMANDS))

env.AddPostAction("buildprog", BuildMergeCommands)