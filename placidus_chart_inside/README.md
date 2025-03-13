# 瑞士星曆占星盤計算程序

此程序基於Swiss Ephemeris天文曆法庫，用於計算和生成精確的占星盤圖表與數據。

## 功能特點

- 精確計算太陽、月亮及行星在黃道十二宮的位置
- 計算上升點、天頂點、北交點和幸運點
- 計算行星之間的相位關係（合相、對分相、三分相等）
- 生成美觀的HTML/SVG占星盤圖表
- 輸出詳細的行星位置、宮位和相位數據
- 支持多種星曆表（JPL DE441/DE431、瑞士星曆表、内建近似計算）

## 系統要求

- C編譯環境（如GCC、MinGW等）
- Swiss Ephemeris庫（已包含在`swisseph-master_inside`目錄中）
- Windows系統（使用`direct.h`創建目錄）

## 編譯指南

### 使用MinGW（Windows）

```
gcc -c -Wall -I./swisseph-master_inside placidus_chart_inside.c -o placidus_chart_inside.o
gcc -o placidus_chart_inside placidus_chart_inside.o -L./swisseph-master_inside -lswe -lm
```

### 使用Makefile

如果提供了Makefile，可以直接運行：

```
mingw32-make
```

## 使用方法

程序通過命令行傳入參數運行：

```
placidus_chart_inside.exe <年> <月> <日> <時> <分> <經度> <緯度>
```

例如：

```
placidus_chart_inside.exe 1990 1 1 12 0 121.5 25.0
```

### 參數說明

- `<年>` - 出生年份（4位數字）
- `<月>` - 出生月份（1-12）
- `<日>` - 出生日期（1-31）
- `<時>` - 出生小時（24小時制，0-23）
- `<分>` - 出生分鐘（0-59）
- `<經度>` - 出生地經度（東經為正，西經為負）
- `<緯度>` - 出生地緯度（北緯為正，南緯為負）

## 輸出文件

程序運行後會生成以下文件：

1. HTML占星盤圖表文件：`chart_YYYY_MM_DD_HH_MM.html`
2. 包含所有數據的文本文件：`YYYY_MM_DD_HH_MM_LON_LAT.txt` 
3. 以上文件的複製版本保存在以日期和位置命名的資料夾中

## 程序結構

- `placidus_chart_inside.c` - 主程序源代碼
- `swisseph-master_inside/` - Swiss Ephemeris庫
- `swisseph-master_inside/ephe/` - 星曆表數據文件

## 開發說明

如需修改或擴展此程序的功能，主要關注以下函數：

- `calculate_chart()` - 計算占星盤的核心函數
- `generate_svg_chart()` - 生成SVG圖表
- `save_txt_data()` - 保存文本數據

## 許可證

本程序使用Swiss Ephemeris，遵循其許可協議。
Swiss Ephemeris版權聲明：https://www.astro.com/swisseph/swephinfo_e.htm 