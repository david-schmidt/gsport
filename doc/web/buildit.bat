@REM
@REM Sorry, this is hopelessly tied to my environment...
@REM I hope folks don't really want to build the doc themselves.
@REM
call c:\dev\env
rmdir /q /s target
call mvn site
xcopy target\site\* /s ..\..\docs