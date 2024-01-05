# NcTTy

This in `nakenchat` client, for `tty`.

## Quick Start

```console
$ make
  ...

$ ./nctty
  ...
```

On startup, `nctty` will prompt you for a username, then a password. If your
username have no password associated, you can leave the password field empty.

By default, messages are formatted in a special way.

- All messages are displayed in default color.
- System messages are displayed in red color.
- Own messages are displayed in blue color.
- Sent private messages are displayed in magenta color.
- Received private messages are displayed in magenta background and black foreground.
- Finally, emote messages are displayed in green background and black foreground.

This app will not break words at the end lines longer than your terminal.
On resize, `nctty` will recalculate the dimensions of the terminal, and update
the display automatically.

## Keyboard

You can use vim-like mode for cursor movement.

### Input Mode

| Key     | Action                         |
| ------- | ------------------------------ |
| `C-c`   | Switch to Cursor Mode.         |
| `ENTER` | Send prompt text.              |
| `RIGHT` | Move Cursor Right.             |
| `LEFT`  | Move Cursor Left.              |
| `UP`    | Scroll Messages Up.            |
| `DOWN`  | Scroll to most recent Message. |

### Cursor Mode

| Key | Action                              |
| --- | ----------------------------------- |
| `i` | Switch to Insert Mode.              |
| `l` | Move Cursor Right.                  |
| `h` | Move Cursor Left.                   |
| `j` | Scroll Messages Up.                 |
| `k` | Scroll to most recent Message.      |
| `0` | Move Cursor to beginning of Prompt. |
| `$` | Move Cursor to end of Prompt.       |
| `a` | Append at location.                 |
| `A` | Append at the end of the Prompt.    |
