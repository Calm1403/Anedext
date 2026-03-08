# Anedext
The Anedext text editor is an editor for text.

# About
After I thought about it for a little bit, having looked
at the names of other text editors, I decided
to call this program 'Anedext' - it's an abbreviation for
the phrase 'an editor for text.'

This is because it's not a particularly remarkable
editor and because, well, it's just one amongst many.

It's also fun to say 'the Anedext text editor is an
editor for text.'

# State
Currently, it only runs on Linux; compiled initially on
Arch Linux, kernel 6.18.7.

There are numerous problems with this code base; it's horribly
naive, but I'm going to be continually refactoring this code.

# Convention
I'm not the best at this, but in terms of convention I try
to adhere to the [C header file guidelines][1] written by David
Kieras for the university of Michigan; they seem reasonable
enough to this day.

# Compilation
If you're interested in playing around with Anedext, you can compile
the program by simply running 'make' in the root directory where the
makefile is located; this will create the 'Build' directory, where
the Anedext objects and binary reside.

[1]: https://websites.umich.edu/~eecs381/handouts/CHeaderFileGuidelines.pdf
