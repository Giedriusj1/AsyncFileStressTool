# AsyncFileStressTool
Simple filesystem asynchronous access stress tool that listens for file events within a given directory,
and then tries to do various operations on files for which events have been received.

Usage:
  AsyncFileStressTool.exe path_to_listen

Directory listening code has been taken from https://github.com/jimbeveridge/readdirectorychanges
