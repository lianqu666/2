# Multiband Harmonizer (VST3 / Standalone)

一个基于 [JUCE](https://juce.com/) 的多频段和声/激励效果器插件脚手架。

## 效果原理

1. `Source/DSP/BandSplitter.h` 里的 `ThreeBandSplitter` 把输入信号分成
   低 / 中 / 高三个频段（低通 + 高通 + 减法得到中频，保证三段相加严格
   等于原始信号，相位安全）。
2. `Source/DSP/HarmonicShaper.h` 里的 `HarmonicShaper` 对每个频段独立做
   非对称软削波，生成额外的谐波（overtone）。
3. 三个频段的谐波信号叠加成"湿信号"，再和原始"干信号"按 `Mix` 参数
   交叉混合，从而制造出多频段的和声/增厚效果。
4. 最后经过 `Output` 增益输出。

可调参数（在 `Source/PluginProcessor.h` 的 `ParamIDs` 里定义）：

| 参数 | 说明 |
|---|---|
| `crossoverLow` / `crossoverHigh` | 低/中、中/高分频点 (Hz) |
| `lowDrive` / `midDrive` / `highDrive` | 各频段谐波激励强度 |
| `lowGain` / `midGain` / `highGain` | 各频段谐波信号电平 (dB) |
| `mix` | 干/湿混合比例 |
| `outputGain` | 输出总电平 (dB) |

这是一个**原型 / 脚手架**：滤波器是简单的 4 阶 Butterworth，
波形整形是基础的 tanh 软削波。方向没问题，但音质和特性还需要你
根据实际听感继续打磨（比如换成 Linkwitz-Riley 滤波器、加过采样
减少混叠、给每个频段加独立的 pitch-shift 制造真正的"和声"音高等）。

## 目录结构

```
CMakeLists.txt              # 顶层构建脚本，会自动下载 JUCE
Source/
  PluginProcessor.h/.cpp    # 音频处理核心 + 参数定义
  PluginEditor.h/.cpp       # GUI（旋钮界面）
  DSP/
    BandSplitter.h          # 三频段拆分
    HarmonicShaper.h        # 谐波波形整形
.vscode/
  tasks.json                # VS Code 里的 Configure/Build 任务
  settings.json             # CMake Tools 配置
```

## 在 VS Code 里编译运行

### 前置条件

- 安装扩展：**CMake Tools**、**C/C++**（`.vscode/extensions.json` 已推荐）
- 本机需要：CMake ≥ 3.22、一个 C++17 编译器（gcc/clang/MSVC）、
  Linux 下还需要 ALSA/X11/FreeType 等开发库（JUCE 依赖）：

  ```bash
  # Ubuntu/Debian 示例
  sudo apt update
  sudo apt install -y libasound2-dev libjack-jackd2-dev \
      libx11-dev libxcomposite-dev libxcursor-dev libxext-dev \
      libxinerama-dev libxrandr-dev libxrender-dev libfreetype6-dev \
      libfontconfig1-dev libcurl4-openssl-dev
  ```

- 第一次 configure 会自动通过 `FetchContent` 从 GitHub 下载 JUCE 7.0.9，
  需要联网。如果你已经有 JUCE 源码，把它整个放到本项目的 `JUCE/` 目录下
  （即存在 `JUCE/CMakeLists.txt`），`CMakeLists.txt` 会自动优先用本地版本，
  不再联网下载。

### 方式一：用 CMake Tools 侧边栏

1. 打开命令面板 `Ctrl/Cmd+Shift+P` → `CMake: Configure`
2. 选择编译器 kit
3. 点底部状态栏的 Build 按钮，或 `Ctrl/Cmd+Shift+P` → `CMake: Build`

### 方式二：用 `.vscode/tasks.json`

`Ctrl/Cmd+Shift+B` 直接跑默认 build 任务（会先 Configure 再 Build）。

### 方式三：命令行

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
```

编译完成后：

- **Standalone** 可执行文件在 `build/MultibandHarmonizer_artefacts/Debug/Standalone/`
  下，可以直接双击/运行，不需要 DAW，方便先听效果。
- **VST3** 插件在 `build/MultibandHarmonizer_artefacts/Debug/VST3/` 下，
  CMake 配置里 `COPY_PLUGIN_AFTER_BUILD TRUE` 会自动把它复制到系统的
  VST3 插件目录（Windows: `%COMMONPROGRAMFILES%\VST3`；macOS:
  `~/Library/Audio/Plug-Ins/VST3`；Linux: `~/.vst3`），编译完重启一下
  你的 DAW 就能扫描到。

## 不想在本地装编译环境？用 GitHub Actions 云端编译

如果你本地（尤其是 Windows）还没装 Visual Studio / Xcode，没法在 VS Code
里直接编译，可以用 `.github/workflows/build.yml` 这个云端自动构建流程：

1. 把这个工程推送到你自己的 GitHub 仓库（`git init` / `git remote add` /
   `git push` 都可以，只要仓库里有这份 `.github/workflows/build.yml`）。
2. 打开仓库页面顶部的 **Actions** 标签，能看到一个叫 `Build VST3` 的
   workflow 自动跑起来了（push 到 `main`/`master`，或者手动点
   `Run workflow` 触发）。
3. 它会同时起两台云端机器：`windows-latest`（自带 MSVC）和
   `macos-latest`（自带 AppleClang），各自执行：
   ```bash
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release --parallel
   ```
4. 编译成功后，进入对应的 workflow 运行记录页面，最下方 **Artifacts**
   区域会有 `Windows-vst3` / `macOS-vst3`（编译好的 `.vst3` 插件）和
   `Windows-standalone` / `macOS-standalone`（不需要 DAW 就能跑的独立
   程序）几个压缩包，直接点击下载即可，不需要你本机装任何编译器。

注意：第一次跑 workflow 时 CMake 会通过 FetchContent 联网下载 JUCE
7.0.9，耗时会长一点（几分钟），workflow 里配置了针对 `build/_deps`
的缓存（按 `CMakeLists.txt` 内容取 hash 做 key），后续再跑通常会快
不少。如果构建失败，先看 Actions 页面里 "Configure (CMake)" 或
"Build (Release)" 那一步的日志报错信息。

## 后续可以扩展的方向

- 把简单的低/高通换成真正的 Linkwitz-Riley 交叉滤波器，相位/幅度更平滑
- 每个频段加独立的 pitch shifter（比如加 +3/+7/+12 半音），做出真正的
  多声部和声，而不只是谐波激励
- 加过采样（oversampling）降低波形整形产生的混叠噪声
- 用 `juce::dsp::LookAndFeel` 或者自绘控件做更精致的 GUI
