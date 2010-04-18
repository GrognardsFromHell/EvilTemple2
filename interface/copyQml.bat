IF NOT EXIST ..\bin\data MKDIR ..\bin\data
IF NOT EXIST ..\bin\data\interface MKDIR ..\bin\data\interface

XCOPY /Y /E interface ..\bin\data\interface
