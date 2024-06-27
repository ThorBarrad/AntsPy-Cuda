:: Converted with the help of https://daniel-sc.github.io/bash-shell-to-bat-converter/
@echo off

SET CMAKE_BUILD_TYPE=Release

SET itkgit=https://github.com/InsightSoftwareConsortium/ITK.git
SET itktag=v5.3rc03

@REM :: if there is a directory but no git, remove it
@REM if exist itksource\ (
@REM   if exist itksource\.git (
@REM     DEL /S /Q itksource\
@REM   )
@REM )
@REM :: if no directory, clone ITK in `itksource` dir
@REM if not exist itksource\ (
@REM   git clone %itkgit% itksource
@REM )
@REM cd itksource
@REM if exist .git\ (
@REM   git checkout master
@REM   git checkout %itktag%
@REM   DEL /S /Q .git\
@REM )

@REM :: go back to main dir
@REM cd ..\

echo Dependency;GitTag REM UNKNOWN: {"type":"Redirect","op":{"text":">","type":"great"},"file":{"text":"./data/softwareVersions.csv","type":"Word"}}
echo "ITK;%itktag%" REM UNKNOWN: {"type":"Redirect","op":{"text":">>","type":"dgreat"},"file":{"text":"./data/softwareVersions.csv","type":"Word"}}
mkdir itkbuild
cd itkbuild
SET compflags= -fPIC -O2  
cmake -G"Visual Studio 17 2022" -DITK_USE_SYSTEM_PNG=OFF -DCMAKE_SH:BOOL=OFF -DCMAKE_BUILD_TYPE:STRING="%CMAKE_BUILD_TYPE%" -DITK_USE_GIT_PROTOCOL:BOOL=OFF -DBUILD_SHARED_LIBS:BOOL=OFF -DBUILD_TESTING:BOOL=OFF -DBUILD_EXAMPLES:BOOL=OFF -DCMAKE_INSTALL_PREFIX:PATH=%R_PACKAGE_DIR%\libs\ -DITK_LEGACY_REMOVE:BOOL=OFF -DITK_FUTURE_LEGACY_REMOVE:=BOOL=ON -DITK_BUILD_DEFAULT_MODULES:BOOL=OFF -DKWSYS_USE_MD5:BOOL=ON -DITK_WRAPPING:BOOL=OFF -DModule_MGHIO:BOOL=ON -DModule_ITKDeprecated:BOOL=OFF -DModule_ITKReview:BOOL=ON -DModule_ITKVtkGlue:BOOL=OFF -DModule_GenericLabelInterpolator:BOOL=ON -DITKGroup_Core=ON -DModule_ITKReview=ON -DITKGroup_Filtering=ON -DITKGroup_IO=ON -DITKGroup_Numerics=ON -DITKGroup_Registration=ON -DITKGroup_Segmentation=ON -DModule_AdaptiveDenoising:BOOL=ON -DModule_GenericLabelInterpolator:BOOL=ON -DCMAKE_C_VISIBILITY_PRESET:BOOL=hidden -DCMAKE_CXX_VISIBILITY_PRESET:BOOL=hidden -DCMAKE_VISIBILITY_INLINES_HIDDEN:BOOL=ON ..\itksource\
msbuild /p:Configuration=Release /p:Platform=x64 ALL_BUILD.vcxproj

cd ..\
