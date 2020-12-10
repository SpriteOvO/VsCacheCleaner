<h1 align="center">
    <a href="https://github.com/SpriteOvO/VsCacheCleaner"><img src="/Resource/Icon.svg" alt="Icon" width="200"></a>
    <br>
    VsCacheCleaner
</h1>
<p align="center">Clear the Visual Studio solution cache, free up your disk space!</p>
<p align="center">
    <a href="https://github.com/SpriteOvO/VsCacheCleaner/releases">
        <img src="https://img.shields.io/github/v/release/SpriteOvO/VsCacheCleaner"/>
    </a>
    <a href="https://github.com/SpriteOvO/VsCacheCleaner/compare">
        <img src="https://img.shields.io/badge/PRs-welcome-brightgreen.svg"/>
    </a>
    <a href="/LICENSE">
        <img src="https://img.shields.io/badge/license-MIT-yellow.svg"/>
    </a>
</p>
<p align="center">:earth_americas: English | :earth_asia: <a href="/README-CN.md">简体中文</a></p>

## :page_with_curl: Description
*Visual Studio solution cache folder* `.vs` is required by *Visual Studio* to store opened documents, breakpoints, and other information about state of the solution.

Usually they can be safely deleted.

If you don't delete these caches manually on a regular basis, they will keep eating up the free space of your disk, making the disk space bar turn red. :t-rex:

And now you have this!

## :mag: Preview
![](/Resource/Preview.png)

## :sparkles: Features
* Scan the *Visual Studio solution cache folder* `.vs` from a specified path or all disks.
* Scanning is very fast (usually completed and results displayed within 1sec).
* Support sorting by path, cache size, and last accessed date.
* Support checking cache items by rules (last accessed some time ago or greater than some size).
* Display stats chart for cache size and disk space size.
* Deleting is actually moving to the *Recycle Bin* so that you can make a final confirmation.
* Multi-language support.

## :hammer_and_wrench: Build
Build requirements *Visual Studio 2019* and [*Qt 5.12.9*](https://download.qt.io/official_releases/qt/5.12/5.12.9/qt-opensource-windows-x86-5.12.9.exe.mirrorlist).

## :beer: Contribute
*VsCacheCleaner* is an open source project, here are some ways you can contribute:
* [Open an issue](https://github.com/SpriteOvO/VsCacheCleaner/issues/new/choose) to report bugs or suggest new features.
* [Translate](/CONTRIBUTING.md#earth_africa-translate) unsupported languages or [improve](/CONTRIBUTING.md#earth_africa-translate) existing translations.
* [Submit a PR](https://github.com/SpriteOvO/VsCacheCleaner/compare) to fix a known bug or try something from the TODO list.

See [contribution guidelines](/CONTRIBUTING.md).

## :gem: ThirdParty
* Qt 5.12.9 ([GPLv3 License](https://doc.qt.io/qt-5/gpl.html))
* Everything ([MIT License](https://www.voidtools.com/License.txt))
* MinHook ([BSD 2-Clause License](https://github.com/TsudaKageyu/minhook/blob/master/LICENSE.txt))

## :rotating_light: License
*VsCacheCleaner* is licensed under the [MIT License](/LICENSE). Dependencies are licensed under their respective licenses.

## :warning: Disclaimer
*VsCacheCleaner* makes no warranties, and you are solely responsible for any unintended consequences of your use.