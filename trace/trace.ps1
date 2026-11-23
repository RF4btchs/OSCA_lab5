# Трассировка загрузки DLL отладчиком cdb (WinDbg) для prog1 и prog2.
# Точки останова (bps.wds) ставятся на функции загрузчика ntdll (LdrLoadDll, LdrUnloadDll,
# LdrGetProcedureAddressForCaller) и на системные вызовы NtOpenFile / NtCreateSection /
# NtMapViewOfSection / NtUnmapViewOfSection, через которые DLL отображается в память.
# Результат: trace\syscalls_prog1.txt и trace\syscalls_prog2.txt
param([ValidateSet('prog1', 'prog2', 'all')] [string]$Prog = 'all')

$ErrorActionPreference = 'Stop'
$Lab  = Split-Path -Parent $PSScriptRoot
$Work = Join-Path $env:TEMP 'trace_lab5'
New-Item -ItemType Directory -Force $Work | Out-Null
$dir = $Work -replace '\\', '/'
Get-ChildItem $PSScriptRoot -Filter *.wds | ForEach-Object {
    (Get-Content $_.FullName -Raw) -replace '@@DIR@@', $dir | Set-Content -Encoding ascii (Join-Path $Work $_.Name)
}

$pkg = Get-AppxPackage Microsoft.WinDbg -ErrorAction SilentlyContinue
if (-not $pkg) { throw 'WinDbg не установлен: winget install Microsoft.WinDbg' }
$cdb = Join-Path $pkg.InstallLocation 'amd64\cdb.exe'
$env:_NT_SYMBOL_PATH = "srv*$env:TEMP\symcache*https://msdl.microsoft.com/download/symbols"

function Trace-One([string]$name) {
    $log = Join-Path $Work "$name.log"
    $out = Join-Path $Work "$name.out.txt"
    $err = Join-Path $Work "$name.err.txt"
    Remove-Item $log, $out, $err -ErrorAction SilentlyContinue
    # cmd.exe нужен только для перенаправления stdin; точки останова ставятся лишь
    # в процессе, который он запускает (событие cpr), сам cmd в трассу не попадает
    $cmdline = "bin\$name.exe < trace\input_$name.txt"
    $p = Start-Process $cdb -WorkingDirectory $Lab -NoNewWindow -Wait -PassThru `
            -RedirectStandardOutput $out -RedirectStandardError $err `
            -ArgumentList @('-o', '-G', '-logou', $log, '-c', "`"`$`$<$dir/main.wds`"",
                            "$env:SystemRoot\System32\cmd.exe", '/c', $cmdline)
    if (-not (Test-Path $log)) { throw "cdb не создал лог (код $($p.ExitCode))" }

    $text = [IO.File]::ReadAllText($log, [Text.Encoding]::Unicode)
    $calls = [regex]::Matches($text, '(?m)^SC\|([0-9a-f]+)\|([0-9a-f]+)\|ntdll!(\w+)[^|]*\|(.*?)\r?$')
    $lines = foreach ($m in $calls) {
        '{0,-6}{1,6} {2,6}  {3,-32} {4}' -f $name, [Convert]::ToInt32($m.Groups[1].Value, 16),
            [Convert]::ToInt32($m.Groups[2].Value, 16), $m.Groups[3].Value, ($m.Groups[4].Value -replace '`', '')
    }
    $hdr = "proc     PID    TID  call                             arguments"
    $res = Join-Path $PSScriptRoot "syscalls_$name.txt"
    @($hdr) + $lines | Set-Content -Encoding utf8 $res
    Write-Host "$name : $($lines.Count) calls -> $res"
}

$list = if ($Prog -eq 'all') { 'prog1', 'prog2' } else { @($Prog) }
foreach ($n in $list) { Trace-One $n }
