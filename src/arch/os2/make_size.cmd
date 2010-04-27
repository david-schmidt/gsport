/*

make_inst.cmd:

Translation of the make_size perl script and makefile to rexx for OS/2

size_c.h: size_tab.h
	$(PERL) make_size c size_tab.h > size_c.h

size_s.h: size_tab.h
	$(PERL) make_size s size_tab.h > size_s.h

8size_s.h: size_tab.h
	$(PERL) make_size 8 size_tab.h > 8size_s.h

16size_s.h: size_tab.h
	$(PERL) make_size 16 size_tab.h > 16size_s.h

*/

FileNameIn = "..\..\size_tab.h"
FileNameOuts = "..\..\size_s.h"
FileNameOutc = "..\..\size_c.h"
FileNameOut8 = "..\..\8size_s.h"
FileNameOut16 = "..\..\16size_s.h"

DEL FileNameOuts
DEL FileNameOutc
DEL FileNameOut8
DEL FileNameOut16
crud = STREAM(FileNameIn,"C","open read")
crud = STREAM(FileNameOuts,"C","open write")
crud = STREAM(FileNameOutc,"C","open write")
crud = STREAM(FileNameOut8,"C","open write")
crud = STREAM(FileNameOut16,"C","open write")

Do while (STREAM(FileNameIn,"S") = "READY")
  line = LINEIN(FileNameIn);

  SymPos = POS("_SYM",line)
  if (SymPos > 0) Then
  Do
    newline = "	.byte 0x"SUBSTR(line,SymPos+5,1)",	/* "SUBSTR(line,SymPos-2,2)" */ "SUBSTR(line,SymPos+6)
    crud = LINEOUT(FileNameOuts, newline);
    newline = "	0x"SUBSTR(line,SymPos+5,1)",	/* "SUBSTR(line,SymPos-2,2)" */ "SUBSTR(line,SymPos+6)
    crud = LINEOUT(FileNameOutc, newline);
    newline = "	.word	inst"SUBSTR(line,SymPos-2,2)" . 8 . 	/*"SUBSTR(line,SymPos+5,1)"*/ "SUBSTR(line,SymPos+6)
    crud = LINEOUT(FileNameOut8, newline);
    newline = "	.word	inst"SUBSTR(line,SymPos-2,2)" . 16 . 	/*"SUBSTR(line,SymPos+5,1)"*/ "SUBSTR(line,SymPos+6)
    crud = LINEOUT(FileNameOut16, newline);
  End
  Else if (POS(".block",line) > 0) Then
  Do
    crud = LINEOUT(FileNameOuts, "");
    crud = LINEOUT(FileNameOutc, "");
    crud = LINEOUT(FileNameOut8, line);
    crud = LINEOUT(FileNameOut16, line);
  End
  Else
  Do
    crud = LINEOUT(FileNameOuts, line);
    crud = LINEOUT(FileNameOutc, line);
    crud = LINEOUT(FileNameOut8, line);
    crud = LINEOUT(FileNameOut16, line);
  End
End
crud = STREAM(FileNameIn,"C","close")
crud = STREAM(FileNameOuts,"C","close")
crud = STREAM(FileNameOutc,"C","close")
crud = STREAM(FileNameOut8,"C","close")
crud = STREAM(FileNameOut16,"C","close")
