Set oShell = CreateObject ("Wscript.Shell") 
Dim strArgs
strArgs = "cmd /c DecardService.exe"
oShell.Run strArgs, 0, false
