# kilo_textEditor
Text Editor written in C from scratch

Kilo is a small text editor in less than 1K lines of code (counted with cloc).

Usage: <code>kilo <filename></code>

Keys:

CTRL-S: Save
CTRL-Q: Quit
CTRL-F: Find string in file (ESC to exit search, arrows to navigate)
Kilo does not depend on any library (not even curses). It uses fairly standard VT100 (and similar terminals) escape sequences. The project is in alpha stage and was written in just a few hours taking code from my other two projects, load81 and linenoise.

People are encouraged to use it as a starting point to write other editors or command line interfaces that are more advanced than the usual REPL style CLI.

Kilo was written by _Salvatore Sanfilippo_ aka antirez and is released under the BSD 2 clause license.
