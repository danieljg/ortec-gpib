DECLARE FUNCTION OrtecString2$ ()
DECLARE SUB OrtecInit ()
DECLARE SUB OrtecString1 (a$)
DECLARE SUB OrtecOut (Cmd$)
DECLARE FUNCTION StripCount! (NCount!, Dat$)
DECLARE FUNCTION StringCleaner$ (St$)
DECLARE SUB CheckGPIB ()
DECLARE FUNCTION OrtecOutIn2$ (Cmd$)

COMMON SHARED /params/ Ort974%, SR430%, SR620%, LF$, CNT%, Npts

LF$ = CHR$(10) ' LF
CR$ = CHR$(13) ' CR

CLS
CALL CheckGPIB
CALL ibfind("DEV4", Ort974%) ' Find Ortec 974 Counter at Device 04

CALL OrtecInit                   ' Check for power-up string, and initialize
CALL OrtecOut("COMP")            ' Shuts off echo from 974
CALL OrtecOut("EN_REM")          ' Set 974 to remote state
CALL OrtecOut("EN_ALA")          ' Auto-transmit counter data at end of scan
CALL OrtecOut("SET_DISP 2")      ' Display channel 2
CALL OrtecOut("SET_COU_PR 1,1")  ' Count for M*10^N tenths of a second

AvgCount = 0
FOR n = 1 TO 10
 CALL OrtecOut("CL_COU")         ' Clear Counters
 Dat$ = OrtecOutIn2$("STA")      ' Start, and hangs until count period done
 NewCount = StripCount(2, Dat$)
 AvgCount = AvgCount + NewCount
 PRINT n; NewCount; AvgCount / n
NEXT n

CALL OrtecOut("EN_LOC")          ' Leave 974 in local state

END

SUB ChangeSP (n) STATIC

 t0 = TIMER
 SROld% = 99
 DO
  CALL ibrsp(Ortec%, SP%)
  IF SP% <> SPOld% THEN
   PRINT "SP%="; SP%
   SPOld% = SP%
  END IF
  LOOP UNTIL TIMER - t0 > n


END SUB

SUB CheckGPIB

' Checks for an error or timeout on the GPIB.  If either has occurred, a
' message is printed and the program is stopped.
' from Stan's program

  GPIBErr% = (IBSta% AND &H8000) = &H8000
  GPIBTimo% = (IBSta% AND &H4000) = &H4000
  IF GPIBErr% OR GPIBTimo% THEN
    IF GPIBErr% THEN
      PRINT "A GPIB error has occurred: IBErr% ="; IBErr%
      IF IBErr% = 0 THEN PRINT "                           IBCnt% = "; IBCnt%
    END IF
    IF GPIBTimo% THEN
      PRINT "A GPIB timeout has occurred."
    END IF
    PRINT "Refer to the GPIB documentation.  Program terminated."
    STOP
  END IF

END SUB

SUB MonitorSP (Lab$, TSec) STATIC

  t0 = TIMER
  t0b = t0

  CALL ibrsp(Ort974%, SPOld%)
  ByteSP$ = LTRIM$(STR$(SPOld%))

  DO
   CALL ibrsp(Ort974%, SP%)

   IF SP% <> SPOld% THEN
    TNew = TIMER
    Dt = 100 * (TNew - t0b)
    t0b = TNew
    ByteSP$ = ByteSP$ + "[" + LTRIM$(STR$(INT(Dt))) + "]" + "->" + LTRIM$(STR$(SP%))
    SPOld% = SP%
   END IF

  LOOP UNTIL TIMER - t0 > TSec

    TNew = TIMER
    Dt = 100 * (TNew - t0b)
    ByteSP$ = ByteSP$ + "[" + LTRIM$(STR$(INT(Dt))) + "]"

  PRINT "SP%: "; Lab$ + " " + ByteSP$


END SUB

SUB OrtecInit STATIC

 CALL ibrsp(Ort974%, SP%)
 IF SP% = 64 THEN        ' SP% = 64 at power on => clear buffer
  a$ = SPACE$(40)
  CALL ibrd(Ort974%, a$)
  CALL OrtecString1(a$)
 END IF

END SUB

SUB OrtecOut (Cmd$) STATIC
' For commands expecting a %-reply:
 
  CALL IBwrt(Ort974%, Cmd$ + LF$)  'Add terminator <lf> here
  a$ = SPACE$(20): CALL ibrd(Ort974%, a$)
  CALL OrtecString1(a$)
  'PRINT StringCleaner$(a$)

END SUB

FUNCTION OrtecOutIn2$ (Cmd$) STATIC

  CALL IBwrt(Ort974%, Cmd$ + LF$)  'Add terminator <lf> here
  Str1$ = SPACE$(40): CALL ibrd(Ort974%, Str1$)
  Str2$ = SPACE$(40): CALL ibrd(Ort974%, Str2$)
 
 
  ' test for which string leads with %:

  IF LEFT$(Str1$, 1) = "%" THEN        ' Usual case
   'PRINT "Forwards"
   CALL OrtecString1(Str1$)
   Tmp$ = RTRIM$(Str2$)
 
  ELSEIF LEFT$(Str2$, 1) = "%" THEN    ' Sometimes occurs
   BEEP: PRINT "Backwards String Pair"
   CALL OrtecString1(Str2$)
   Tmp$ = RTRIM$(Str1$)
 
  ELSE                                 ' Shouldn't happen
   BEEP: PRINT StringCleaner(Str1$); "  "; StringCleaner(Str2$): STOP
 
  END IF


  'PRINT StringCleaner(Str1$); "  "; StringCleaner(Str2$)
  OrtecOutIn2$ = Tmp$

END FUNCTION

SUB OrtecString1 (a$) STATIC

'To interpret 1st Ortec output string

  SELECT CASE RTRIM$(a$)
 
   CASE "%000000069" + CHR$(10)  ' Usual String
   'PRINT "Ok1"

   CASE "%001000070" + CHR$(10)  ' Startup String
   'PRINT "Ok2"

   CASE "%000001070" + CHR$(10)  ' Sometimes appears
   'PRINT "Ok3"

   CASE ELSE
   BEEP: PRINT StringCleaner$(a$): STOP  ' Anything else

  END SELECT


END SUB

FUNCTION OrtecString2$ STATIC

  a$ = SPACE$(40) ' 40 is sufficient for a full 4-counter read
  CALL ibrd(Ort974%, a$)
  OrtecString2$ = RTRIM$(a$)

END FUNCTION

FUNCTION StringCleaner$ (St$) STATIC

 ST2$ = RTRIM$(St$)
 IF ST2$ = "" THEN
  Tmp$ = ""
 ELSE
  Tmp$ = ""
  FOR n = 1 TO LEN(ST2$)
   c$ = MID$(ST2$, n, 1)
   SELECT CASE c$
    CASE CHR$(13)
    Tmp$ = Tmp$ + "<cr>"

    CASE CHR$(10)
    Tmp$ = Tmp$ + "<lf>"

    CASE ELSE
    Tmp$ = Tmp$ + c$
   END SELECT
  NEXT n
 END IF
 StringCleaner$ = Tmp$

END FUNCTION

FUNCTION StripCount (NCount, Dat$) STATIC
 
  StripCount = VAL(MID$(Dat$, 9 * (NCount - 1) + 1, 8))

END FUNCTION

