cd target\site
perl -i.orig -p ..\..\removeMaven.re appletalk.html
perl -i.orig -p ..\..\removeMaven.re developing.html
perl -i.orig -p ..\..\removeMaven.re ethernet.html
perl -i.orig -p ..\..\removeMaven.re history.html
perl -i.orig -p ..\..\removeMaven.re index.html
perl -i.orig -p ..\..\removeMaven.re operating.html
perl -i.orig -p ..\..\removeMaven.re printer.html
erase *.orig
cd ..\..
xcopy /y target\site\* /s ..\..\docs