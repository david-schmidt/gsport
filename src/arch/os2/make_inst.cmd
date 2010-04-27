/*

make_inst.cmd:

Translation of the make_inst perl script and makefile to rexx for OS/2

8inst_s.h: instable.h
	$(PERL) make_inst s 8 instable.h > 8inst_s.h

16inst_s.h: instable.h
	$(PERL) make_inst s 16 instable.h > 16inst_s.h

8inst_c.h: instable.h
	$(PERL) make_inst c 8 instable.h > 8inst_c.h

16inst_c.h: instable.h
	$(PERL) make_inst c 16 instable.h > 16inst_c.h

*/

count = 0;

FileNameIn = "..\..\instable.h"
FileNameOut8s = "..\..\8inst_s.h"
FileNameOut8c = "..\..\8inst_c.h"
FileNameOut16s = "..\..\16inst_s.h"
FileNameOut16c = "..\..\16inst_c.h"

DEL FileNameOut8s
DEL FileNameOut8c
DEL FileNameOut16s
DEL FileNameOut16c
crud = STREAM(FileNameIn,"C","open read")
crud = STREAM(FileNameOut8s,"C","open write")
crud = STREAM(FileNameOut8c,"C","open write")
crud = STREAM(FileNameOut16s,"C","open write")
crud = STREAM(FileNameOut16c,"C","open write")

Do while (STREAM(FileNameIn,"S") = "READY")
  line = LINEIN(FileNameIn);

  SymPos = POS("_SYM",line)
  if (SymPos > 0) Then
  Do
    if POS("inst",line) > 0 Then
    Do
      if (count > 0) Then
      Do
        crud = LINEOUT(FileNameOut8c,"	break;");
        crud = LINEOUT(FileNameOut16c,"	break;");
      End
      newline = "case 0x"SUBSTR(line,SymPos-2,2)":	"SUBSTR(line,SymPos+4)
      crud = LINEOUT(FileNameOut8c, newline);
      crud = LINEOUT(FileNameOut16c, newline);
      count = count + 1;
    End
    Else
    Do
      crud = LINEOUT(FileNameOut8s,SUBSTR(line,1,SymPos)" . "8" . "SUBSTR(line,SymPos+4));
      crud = LINEOUT(FileNameOut16s,SUBSTR(line,1,SymPos)" . "16" . "SUBSTR(line,SymPos+4));
    End
  End
  Else
  Do
    crud = LINEOUT(FileNameOut8c,line);
    crud = LINEOUT(FileNameOut8s,line);
    crud = LINEOUT(FileNameOut16c,line);
    crud = LINEOUT(FileNameOut16s,line);
  End
End
say "Lines read: "Count
crud = STREAM(FileNameIn,"C","close")
crud = STREAM(FileNameOut8s,"C","close")
crud = STREAM(FileNameOut8c,"C","close")
crud = STREAM(FileNameOut16s,"C","close")
crud = STREAM(FileNameOut16c,"C","close")
