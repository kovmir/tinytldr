# tinytldr

[![tinytldr status](https://builds.sr.ht/~kovmir/tinytldr.svg)](https://builds.sr.ht/~kovmir/tinytldr?)

Minimalist [tldr][1] command line client, written in plain C99.

# PREVIEW

![screenshot](screenshot.png)

*[srcery][2] - terminal colorscheme on the screenshot.*

# INSTALL

## Package

* [Slackware][3]

## Compile from source

Satisfy the [dependencies](#dependencies) first, and then:

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

* [GNU Make][4]
* [pkg-config][5]
* [GCC][6] or [Clang][7]
* [libarchive][8]
* [libcurl][9]

# SUPPORTED OPERATING SYSTEMS

* Linux
* BSD
* M$ Windows

# FAQ

**Q: Can I use it to display my personal pages?**

A: Yes, you can.

```bash
TLDR_PAGES="$HOME/.local/share/tinytldr/pages"
mkdir "$TLDR_PAGES/mypages"
echo '# My custom page' > "$TLDR_PAGES/mypages/testpage.md"
tldr -u
tldr testpage
```

# CREDITS

Thanks [@bilditup1](https://github.com/bilditup1) for Windows support.

# CONTRIBUTING

When submitting PRs, please maintain the [coding style][11] used for the
project.

[1]: https://tldr.sh/
[2]: https://srcery.sh/
[3]: https://slackbuilds.org/repository/15.0/misc/tinytldr/?search=tinytldr
[4]: https://www.gnu.org/software/make/
[5]: https://gitlab.freedesktop.org/pkg-config/pkg-config
[6]: https://gcc.gnu.org/
[7]: https://clang.llvm.org/
[8]: https://www.libarchive.org/
[9]: https://curl.se/libcurl/
[10]: https://github.com/kovmir/tinytldr/issues/6#issuecomment-1884332215
[11]: https://suckless.org/coding_style/
