# Necromail

Necromail recovers deleted emails from a Thunderbird mailbox file (Thunderbird's mbox format).

It is not an every day tool, but if you need it (it's probably the case if you've landed here), it gets the job done!
**If you have not _compacted_ your inbox.**

# Features
- Undelete the emails marked as deleted while preserving the others flags (read, replied, forwarded).
- Works with large mbox files (>4Go).
- Handles corrupted files.
- Tested on Linux and Windows.

# Usage

First, backup the mbox file you want to process, better safe than sorry.

Analyse the file (dry run). It should indicate how many emails can be recovered.

`necromail -v file.mbox`

Undelete all the emails marked as deleted (it works directly on the mbox file).

`necromail -r file.mbox`

# How to build
1. git clone https://github.com/HelloWorldo/necromail.git
2. cd necromail
3. gcc main.cpp -o necromail

## Windows (Visual project)
You can build a Microsoft Visual Studio project file with [CMake](https://cmake.org) and the CMakeFile.txt file.

# Credits

This is a one shot utility made for my own purpose, I hope it will be useful to someone else.

Thanks to A.P.Veening and Paolo "Kaosmos" for their tool. The code was started from [this](https://freeshell.de//~kaosmos/misc/index.html).

Documentation about X-Mozilla-Status flags can be found [here](http://www.eyrich-net.org/mozilla/X-Mozilla-Status.html?en).
