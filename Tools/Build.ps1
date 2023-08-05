# We have migrated the build scripts to Python 3.4+ (we will not support any Python version lower than that).

$pythonExec = "python.exe"
if ($null -ne $env:ALUSUS_PYTHON_EXECUTABLE) {
    $pythonExec = $env:ALUSUS_PYTHON_EXECUTABLE
}

& "$pythonExec" $(Join-Path -Path $(Join-Path -Path "$PSScriptRoot" -ChildPath "BuildSrc") -ChildPath "install_deps.py")
if ($?) {
    & "$pythonExec" $(Join-Path -Path $(Join-Path -Path "$PSScriptRoot" -ChildPath "BuildSrc") -ChildPath "build.py") @args
}

exit $LASTEXITCODE
