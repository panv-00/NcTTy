# NcTTy

This in nakenchat client, for tty.

## Quick Start

```console
$ make
$ ./nctty
```

On startup, nctty will prompt you for a username, then a password. If your username have no password associated, you can leave the password field empty.

By default, messages are formatted in a special way.
All messages are displayed in default color, system messages in red, own messages in blue, sent private messages in magenta, received private messages in magenta background and black foreground, finally, emote messages are in green background and black foreground.

This app will not break words at the end of long lines.

You can also use vim-like mode for cursor movement.

In normal mode:
Ctrl-C: -> Cursor mode.
Enter: -> sends message.
Arrows Right/Left: -> move cursor right and left.
Arrow Up: -> Go up in history
Arrow Down: -> Display most recent message.

In Cursor mode:
hjkl: -> vim-like cursor movement.
0: -> Beginning of input line.
$: -> End of line.
a: -> append at location.
A: -> append at end of line.
i: -> takes you back to insert mode :)
