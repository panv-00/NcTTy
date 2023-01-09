# NcRev
Yet, another nakenchat client: Nakenchat in Reverse.

This is another nakenchat terminal client, with minimal libraries requirements, written in C++.

Terminal should support ANSI escape codes.

## Quick Start

```console
$ make
$ ./ncrev [username]
     (or)
$ ./ncrev [username]=[password]
```

## Commands

| Command | Action                                     |
|---------|--------------------------------------------|
| :q      | Quit                                       |
| /quit   | Quit                                       |
| .q      | Quit                                       |
| .n      | Change name                                |
| .w      | List users                                 |
| .p [n]  | Send private message to user #[n]          |
| .e      | Enable/disable echoing of private messages |
| .d      | Enable/disable Timestamps                  |
| %       | Emote a message                            |


## Special Features

- Great memory handling
- Own messages in different color
- Private messages in different color/style
- Emote messages in different color/style
