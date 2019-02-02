# binpretty

Binpretty is a small tool to view the content of mixed binary/plaintext files.

![binpretty showing a tar archive of this repository](https://raw.githubusercontent.com/mortie/binpretty/master/img/screenshot-1.png)
> binpretty showing a tar archive of this repository

## Rationale

It turns out a lot of binary formats are a mix of plaintext and binary; the
binary means regular text editors aren't great at viewing them, but reading the
plaintext in the small ascii representation on the right hand side of hex
viewers isn't very nice either.

## Usage

	binpretty [file]...

You may want to pipe the output to `less -R`.

## Install

Just `git clone` this repo and run `make && sudo make install`. There are no
dependencies other than regular stdlib/posix stuff.
