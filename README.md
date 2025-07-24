# tinytldr

[![tinytldr status](https://builds.sr.ht/~kovmir/tinytldr.svg)](https://builds.sr.ht/~kovmir/tinytldr?)

Minimalist [tldr][1] command line client, written in plain C99.

# PREVIEW

![screenshot](screenshot.png)

* Never gets in the way.
* Displays a page in under 5 ms.

*[srcery][2] - terminal colorscheme on the screenshot.*

# INSTALL

Satisfy the [dependencies](#dependencies) first, and then:

```bash
git clone https://git.sr.ht/~kovmir/tinytldr
cd tinytldr
# Optional: Adjust ./config.h to your linking.
make
sudo make install
```

# USAGE

```bash
tldr -u # Fetch or update pages.
tldr cd # View 'cd' page.
tldr windows/scoop # One could specify a platform.
tldr common/git-apply
```

This implementation does not and will not support [tldr-pages client
specification][3], as it makes clients identical for no reason and goes against
the minimalist approach of this project.

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
tldr -i
tldr testpage
```

# CREDITS

Thanks [@bilditup1](https://github.com/bilditup1) for Windows support.

# CONTRIBUTING

When submitting PRs, please maintain the [coding style][11] used for the
project.

[1]: https://tldr.sh/
[2]: https://srcery.sh/
[3]: https://github.com/tldr-pages/tldr/blob/main/CLIENT-SPECIFICATION.md
[4]: https://www.gnu.org/software/make/
[5]: https://gitlab.freedesktop.org/pkg-config/pkg-config
[6]: https://gcc.gnu.org/
[7]: https://clang.llvm.org/
[8]: https://www.libarchive.org/
[9]: https://curl.se/libcurl/
[11]: https://suckless.org/coding_style/
