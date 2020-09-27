[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
Add-Type -AssemblyName System.IO.Compression.FileSystem

$scriptPath = split-path -parent $MyInvocation.MyCommand.Definition
cd $scriptPath

function DownloadAndUnzip
{
    param([string]$url, [string]$unzipDir)
    
    $zipfile = Split-Path $url -leaf
    Invoke-WebRequest -Uri $url -OutFile ./$zipfile
    Remove-Item $unzipDir -ErrorAction SilentlyContinue -r -fo
    [System.IO.Compression.ZipFile]::ExtractToDirectory($zipfile, $unzipDir)

    $dir = Get-ChildItem $unzipDir
    $directoryInfo =  $dir | Measure-Object
    if (($directoryInfo.Count -eq 1) -and  $dir[0] -is [System.IO.DirectoryInfo]) {
        
        $unzipedDir=-join($dir[0].FullName, "\*")
        mv $unzipedDir $unzipDir
        rm -fo $dir[0].FullName
    }
    
    rm -fo $zipfile
}


DownloadAndUnzip "https://curl.haxx.se/windows/dl-7.72.0_3/curl-7.72.0_3-win64-mingw.zip" "curl"
DownloadAndUnzip "https://github.com/opencv/opencv/releases/download/4.4.0/opencv-4.4.0-dldt-2020.4-vc16-avx2.zip" "opencv"
DownloadAndUnzip "https://software-network.org/client/sw-master-windows-client.zip" "sw"
DownloadAndUnzip "https://cppan.org/client/cppan-master-Windows-client.zip" "cppan"


############# install sw
echo "install sw......"
cd $scriptPath
cd sw
.\sw.exe --self-upgrade
.\sw.exe setup
$env:Path += ";" + $scriptPath + "\sw"

############# install cppan
echo "install cppan......"
cd $scriptPath
cd cppan
.\cppan.exe --self-upgrade
$env:Path += ";" + $scriptPath + "\cppan"

############# install leptonica
echo build leptonica.....
cd $scriptPath
cd leptonica
mkdir build -ErrorAction SilentlyContinue
cd build
cmake -G "Visual Studio 16 2019" -A x64 -DCPPAN_BUILD_SHARED_LIBS=0 –DBUILD_SHARED_LIBS=ON -DCMAKE_MODULE_LINKER_FLAGS=-whole-archive ..
Start-Process "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" -ArgumentList ".\leptonica.sln","/p:Configuration=Release","-fl","-flp:logfile=build.log"
 

############# install tesseract
echo build tesseract.....
cd $scriptPath
cd tesseract
mkdir build -ErrorAction SilentlyContinue
cd build
cmake -G "Visual Studio 16 2019" -A x64 ..
Start-Process "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" -ArgumentList ".\tesseract.sln","/p:Configuration=Release","-fl","-flp:logfile=build.log"

cd $scriptPath
