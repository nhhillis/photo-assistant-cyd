# Windows helper: builds and runs the host tests with g++ (no make needed).
$env:Path = [Environment]::GetEnvironmentVariable('Path','User') + ';' + [Environment]::GetEnvironmentVariable('Path','Machine')
$c = '../../lib/PhotoCore'
g++ -std=c++14 -Wall -Wextra "-I$c" -o test_core test_core.cpp $c/films.cpp $c/exposure.cpp $c/exposure_state.cpp $c/solar.cpp
if ($LASTEXITCODE -eq 0) { & .\test_core.exe }
