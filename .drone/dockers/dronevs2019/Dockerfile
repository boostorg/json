# escape=`
# Use the latest Windows Server Core image with .NET Framework 4.8.
FROM mcr.microsoft.com/dotnet/framework/sdk:4.8-windowsservercore-ltsc2019
# FROM mcr.microsoft.com/windows/servercore:ltsc2019
# Restore the default Windows shell for correct batch processing.
SHELL ["cmd", "/S", "/C"]

WORKDIR C:\
RUN powershell -Command iex ((new-object net.webclient).DownloadString('https://chocolatey.org/install.ps1'))

# List from https://docs.travis-ci.com/user/reference/windows/

# move package to top, so the selected version will be installed
RUN choco install -y visualstudio2017buildtools --version 15.9.18.0

RUN choco install -y 7zip.install --version 19.0
RUN choco install -y chocolatey --version 0.10.15
RUN choco install -y chocolatey-core.extension --version 1.3.5.1
RUN choco install -y chocolatey-dotnetfx.extension --version 1.0.1
RUN choco install -y chocolatey-fastanswers.extension --version 0.0.2
RUN choco install -y chocolatey-visualstudio.extension --version 1.8.1
RUN choco install -y chocolatey-windowsupdate.extension --version 1.0.4

# cmake not in path without extra flag:
RUN choco install -y cmake.install --version 3.16.2 --installargs '"ADD_CMAKE_TO_PATH=System"'

RUN choco install -y curl --version 7.68.0
RUN choco install -y DotNet4.5.2 --version 4.5.2.20140902
RUN choco install -y DotNet4.6 --version 4.6.00081.20150925
RUN choco install -y DotNet4.6-TargetPack --version 4.6.00081.20150925
RUN choco install -y DotNet4.6.1 --version 4.6.01055.20170308

# This is already the dotnet image
# RUN choco install -y dotnetfx --version 4.8.0.20190930

RUN choco install -y git.install --version 2.25.0
RUN choco install -y hashdeep --version 4.4
RUN choco install -y jq --version 1.6
RUN choco install -y KB2919355 --version 1.0.20160915
RUN choco install -y KB2919442 --version 1.0.20160915
RUN choco install -y KB2999226 --version 1.0.20181019
RUN choco install -y KB3033929 --version 1.0.5
RUN choco install -y KB3035131 --version 1.0.3
RUN choco install -y llvm --version 9.0.0
RUN choco install -y microsoft-build-tools --version 15.0.26320.2
RUN choco install -y mingw --version 8.1.0
RUN choco install -y netfx-4.5.1-devpack --version 4.5.50932
RUN choco install -y netfx-4.5.2-devpack --version 4.5.5165101.20180721
RUN choco install -y netfx-4.6.1-devpack --version 4.6.01055.00
RUN choco install -y rsync --version 5.5.0.20190204
RUN choco install -y ruby --version 2.7.0.1
RUN choco install -y vcredist140 --version 14.24.28127.4
RUN choco install -y vcredist2017 --version 14.16.27033
RUN choco install -y visualstudio-installer --version 2.0.1
RUN choco install -y visualstudio2017-workload-netcorebuildtools --version 1.1.2
RUN choco install -y visualstudio2017-workload-vctools --version 1.3.2
RUN choco install -y visualstudio2017-workload-webbuildtools --version 1.3.2
RUN choco install -y Wget --version 1.20.3.20190531
RUN choco install -y windows-sdk-10.1 --version 10.1.18362.1
RUN choco install -y winscp --version 5.15.9
RUN choco install -y winscp.install --version 5.15.9

# wsl error. skip for now.
# RUN choco install -y wsl --version 1.0.1

RUN choco install -y python --version 3.8.3
RUN mklink /D "Git" "C:\Program Files\Git"

# Adding visualstudio2019
RUN choco install visualstudio2019-workload-vctools --package-parameters "--add Microsoft.VisualStudio.Component.VC.14.24.x86.x64" -y

RUN setx path "C:\Git\usr\bin\;%path%"
ENTRYPOINT ["powershell.exe", "-NoLogo", "-ExecutionPolicy", "Bypass"]
