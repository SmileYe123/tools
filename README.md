# MultiTool 多功能工具箱

一款基于 Qt 框架开发的多功能桌面工具箱，采用插件式架构，提供多种实用工具，界面简洁美观，支持浅色/深色主题切换。

## 功能特性

### 已集成的工具

| 工具 | 图标 | 分类 | 功能描述 |
|------|------|------|----------|
| **颜色选择** | 🎨 | ui | 屏幕取色和颜色格式转换（HEX/RGB/HSL） |
| **CSV工具** | 📊 | data | CSV 文件加载、预览和转 JSON |
| **哈希计算** | 🔐 | security | 计算 MD5、SHA1、SHA256、SHA512 哈希值 |
| **图片工具** | 🖼️ | media | 图片压缩和格式转换（PNG/JPG/WEBP/BMP） |
| **JSON格式化** | 📋 | text | JSON 格式化、压缩、验证，支持自定义缩进 |
| **正则工具** | 🔍 | text | 正则表达式测试、匹配和替换 |
| **SQL格式化** | 🗄️ | data | SQL 语句格式化和压缩，支持自定义缩进 |
| **文本工具** | 📝 | text | 大小写转换、去空格、去空行、反转、统计、排序 |
| **时间戳工具** | ⏰ | text | Unix 时间戳与日期互转，实时显示当前时间 |
| **上传工具** | 🚀 | system | 发布新版本到自动更新服务器，含项目管理 |

### 界面特性

- 🎨 **主题切换**：支持浅色、深色、海洋蓝、森林绿四种主题
- 🔍 **工具搜索**：侧边栏支持按名称搜索过滤工具
- 🏠 **快捷卡片**：欢迎页提供常用工具快捷入口，点击即可跳转
- 🔤 **中文界面**：所有界面元素已全面中文化
- 📏 **舒适字体**：全局 14px 字体，阅读体验更佳
- 📱 **响应式布局**：自适应窗口大小调整

## 技术栈

- **框架**：Qt 5/6（core、gui、widgets 模块）
- **语言**：C++11
- **构建工具**：qmake
- **样式**：QSS（Qt StyleSheet）

## 项目结构

```
cpp/
├── src/
│   ├── main.cpp                # 程序入口
│   ├── core/                   # 核心模块
│   │   ├── config.h/cpp        # 配置管理（单例，QSettings）
│   │   ├── main_window.h/cpp   # 主窗口
│   │   └── plugin_manager.h/cpp # 插件管理器
│   ├── ui/                     # UI 模块
│   │   ├── sidebar.h/cpp       # 侧边栏导航（含搜索）
│   │   ├── theme.h/cpp         # 主题管理器
│   │   └── themes/             # QSS 主题样式文件
│   │       ├── light.qss       # 浅色主题
│   │       └── dark.qss        # 深色主题
│   └── plugins/                # 插件目录
│       ├── base.h              # 插件基类 BasePlugin
│       ├── color_picker/       # 颜色选择器
│       ├── csv_tool/           # CSV 工具
│       ├── hash_tool/          # 哈希计算
│       ├── image_tool/         # 图片工具
│       ├── json_formatter/     # JSON 格式化
│       ├── regex_tool/         # 正则工具
│       ├── sql_formatter/      # SQL 格式化
│       ├── text_tools/         # 文本工具
│       ├── timestamp_tool/     # 时间戳工具
│       └── upload_tool/        # 上传工具
│           ├── upload_tool.h/cpp
│           ├── upload_config.h/cpp
│           └── project_manage_dialog.h/cpp
├── resources.qrc               # Qt 资源文件
└── MultiTool.pro               # qmake 构建配置
```

## 构建指南

### 使用 qmake

```bash
cd cpp
qmake MultiTool.pro
make  # 或 mingw32-make（Windows）
```

## 运行要求

- Qt 5.12+ 或 Qt 6.x
- C++11 或更高版本编译器
- 支持的操作系统：Windows、Linux、macOS

## 主题系统

主题样式统一使用 QSS 文件管理，位于 `src/ui/themes/` 目录：

- **light.qss**：浅色主题（白色背景，适合日间使用）
- **dark.qss**：深色主题（暗色背景，适合夜间使用）

海洋蓝和森林绿主题复用 dark.qss 基础样式，通过 Theme 类中的颜色变量实现差异化配色。主题切换通过 `Theme` 类动态加载对应的 QSS 文件实现。

## 插件开发

所有工具均以插件形式实现，继承自 `BasePlugin` 基类：

```cpp
class MyPlugin : public BasePlugin
{
    Q_OBJECT

public:
    MyPlugin(QObject* parent = nullptr) : BasePlugin(parent)
    {
        setName(tr("我的工具"));
        setIcon("🔧");
        setDescription(tr("工具描述"));
        setCategory("category");
    }

    QWidget* createWidget(QWidget* parent = nullptr) override
    {
        // 创建并返回工具界面 Widget
    }
};
```

新增插件后，在 `PluginManager::loadPlugins()` 中注册即可。

## 开源协议

本项目采用 MIT 协议开源。
