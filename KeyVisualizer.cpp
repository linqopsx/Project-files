// ==================== 头文件包含 ====================
#include <windows.h>      // Windows API 核心头文件，提供窗口、绘图、钩子等功能
#include <string>         // C++ 字符串类 std::string
#include <vector>         // C++ 动态数组容器 std::vector

// ==================== 主题颜色结构体 ====================
struct ThemeColors {
    std::string name;           // 主题名称（用于显示）
    COLORREF keyBgNormal;       // 按键正常状态下的背景颜色（RGB值）                            
    COLORREF keyBgPressed;      // 按键按下状态下的背景颜色
    COLORREF keyBorderNormal;   // 按键正常状态下的边框颜色
    COLORREF keyBorderPressed;  // 按键按下状态下的边框颜色
    COLORREF textNormal;        // 按键文字正常状态颜色
    COLORREF textPressed;       // 按键文字按下状态颜色
    COLORREF windowBg;          // 整个窗口背景颜色
}; 
// COLORREF 是 Windows 的 32 位颜色类型，格式为 0x00BBGGRR，通常用 RGB(r,g,b) 宏创建

// ==================== 预设主题数组 ====================
// 全局数组，存储 6 套预设主题配置，通过索引 0-5 访问
ThemeColors g_themes[] = {
    // 主题 0: 经典灰 - 浅灰背景 + 蓝色高亮
    { "Classic", RGB(240,240,240), RGB(100,149,237), RGB(160,160,160), RGB(65,105,225), RGB(0,0,0), RGB(255,255,255), RGB(30,30,30) },

    // 主题 1: 黑客绿 - 深色背景 + 荧光绿文字（终端风格）
    { "Hacker", RGB(30,30,30), RGB(0,255,0), RGB(60,60,60), RGB(0,128,0), RGB(0,255,0), RGB(0,0,0), RGB(10,10,10) },

    // 主题 2: 游戏红 - 暗黑背景 + 红色高亮（电竞风格）
    { "Gaming", RGB(20,20,20), RGB(255,0,0), RGB(50,50,50), RGB(139,0,0), RGB(255,50,50), RGB(255,255,255), RGB(15,15,15) },

    // 主题 3: 赛博紫 - 深紫背景 + 霓虹配色（赛博朋克风格）
    { "Cyberpunk", RGB(15,15,35), RGB(180,0,255), RGB(40,20,60), RGB(128,0,128), RGB(0,255,255), RGB(255,255,255), RGB(5,5,20) },

    // 主题 4: 苹果白 - 白色简约风格（macOS 风格）
    { "Apple", RGB(255,255,255), RGB(200,200,200), RGB(220,220,220), RGB(150,150,150), RGB(50,50,50), RGB(30,30,30), RGB(245,245,245) },

    // 主题 5: 深海蓝 - 蓝色渐变风格（海洋主题）
    { "Ocean", RGB(20,40,60), RGB(0,150,255), RGB(40,80,120), RGB(0,100,200), RGB(150,200,255), RGB(255,255,255), RGB(10,30,50) }
};

// 当前使用的主题索引（0-5），默认使用第一个主题
int g_currentTheme = 0;

// ==================== 按键结构体 ====================
struct Key {
    int vkCode;          // 虚拟键码（Windows 定义的按键标识，如 VK_SPACE=32）
    std::string label;   // 按键上显示的文字（如 "A", "Enter", "Shift"）
    int x, y;            // 按键左上角在窗口中的坐标（像素）
    int width, height;   // 按键的宽度和高度（像素）
    bool isPressed;      // 标记：该按键当前是否处于按下状态
};

