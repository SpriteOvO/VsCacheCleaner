<h1 align="center">
    <a href="https://github.com/SpriteOvO/VsCacheCleaner"><img src="/Resource/Icon.svg" alt="Icon" width="200"></a>
    <br>
    VsCacheCleaner
</h1>
<p align="center">清理 Visual Studio 解决方案缓存，拯救你的磁盘空间！</p>
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
<p align="center">:earth_americas: <a href="/README.md">English</a> | :earth_asia: 简体中文</p>

## :page_with_curl: 描述
*Visual Studio 解决方案缓存文件夹* `.vs` 用于 *Visual Studio* 存储已打开的文档、断点和其他有关解决方案状态的信息。

通常它们可以安全地被删除。

如果您没有定期手动删除这些缓存，它们就会不断吃掉你磁盘的空闲空间，使磁盘空间条变为红色。:t-rex:

现在，你拥有了这个！

## :mag: 预览
![](/Resource/Preview.png)

## :sparkles: 特性
* 从指定路径或所有磁盘扫描 *Visual Studio 解决方案缓存文件夹* `.vs`。
* 扫描非常快（通常在 1 秒内完成并显示结果）。
* 支持按路径、缓存大小和最后访问日期排序。
* 支持按规则勾选缓存项（最后访问于某段时间前或超过某个大小）。
* 显示缓存大小和磁盘空间大小的统计图表。
* 删除实际是移动到*回收站*，以便您可以做最后确认。
* 多语言支持。

## :hammer_and_wrench: 构建
构建要求 *Visual Studio 2019* 和 [*Qt 5.12.9*](https://download.qt.io/official_releases/qt/5.12/5.12.9/qt-opensource-windows-x86-5.12.9.exe.mirrorlist)。

## :beer: 贡献
*VsCacheCleaner* 是一个开源项目，您可以通过以下方式贡献：
* [打开问题](https://github.com/SpriteOvO/VsCacheCleaner/issues/new/choose)来报告错误或建议新功能。
* [翻译](/CONTRIBUTING.md#earth_africa-translate)尚未支持的语言或[改进](/CONTRIBUTING.md#earth_africa-translate)现有翻译。
* [提交 PR](https://github.com/SpriteOvO/VsCacheCleaner/compare) 来修复已知 BUG 或尝试 TODO 列表中的事项。

查看[贡献准则](/CONTRIBUTING.md)。

## :gem: 第三方
* Qt 5.12.9 ([GPLv3 License](https://doc.qt.io/qt-5/gpl.html))
* Everything ([MIT License](https://www.voidtools.com/License.txt))
* MinHook ([BSD 2-Clause License](https://github.com/TsudaKageyu/minhook/blob/master/LICENSE.txt))

## :rotating_light: 许可
*VsCacheCleaner* 是根据 [MIT License](/LICENSE) 许可的。依赖项根据其各自的许可证许可。

## :warning: 免责声明
*VsCacheCleaner* 不提供任何保证，使用过程中产生的一切意外后果由您自己承担。