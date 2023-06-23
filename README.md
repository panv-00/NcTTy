# NcTTy

This in nakenchat client, for tty.

## Quick Start

```console
$ make
$ ./nctty
```

On startup, nctty will prompt you for a username, then a password. If your username have no password associated, you can leave the password field empty.

By default, messages are formatted in a special way.

- All messages are displayed in default color.
- System messages are displayed in red color.
- Own messages are displayed in blue color.
- Sent private messages are displayed in magenta color.
- Received private messages are displayed in magenta background and black foreground.
- Finally, emote messages are displayed in green background and black foreground.

This app will not break words at the end lines longer than your terminal.
On resize, nctty will recalculate the dimensions of the terminal, and update the display automatically.

## Keyboard

You can use vim-like mode for cursor movement.

### Input Mode

| Key              | Action                         |
| ---------------- | ------------------------------ |
| <kbd>C-c</kbd>   | Switch to Cursor Mode.         |
| <kbd>ENTER</kbd> | Send prompt text.              |
| <kbd>RIGHT</kbd> | Move Cursor Right.             |
| <kbd>LEFT</kbd>  | Move Cursor Left.              |
| <kbd>UP</kbd>    | Scroll Messages Up.            |
| <kbd>DOWN</kbd>  | Scroll to most recent Message. |

### Cursor Mode

| Key          | Action                              |
| ------------ | ----------------------------------- |
| <kbd>i</kbd> | Switch to Insert Mode.              |
| <kbd>l</kbd> | Move Cursor Right.                  |
| <kbd>h</kbd> | Move Cursor Left.                   |
| <kbd>j</kbd> | Scroll Messages Up.                 |
| <kbd>k</kbd> | Scroll to most recent Message.      |
| <kbd>0</kbd> | Move Cursor to beginning of Prompt. |
| <kbd>$</kbd> | Move Cursor to end of Prompt.       |
| <kbd>a</kbd> | Append at location.                 |
| <kbd>A</kbd> | Append at the end of the Prompt.    |
