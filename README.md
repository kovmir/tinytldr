# tinytldr

Minimalist [tldr](https://tldr.sh/) command line client, written in plain C99.

# KILLER FEATURES

* It builds.
* And then works.
* Small.
* Cross platform, tested on Linux, Windows, and FreeBSD.
* Depends on nothing but libcurl and libarchive.

# PREVIEW

![screenshot](https://raw.githubusercontent.com/kovmir/tinytldr/master/screenshot.png)

# QUICK START

## Install package

* [ArchLinux AUR](https://aur.archlinux.org/packages/untldr)
* [Slackware](https://slackbuilds.org/repository/15.0/misc/untldr/?search=untldr)

## Install from source

Clone this repository and enter it:

```
git clone https://github.com/kovmir/tinytldr
cd tinytldr
```

Compile and install:

```
make
sudo make install
```

Run:

```
tldr -u
```

To view a page
```
tldr cd
tldr windows/scoop # Or one could specify a platform.
```

# LICENSE

Copyright 2023 Ivan Kovmir

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

# CREDITS

Thanks [@bilditup1](https://github.com/bilditup1) for Windows support.

# CONTRIBUTING

Contributions are welcome.

When submitting PRs, please maintain the [coding style](https://suckless.org/coding_style/)
used for the project.
