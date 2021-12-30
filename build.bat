msbuild . "/property:Configuration=Release" /p:Platform=x86
cd TinyModGameMakerGame\datafiles
cmd /C 7z a ..\..\export\TinyModLoader-x86.zip dinput8.dll NativeMods
cd ..\..

msbuild . "/property:Configuration=Release" /p:Platform=x64
cd TinyModGameMakerGame\datafiles
cmd /C 7z a ..\..\export\TinyModLoader-x64.zip dinput8.dll NativeMods
cd ..\..

msbuild . "/property:Configuration=Release" /p:Platform=x86 /p:tmiDefs=TMI_DELAY
cd TinyModGameMakerGame\datafiles
cmd /C 7z a ..\..\export\TinyModLoader-x86-delay.zip dinput8.dll NativeMods
cd ..\..

msbuild . "/property:Configuration=Release" /p:Platform=x64 /p:tmiDefs=TMI_DELAY
cd TinyModGameMakerGame\datafiles
cmd /C 7z a ..\..\export\TinyModLoader-x64-delay.zip dinput8.dll NativeMods
cd ..\..
