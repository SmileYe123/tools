# MultiTool 多功能工具箱

一款基于 Qt 框架开发的多功能桌面工具箱，提供多种实用工具，界面简洁美观，支持浅色/深色主题切换。

## 功能特性

### 已集成的工具

| 工具 | 图标 | 功能描述 |
|------|------|----------|
| **颜色选择** | 🎨 | 屏幕取色和颜色格式转换（HEX/RGB/HSL） |
| **JSON格式化** | 📋 | JSON 格式化、压缩、验证 |
| **文本工具** | 📝 | 文本格式转换、大小写转换、统计等 |
| **CSV工具** | 📊 | CSV 文件处理和转换（JSON/XML） |
| **哈希计算** | 🔐 | 计算 MD5、SHA1、SHA256 哈希值 |
| **图片工具** | 🖼️ | 图片压缩和格式转换 |
| **正则工具** | 🔍 | 正则表达式测试、匹配和替换 |
| **SQL格式化** | 🗄️ | SQL 语句格式化和美化 |
| **时间戳工具** | ⏰ | 时间戳和日期时间转换 |
| **密码生成** | 🔑 | 随机密码生成、强度检测、批量生成 |

### 界面特性

- 🎨 **主题切换**：支持浅色、深色、海洋蓝、森林绿四种主题
- 📐 **白色背景**：默认浅色主题采用专业白色背景设计
- 🔤 **中文界面**：所有界面元素已全面中文化
- 📏 **舒适字体**：全局 14px 字体，阅读体验更佳
- 📱 **响应式布局**：自适应窗口大小调整

## 技术栈

- **框架**：Qt 5/6
- **语言**：C++
- **构建工具**：CMake / qmake
- **样式**：QSS（Qt StyleSheet）

## 项目结构

```
cpp/
├── src/
│   ├── core/           # 核心模块（主窗口、插件管理器）
│   ├── ui/             # UI 模块（侧边栏、主题管理、QSS 主题文件）
│   └── plugins/        # 插件目录
│       ├── base.h              # 插件基类
│       ├── color_picker/       # 颜色选择器
│       ├── csv_tool/           # CSV 工具
│       ├── hash_tool/          # 哈希计算
│       ├── image_tool/         # 图片工具
│       ├── json_formatter/     # JSON 格式化
│       ├── password_generator/ # 密码生成器
│       ├── regex_tool/         # 正则工具
│       ├── sql_formatter/      # SQL 格式化
│       ├── text_tools/         # 文本工具
│       └── timestamp_tool/     # 时间戳工具
├── src/ui/themes/      # QSS 主题文件
│   ├── light.qss       # 浅色主题
│   └── dark.qss        # 深色主题
├── resources.qrc       # Qt 资源文件
├── CMakeLists.txt      # CMake 构建配置
└── MultiTool.pro       # qmake 构建配置
```

## 构建指南

### 使用 CMake

```bash
cd cpp
mkdir build && cd build
cmake ..
cmake --build .
```

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

主题切换通过 `Theme` 类动态加载对应的 QSS 文件实现。

## 插件开发

所有工具均以插件形式实现，继承自 `BasePlugin` 基类：

```cpp
class MyPlugin : public BasePlugin
{
public:
    MyPlugin()
    {
        m_name = tr("我的工具");
        m_icon = "🔧";
        m_description = tr("工具描述");
        m_category = "category";
    }

    QWidget* createWidget(QWidget* parent = nullptr) override
    {
        // 实现工具界面
    }
};
```

## 开源协议

本项目采用 MIT 协议开源。
