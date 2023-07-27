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

*[srcery](https://srcery.sh/) - terminal colorscheme on the screenshot.*

# INSTALL

## Package

* [Slackware](https://slackbuilds.org/repository/15.0/misc/tinytldr/?search=tinytldr)

## Compile from source

```bash
git clone https://github.com/kovmir/tinytldr
cd tinytldr
# Optional: Adjust ./config.h to your linking.
make
sudo make install
```

# USAGE

```bash
tldr -u # Fetch or update pages.
tldr cd # View 'cd' page.
tldr windows/scoop # Or one could specify a platform.
```

# DEPENDENCIES

* [libarchive](https://www.libarchive.org/)
* [libcurl](https://curl.se/libcurl/)
* ^[pkg-config](https://gitlab.freedesktop.org/pkg-config/pkg-config)

*^: compilation time dependencies.*

# CREDITS

Thanks [@bilditup1](https://github.com/bilditup1) for Windows support.

# CONTRIBUTING

When submitting PRs, please maintain the [coding
style](https://suckless.org/coding_style/) used for the project.
