library hello;
 
uses
  unixtype;
 
// use the C function 'write'
function CWrite(fd : cInt; buf:pChar; nbytes : unixtype.TSize): TSsize;  external name 'write';
 
// start function for OSv
function main: longint; cdecl;
const
  MyText: PChar = 'It works!';
begin
  CWrite(StdOutputHandle,MyText,strlen(MyText));
  main:=0;
end;
 
exports main name 'main'; // OSv searches for 'main' in the library
 
end.
