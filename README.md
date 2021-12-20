# untldr
Minimalist [tldr](https://tldr.sh/) command line client, written in plain C99.

# KILLER FEATURES
* It builds.
* And then works.
* Under 300 lines.
* Depends on nothing but libcurl and libarchive.

# PREVIEW
![untldr screenshot](https://raw.githubusercontent.com/unInstance/untldr/master/screenshot.png)

# QUICK START
Clone this repository and enter it:

```
git clone https://github.com/unInstance/tldr
cd tldr
```

Compile and install tldr:

```
make
sudo make install
```

Run
```
tldr -u
```

To view a page
```
tldr cd
tldr windows/scoop # Or one could specify a platform.
```

# LICENSE
Copyright 2021 Ivan Kovmir

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

# CONTRIBUTING
Contributions are welcome.

When submitting PRs, please maintain the [coding style](https://suckless.org/coding_style/)
used for the project.
