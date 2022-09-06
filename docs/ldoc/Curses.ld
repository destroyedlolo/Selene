
project = 'Selene Curses plug-in'
title = 'Selene text based plug-in'
dir = 'docs/Curses'
description = "Selene's Curses plug-in"
full_description = [[
This plug-in provides an easy way to implement a text based graphics to Séléné / Lua application.

[Back to top level documentation](../)

----
**Notez-bien :** 

  - Unlike graphical windows manager, Curses is somewhat limited : as example, it doesn't support layers meaning if you're creating a sub windows, parent window's content is lost.
  - This plug-in implements only a subset of Curses capabilities : in fact, only the ones I'm using to build semi-graphical interface to my smart home automation.
  - Lowercase methods try to follow the Curses specification. Those with capital are extensions.
The curses' very own functions take the ordinates Y then the abscissas X. Selene takes the natural order X then Y.

]]
use_markdown_titles = true
format = "markdown"

add_language_extension("sel", "lua")
examples = "./SelenitesCurses"