// ==================== 全局变量声明 ====================
HHOOK g_hHook = NULL;           // 存储键盘钩子的句柄，用于后续卸载
HWND g_hWnd = NULL;             // 存储程序创建的窗口句柄，用于绘图和消息发送
std::vector<Key> g_keys;        // 动态数组，存储所有按键的布局信息
bool g_running = true;          // 程序运行标志，设为 false 时退出主循环
// 🎯 新增：窗口尺寸全局变量（供双缓冲使用）
int g_windowWidth = 740;        // 窗口宽度（像素）
int g_windowHeight = 350;       // 窗口高度（像素）
// ==================== 函数：初始化键盘布局 ====================
void InitKeyboardLayout() {
    float scale = 0.9f;  // 缩放系数：0.7 表示按键大小为原始的 70%

    // 计算单个按键的基础尺寸（按缩放比例）
    int keyWidth = (int)(45 * scale);      // 标准按键宽度
    int keyHeight = (int)(45 * scale);     // 标准按键高度
    int gap = (int)(4 * scale);            // 按键之间的间距
    int startX = 8;                        // 第一列按键的起始 X 坐标（左边距）
    int startY = 8;                        // 第一行按键的起始 Y 坐标（上边距）

    g_keys.clear();  // 清空之前的按键数据，防止重复添加

    // --- 第一排：数字键区域（共 14 个键）---
    int row1[] = { 192, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 189, 187, 8 };  // 虚拟键码数组
    std::string row1Labels[] = { "`", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "Back" };  // 对应显示文字

    for (int i = 0; i < 14; i++) {  // 遍历 14 个按键
        Key key;  // 创建临时按键对象
        key.vkCode = row1[i];  // 设置键码
        key.label = row1Labels[i];  // 设置显示文字
        key.x = startX + i * (keyWidth + gap);  // 横向等间距排列计算 X 坐标
        key.y = startY;  // 第一排 Y 坐标固定
        // 特殊处理：Backspace 键宽度加倍
        key.width = (row1[i] == 8) ? (keyWidth * 2 + gap) : keyWidth;
        key.height = keyHeight;  // 高度统一
        key.isPressed = false;   // 初始状态为未按下
        g_keys.push_back(key);   // 加入全局按键数组
    }

    // --- 第二排：QWERTY 字母区（共 14 个键）---
    int row2[] = { 9, 81, 87, 69, 82, 84, 89, 85, 73, 79, 80, 219, 221, 220 };  // Tab, Q-W-E-R-T-Y-U-I-O-P, [, ], 
        std::string row2Labels[] = { "Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "\\" };

    for (int i = 0; i < 14; i++) {
        Key key;
        key.vkCode = row2[i];
        key.label = row2Labels[i];
        key.x = startX + i * (keyWidth + gap);  // 横向排列
        key.y = startY + (keyHeight + gap) * 1;  // 第二排：Y 坐标下移一行
        // 特殊处理：Tab 键宽度为 1.5 倍
        key.width = (row2[i] == 9) ? (int)(keyWidth * 1.5) : keyWidth;
        key.height = keyHeight;
        key.isPressed = false;
        g_keys.push_back(key);
    }

    // --- 第三排：ASDF 字母区（共 13 个键）---
    int row3[] = { 20, 65, 83, 68, 70, 71, 72, 74, 75, 76, 186, 222, 13 };  // Caps, A-S-D-F-G-H-J-K-L, ;, ', Enter
    std::string row3Labels[] = { "Caps", "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "Enter" };

    for (int i = 0; i < 13; i++) {
        Key key;
        key.vkCode = row3[i];
        key.label = row3Labels[i];
        key.x = startX + i * (keyWidth + gap);
        key.y = startY + (keyHeight + gap) * 2;  // 第三排：Y 坐标下移两行
        key.width = keyWidth;  // 默认宽度
        // 特殊处理：CapsLock 宽 1.75 倍，Enter 宽 2 倍
        if (row3[i] == 20) key.width = (keyWidth * 1.75);
        if (row3[i] == 13) key.width = (keyWidth * 2);
        key.height = keyHeight;
        key.isPressed = false;
        g_keys.push_back(key);
    }

    // --- 第四排：ZXCV 字母区 + Shift（共 12 个键）---
    int row4[] = { 16, 90, 88, 67, 86, 66, 78, 77, 188, 190, 191, 16 };  // Shift, Z-X-C-V-B-N-M, ,, ., /, Shift
    std::string row4Labels[] = { "Shift", "Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", "Shift" };

    for (int i = 0; i < 12; i++) {
        Key key;
        key.vkCode = row4[i];
        key.label = row4Labels[i];
        key.y = startY + (keyHeight + gap) * 3;  // 第四排：Y 坐标下移三行
        key.width = keyWidth;
        // 特殊处理：左右 Shift 键位置和宽度不同
        if (row4[i] == 16) {
            if (i == 0) {  // 左侧 Shift
                key.x = startX;
                key.width = (int)(keyWidth * 2.25);
            }
            else {  // 右侧 Shift（i==11）
                key.x = startX + (keyWidth + gap) * 10;  // 跳到右侧位置
                key.width = (int)(keyWidth * 2.75);
            }
        }
        else {
            key.x = startX + i * (keyWidth + gap);  // 普通按键等间距排列
        }
        key.height = keyHeight;
        key.isPressed = false;
        g_keys.push_back(key);
    }

    // --- 第五排：控制键 + 空格键 ---
    int ctrlWidth = (int)(50 * scale);    // 控制键（Ctrl/Alt/Win）宽度
    int spaceWidth = (int)(250 * scale);  // 空格键宽度（较宽）

    // 创建左 Ctrl 键
    Key ctrlLeft = { VK_CONTROL, "Ctrl", startX, startY + (keyHeight + gap) * 4, ctrlWidth, keyHeight, false };
    g_keys.push_back(ctrlLeft);

    // 创建左 Win 键（紧接 Ctrl 右侧）
    Key winLeft = { VK_LWIN, "Win", startX + ctrlWidth + gap, startY + (keyHeight + gap) * 4, ctrlWidth, keyHeight, false };
    g_keys.push_back(winLeft);

    // 创建左 Alt 键
    Key altLeft = { VK_LMENU, "Alt", startX + (ctrlWidth + gap) * 2, startY + (keyHeight + gap) * 4, ctrlWidth, keyHeight, false };
    g_keys.push_back(altLeft);

    // 创建空格键（居中，较宽）
    int spaceX = startX + (ctrlWidth + gap) * 3;  // 空格键起始 X 坐标
    Key spaceKey = { VK_SPACE, "Space", spaceX, startY + (keyHeight + gap) * 4, spaceWidth, keyHeight, false };
    g_keys.push_back(spaceKey);

    // 创建右侧按键组（从空格键右侧开始排列）
    int rightStartX = spaceX + spaceWidth + gap;  // 右侧区域起始 X 坐标

    Key altRight = { VK_RMENU, "Alt", rightStartX, startY + (keyHeight + gap) * 4, ctrlWidth, keyHeight, false };
    g_keys.push_back(altRight);

    Key winRight = { VK_RWIN, "Win", rightStartX + ctrlWidth + gap, startY + (keyHeight + gap) * 4, ctrlWidth, keyHeight, false };
    g_keys.push_back(winRight);

    Key ctrlRight = { VK_RCONTROL, "Ctrl", rightStartX + (ctrlWidth + gap) * 2, startY + (keyHeight + gap) * 4, ctrlWidth, keyHeight, false };
    g_keys.push_back(ctrlRight);
}

// ==================== 函数：根据键码查找按键 ====================
Key* FindKeyByVkCode(int vkCode) {
    // 遍历全局按键数组
    for (auto& key : g_keys) {  // C++11 范围 for 循环，key 是引用避免拷贝
        if (key.vkCode == vkCode) {  // 匹配键码
            return &key;  // 返回该按键的地址（指针），便于修改状态
        }
    }
    return nullptr;  // 未找到返回空指针
}

// ==================== 函数：绘制单个按键 ====================
void DrawKey(HDC hdc, const Key& key) {
    // 获取当前主题配置（引用避免拷贝）
    ThemeColors& theme = g_themes[g_currentTheme];

    // 根据按键状态选择对应颜色
    COLORREF bgColor, borderColor, textColor;
    if (key.isPressed) {  // 按下状态
        bgColor = theme.keyBgPressed;
        borderColor = theme.keyBorderPressed;
        textColor = theme.textPressed;
    }
    else {  // 正常状态
        bgColor = theme.keyBgNormal;
        borderColor = theme.keyBorderNormal;
        textColor = theme.textNormal;
    }

    // 创建 GDI 绘图对象：画刷（填充）和画笔（边框）
    HBRUSH hBrush = CreateSolidBrush(bgColor);  // 创建纯色画刷
    HPEN hPen = CreatePen(PS_SOLID, 2, borderColor);  // 创建 2 像素宽的实线画笔

    // 将绘图对象选入设备上下文（HDC）
    SelectObject(hdc, hBrush);
    SelectObject(hdc, hPen);

    // 绘制按键矩形边框+填充
    Rectangle(hdc, key.x, key.y, key.x + key.width, key.y + key.height);

    // 设置文字绘制属性：透明背景 + 指定颜色
    SetBkMode(hdc, TRANSPARENT);  // 文字背景透明，不覆盖按键背景
    SetTextColor(hdc, textColor); // 设置文字颜色

    // --- 动态字体大小逻辑 ---
    int labelLen = (int)key.label.length();  // 获取文字长度（本变量未实际使用）
    int fontSize = 10;  // 默认字体大小
    int keyWidth = 10;  // ⚠️ 注意：此变量与外层 keyWidth 重名，实际比较逻辑可能有问题
    // 宽按键用稍大字体
    if (key.width >= keyWidth * 2) {
        fontSize = 11;
    }
    // 窄按键用小字体
    else if (key.width < keyWidth * 1.5) {
        fontSize = 9;
    }

    // 创建字体对象（使用微软雅黑）
    HFONT hFont = CreateFontA(fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Microsoft YaHei");
    SelectObject(hdc, hFont);  // 选入字体

    // --- 文字居中计算 ---
    SIZE textSize;  // 存储文字宽高
    // 获取文字像素尺寸
    GetTextExtentPoint32A(hdc, key.label.c_str(), (int)key.label.length(), &textSize);

    // 计算居中坐标：按键中心 - 文字半宽
    int textX = key.x + (key.width - textSize.cx) / 2;
    int textY = key.y + (key.height - textSize.cy) / 2;

    // 边界保护：防止文字贴边或溢出
    if (textX < key.x + 2) textX = key.x + 2;  // 最小左边距 2 像素
    if (textY < key.y + 2) textY = key.y + 2;  // 最小上边距 2 像素

    // 在计算好的位置输出文字
    TextOutA(hdc, textX, textY, key.label.c_str(), (int)key.label.length());

    // --- 资源清理（防止 GDI 泄漏）---
    DeleteObject(hFont);   // 删除字体
    DeleteObject(hBrush);  // 删除画刷
    DeleteObject(hPen);    // 删除画笔
}

// ==================== 函数：键盘钩子回调（核心逻辑）====================
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    // nCode >= 0 表示可以处理该消息
    if (nCode >= 0) {
        // 将 lParam 转换为键盘事件结构体指针
        KBDLLHOOKSTRUCT* pKey = (KBDLLHOOKSTRUCT*)lParam;

        // --- 处理按键按下事件 ---
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {

            // 🔑 退出快捷键：Ctrl + P（注意代码中是 P）
            if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && pKey->vkCode == 'P') {
                // 0x8000 表示按键当前处于按下状态
                g_running = false;        // 设置退出标志
                PostQuitMessage(0);       // 向消息队列发送退出消息
                return 1;                 // 拦截该按键，不传递给其他程序
            }

            // 🔑 主题切换快捷键：F1 ~ F6
            if (pKey->vkCode >= VK_F1 && pKey->vkCode <= VK_F6) {
                g_currentTheme = pKey->vkCode - VK_F1;  // F1→0, F2→1, ..., F6→5
                InvalidateRect(g_hWnd, NULL, TRUE);     // 标记窗口区域无效，触发 WM_PAINT 重绘
                return 1;  // 拦截 F 键，防止在游戏中误触发
            }

            // 普通按键：查找并更新按下状态
            Key* key = FindKeyByVkCode(pKey->vkCode);
            if (key) {  // 找到对应按键
                key->isPressed = true;              // 标记为按下
                InvalidateRect(g_hWnd, NULL, TRUE); // 触发重绘，显示按下效果
            }
        }
        // --- 处理按键释放事件 ---
        else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            Key* key = FindKeyByVkCode(pKey->vkCode);
            if (key) {
                key->isPressed = false;             // 标记为释放
                InvalidateRect(g_hWnd, NULL, TRUE); // 触发重绘，恢复正常样式
            }
        }
    }
    // 调用下一个钩子（保证系统和其他钩子能正常接收消息）
    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

// ==================== 函数：安装全局键盘钩子 ====================
void InstallHook() {
    // 设置低级键盘钩子（WH_KEYBOARD_LL）
    // 参数：钩子类型、回调函数、模块句柄（0 表示当前进程）、线程 ID（0 表示全局）
    g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, GetModuleHandle(NULL), 0);

    // 检查是否安装成功
    if (!g_hHook) {
        MessageBoxA(NULL, "安装钩子失败！", "错误", MB_ICONERROR);  // 弹出错误提示
    }
}

