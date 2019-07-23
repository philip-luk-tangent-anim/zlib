call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"

if not exist .\win64_vs_build (
    mkdir .\win64_vs_build
)

cd .\win64_vs_build

if exist .\CMakeCache.txt ( 
    del CMakeCache.txt
) 

cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_INSTALL_PREFIX=T:\Development\philip.luk\repo\local\thirdparty\zlib\1.2.11\win_x64 ..\
      
REM "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\amd64\msbuild.exe" .\INSTALL.vcxproj /p:Configuration=Release /t:Build /m 

cmake --build . --target install --config Release -- /maxcpucount:8 

cd ..\
