# AsyncFileStressTool
Simple filesystem asynchronous access stress tool that listens for file events within a given directory,
and then tries to do various operations on files for which events have been received.

Useful for testing filesystem drivers and applications that should be able to deal with access denied/sharing violation errors.

Should be safe to listen for all events under C:\

Usage:
  AsyncFileStressTool.exe path_to_listen

Directory listening code has been taken from https://github.com/jimbeveridge/readdirectorychanges
