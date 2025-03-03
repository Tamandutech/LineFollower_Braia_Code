@ECHO OFF

REM Define the directory of your CubeIDE workspace
set workspace=C:\Users\samoc\STM32CubeIDE\workspace

REM Define the name of your project
set project=LineFollower_Braia_Code

REM Define the base directory of the CubeIDE installation
set "basedir="
REM Loop through directories in "C:\ST" to find the STM32CubeIDE installation folder
for /d %%d in ("C:\ST\STM32CubeIDE_*") do (
    REM If "basedir" is not set, assign the first found directory
    if not defined basedir (
        set "basedir=%%~fd"
    ) else (
        REM Compare versions: Extract the version number and compare
        for /f "tokens=2 delims=_" %%a in ("%%~nd") do (
            for /f "tokens=2 delims=_" %%b in ("!basedir!") do (
                REM Compare the found version with the one already stored
                if %%a gtr %%b set "basedir=%%~fd"
            )
        )
    )
)
REM Uncomment the line below if you want to set a specific version
REM set basedir=C:\ST\STM32CubeIDE_1.12.0


:: Check the provided argument
if "%1" == "build" (
    echo Executing build...
    REM Call CubeIDE to perform the build
    "%basedir%\STM32CubeIDE\stm32cubeidec.exe" --launcher.suppressErrors -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -build %project% -data "%workspace%"
    ECHO Build completed.

) else if "%1" == "flash" (
    echo Flashing...
    REM Call STM32_Programmer_CLI to upload the code
    STM32_Programmer_CLI -c port=SWD -w Debug\%project%.elf -v -rst
    ECHO Programming completed.

) else if "%1" == "all" (
    echo Executing build and flashing...

    REM Call CubeIDE to perform the build
    "%basedir%\STM32CubeIDE\stm32cubeidec.exe" --launcher.suppressErrors -nosplash -application org.eclipse.cdt.managedbuilder.core.headlessbuild -build %project% -data "%workspace%"
    ECHO Build completed.

    REM Call STM32_Programmer_CLI to upload the code
    STM32_Programmer_CLI -c port=SWD -w Debug\%project%.elf -v -rst
    ECHO Programming completed.

) else (
    echo Unrecognized argument. Use "build", "flash" or "all".
)