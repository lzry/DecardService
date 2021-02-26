Set oShell = CreateObject ("Wscript.Shell") 
Dim strArgs
strArgs = "cmd /c DecardService.exe >> decard_service.log"
oShell.Run strArgs, 0, false