// ==================== 函数：卸载钩子 ====================
void UninstallHook() {
    if (g_hHook) {  // 如果钩子存在
        UnhookWindowsHookEx(g_hHook);  // 卸载钩子，释放系统资源
        g_hHook = NULL;  // 避免野指针
    }
}

// ==================== 函数：窗口消息处理 ====================
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {  // 根据消息类型分支处理

        // --- 绘图消息：窗口需要重绘时触发 ---
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // 🎯 双缓冲：创建内存DC和位图（使用全局尺寸）
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP hBitmap = CreateCompatibleBitmap(hdc, g_windowWidth, g_windowHeight);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

        // 🎯 填充整个内存位图背景（避免透明区域闪烁）
        ThemeColors& theme = g_themes[g_currentTheme];
        HBRUSH hBgBrush = CreateSolidBrush(theme.windowBg);
        FillRect(memDC, &ps.rcPaint, hBgBrush);  
        DeleteObject(hBgBrush);

        // 🎯 绘制所有按键（传入 memDC）
        for (const auto& key : g_keys) {
            DrawKey(memDC, key);
        }

        // 🎯 绘制文字信息（传入 memDC）
        SetBkMode(memDC, TRANSPARENT);
        SetTextColor(memDC, RGB(255, 255, 255));
        HFONT hTitleFont = CreateFontA(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Microsoft YaHei");
        HFONT hOldFont = (HFONT)SelectObject(memDC, hTitleFont);

        std::string themeInfo = "Theme: " + theme.name + " (F1-F6 切换)";
        TextOutA(memDC, 10, 220, themeInfo.c_str(), (int)themeInfo.length());
        TextOutA(memDC, 10, 230, "F1-F6: Switch Theme | Ctrl+P: Exit", 38);

        SelectObject(memDC, hOldFont);
        DeleteObject(hTitleFont);

        // 🎯 关键：将内存DC内容一次性拷贝到屏幕
        BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top,
            ps.rcPaint.right - ps.rcPaint.left,
            ps.rcPaint.bottom - ps.rcPaint.top,
            memDC, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);

        // 🎯 清理资源
        SelectObject(memDC, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(memDC);

        EndPaint(hWnd, &ps);
        break;
    }

                 // --- 窗口销毁消息 ---
    case WM_DESTROY:
        PostQuitMessage(0);  // 发送退出消息，结束消息循环
        break;

        // --- 其他消息交给系统默认处理 ---
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;  // 消息已处理
}

