# tinytldr

Minimalist [tldr](https://tldr.sh/) command line client, written in plain C99.

# KILLER FEATURES

* It builds.
* And then works.
* Small.
* Cross platform, tested on Linux, Windows, and FreeBSD.
* Depends on nothing but libcurl and libarchive.

# PREVIEW

![screenshot](screenshot.png)

# QUICK START

## Install package

* [Slackware](https://slackbuilds.org/repository/15.0/misc/tinytldr/?search=tinytldr)

## Install from source

Clone this repository and enter it:

```bash
git clone https://github.com/kovmir/tinytldr
cd tinytldr
```

Compile and install:

```bash
make
sudo make install
```

Run:

```bash
tldr -u
```

To view a page

```bash
tldr cd
tldr windows/scoop # Or one could specify a platform.
```

# DEPENDENCIES

* [libarchive](https://www.libarchive.org/)
* [libcurl](https://curl.se/libcurl/)
* [pkg-config](https://gitlab.freedesktop.org/pkg-config/pkg-config)

# CREDITS

Thanks [@bilditup1](https://github.com/bilditup1) for Windows support.

# CONTRIBUTING

When submitting PRs, please maintain the [coding
style](https://suckless.org/coding_style/) used for the project.
