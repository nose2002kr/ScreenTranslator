[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
Add-Type -AssemblyName System.IO.Compression.FileSystem

$scriptPath = split-path -parent $MyInvocation.MyCommand.Definition
cd $scriptPath

$MSBUILD = "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe"

function DownloadAndUnzip
{
    param([string]$url, [string]$unzipDir)
    
    $zipfile = Split-Path $url -leaf
    Invoke-WebRequest -Uri $url -OutFile ./$zipfile
    Remove-Item $unzipDir -ErrorAction SilentlyContinue -r -fo
    if ($zipfile.EndsWith(".zip")) {
        [System.IO.Compression.ZipFile]::ExtractToDirectory($zipfile, $unzipDir)
    } elseif ($zipfile.EndsWith(".7z")) {
        Start-Process "$env:ProgramFiles\7-Zip\7z.exe" -ArgumentList "x",("-o"+$unzipDir),$zipfile -NoNewWindow -Wait
    }

    $dir = Get-ChildItem $unzipDir
    $directoryInfo =  $dir | Measure-Object
    if (($directoryInfo.Count -eq 1) -and  $dir[0] -is [System.IO.DirectoryInfo]) {
        
        $unzipedDir=-join($dir[0].FullName, "\*")
        mv $unzipedDir $unzipDir
        rm -fo $dir[0].FullName
    }
    
    rm -fo $zipfile
}

#DownloadAndUnzip "https://curl.haxx.se/windows/dl-7.72.0_3/curl-7.72.0_3-win64-mingw.zip" "curl"
DownloadAndUnzip "https://github.com/opencv/opencv/releases/download/4.4.0/opencv-4.4.0-dldt-2020.4-vc16-avx2.zip" "opencv"
DownloadAndUnzip "https://github.com/opencv/opencv/releases/download/4.4.0/opencv-4.4.0-dldt-2020.4-vc16-avx2-debug.7z" "opencv-debug"
#DownloadAndUnzip "https://software-network.org/client/sw-master-windows-client.zip" "sw"
#DownloadAndUnzip "https://cppan.org/client/cppan-master-Windows-client.zip" "cppan"
DownloadAndUnzip "https://www.npcglib.org/~stathis/downloads/openssl-1.0.2k-vs2017.7z" "openssl"

############# build libjson
echo "build libjson....."
cd $scriptPath
cd libjson
Start-Process $MSBuild -ArgumentList ".\libjson.sln","/p:Configuration=Release","-fl","-flp:logfile=build.log" -NoNewWindow -Wait

############# build libcurl
echo "build libcurl....."
cd $scriptPath
cd openssl
cmd /c "mklink /J .\inc32 .\include64\"
cd $scriptPath
cd curl\projects\Windows\VC16\lib
Start-Process $MSBuild -ArgumentList ".\libcurl.sln","`"/p:Configuration=LIB Release - LIB OpenSSL`"","/p:Platform=x64","-fl","-flp:logfile=build.log" -NoNewWindow -Wait
Start-Process $MSBuild -ArgumentList ".\libcurl.sln","`"/p:Configuration=LIB Debug - LIB OpenSSL`"","/p:Platform=x64","-fl","-flp:logfile=build.log" -NoNewWindow -Wait

############# build vcpkg
echo "build vcpkg....."
cd $scriptPath
cd vcpkg
cd toolsrc
mkdir build -ErrorAction SilentlyContinue
cd build
cmake -G "Visual Studio 16 2019" -A x64 ..
Start-Process $MSBuild -ArgumentList ".\vcpkg.sln","/p:Configuration=Release","-fl","-flp:logfile=build.log" -NoNewWindow -Wait

############# build tesseract
echo "build tesseract...."
Start-Process ".\Release\vcpkg.exe" -ArgumentList "install","--triplet","x64-windows","tesseract" -NoNewWindow -Wait

cd $scriptPath