// ==================== 函数：创建透明置顶窗口 ====================
void CreateOverlayWindow(HINSTANCE hInstance) {
    float scale = 0.9f;  // 缩放系数，与键盘布局保持一致

    // 计算缩放后的窗口尺寸
    int windowWidth = (int)(740 * scale);   // 原始宽度 740px → 缩放后
    int windowHeight = (int)(350 * scale);  // 原始高度 350px → 缩放后

    g_windowWidth = windowWidth;
    g_windowHeight = windowHeight;
    // 获取屏幕分辨率
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);   // 屏幕宽度
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);  // 屏幕高度

    // 计算窗口位置：屏幕右下角（留出边距）
    int windowX = screenWidth - windowWidth - 10;   // 距右边缘 10px
    int windowY = screenHeight - windowHeight - 60; // 距底部 60px（避开任务栏）

    // 注册窗口类
    WNDCLASSEXA wc = { sizeof(WNDCLASSEXA) };  // 初始化结构体大小
    wc.style = CS_HREDRAW | CS_VREDRAW;        // 窗口大小变化时重绘
    wc.lpfnWndProc = WndProc;                  // 设置消息处理函数
    wc.hInstance = hInstance;                  // 设置实例句柄
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);  // 设置鼠标指针为箭头
    wc.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);  // 默认背景画刷
    wc.lpszClassName = "KeyVisualizerClass";   // 类名
    RegisterClassExA(&wc);  // 注册窗口类

    // 创建窗口（关键参数详解）
    g_hWnd = CreateWindowExA(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,  // 扩展样式：
        // WS_EX_LAYERED: 支持透明/半透明
        // WS_EX_TOPMOST: 窗口始终置顶
        // WS_EX_TOOLWINDOW: 不在任务栏显示图标
        "KeyVisualizerClass",  // 窗口类名
        "Keyboard Visualizer", // 窗口标题（实际不显示，因 WS_POPUP）
        WS_POPUP | WS_BORDER,  // 窗口样式：无边框弹窗 + 细边框
        windowX, windowY,      // 窗口位置（右下角）
        windowWidth, windowHeight,  // 窗口尺寸
        NULL, NULL, hInstance, NULL  // 其他参数（无菜单、无父窗口等）
    );

    // 设置窗口透明度：240/255 ≈ 94% 不透明度
    SetLayeredWindowAttributes(g_hWnd, 0, 240, LWA_ALPHA);

    // 显示并更新窗口
    ShowWindow(g_hWnd, SW_SHOW);   // 显示窗口
    UpdateWindow(g_hWnd);          // 立即触发一次 WM_PAINT 绘图
}

// ==================== 函数：程序入口（WinMain）====================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 1. 初始化：构建键盘布局数据
    InitKeyboardLayout();

    // 2. 安装全局键盘钩子（开始监听按键）
    InstallHook();

    // 3. 创建并显示透明窗口
    CreateOverlayWindow(hInstance);

    // 4. 消息循环（程序主循环）
    MSG msg;  // 消息结构体
    while (g_running && GetMessage(&msg, NULL, 0, 0)) {  // 获取消息，g_running=false 时退出

        TranslateMessage(&msg);   // 转换键盘消息（如 WM_KEYDOWN → WM_CHAR）
        DispatchMessage(&msg);    // 分发消息到 WndProc 处理

    }

    // 5. 清理资源：卸载钩子
    UninstallHook();

    return 0;  // 程序正常退出
}