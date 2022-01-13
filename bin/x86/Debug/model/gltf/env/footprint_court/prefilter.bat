echo off
SETLOCAL ENABLEDELAYEDEXPANSION

FOR %%I in (*.hdr) DO (
	REM set CurDir=%%~nI/
 	REM echo !CurDir!
	REM if not exist !CurDir! mkdir "!CurDir!"
	
	REM cd !CurDir!
	REM if not exist lambertian mkdir lambertian
	REM if not exist ggx mkdir ggx
	REM if not exist charlie mkdir charlie
	REM cd ..
	
	REM set outputPathLambertian=!CurDir!/lambertian/diffuse.ktx2
	REM cli.exe -inputPath %%I -distribution Lambertian -outCubeMap !outputPathLambertian!

	REM set outputPathGgx=!CurDir!/ggx/specular.ktx2
	REM cli.exe -inputPath %%I -distribution GGX -outCubeMap !outputPathGgx! -mipLevelCount 11 -lodBias 0

	REM set outputPathCharlie=!CurDir!/charlie/sheen.ktx2
	REM cli.exe -inputPath %%I -distribution Charlie -outCubeMap !outputPathCharlie! -mipLevelCount 11 -lodBias 0
)

FOR %%I in (*.hdr) DO (
	set CurDir=%%~nI
	cli.exe -inputPath %%I -sampleCount 2048 -mipLevelCount 5 -cubeMapResolution 256 -distribution Lambertian -outCubeMap diffuse_env.ktx2
	cli.exe -inputPath %%I -sampleCount 1024 -mipLevelCount 5 -cubeMapResolution 256 -distribution GGX            -outCubeMap specular_env.ktx2 -outLUT lut.png
	REM cli.exe -inputPath %%I -sampleCount 2048 -mipLevelCount 5 -distribution Lambertian -outCubeMap diffuse_env.ktx2
	REM cli.exe -inputPath %%I -sampleCount 1024 -mipLevelCount 5 -distribution GGX           -outCubeMap specular_env.ktx2 -outLUT lut.png
)
pause