# 瑞士星曆占星盤計算程序 - 開發者文檔

本文檔提供關於占星盤計算程序的詳細技術說明，幫助開發者理解程序結構、修改或擴展功能。

## 設計原理

這個程序基於Swiss Ephemeris天文曆法庫，遵循以下占星學原則：

1. 使用太陽黃經座標系統（ecliptic coordinates）計算天體位置
2. 使用Placidus分宮系統（可定制為其他系統如Koch、Campanus等）
3. 計算行星之間的相位關係及其軌道差距（Orb）
4. 計算並展示傳統的十大行星、交點及重要固定點

## 核心數據結構

程序使用以下關鍵數據結構：

### PlanetInfo 結構

```c
typedef struct {
    int planet_num;          // 行星編號
    double longitude;        // 經度（0-360度）
    int sign;                // 所在星座（0-11）
    double sign_longitude;   // 在星座內的度數（0-30度）
    double latitude;         // 緯度
    double distance;         // 距離（天文單位）
    double speed;            // 速度（度/天）
    int house;               // 所在宮位（0-11）
    int retrograde;          // 逆行標記（1=逆行，0=順行）
} PlanetInfo;
```

### HouseInfo 結構

```c
typedef struct {
    double cusp_longitude;   // 宮頭經度
    int sign;                // 宮頭所在星座
    double sign_longitude;   // 宮頭在星座內的度數
    int ruler;               // 宮主星（行星編號）
} HouseInfo;
```

### AspectInfo 結構

```c
typedef struct {
    int planet1;             // 第一個行星
    int planet2;             // 第二個行星
    int aspect_type;         // 相位類型（0=合相，1=對分相，等）
    double orb;              // 軌道差距（度）
    int applying;            // 是否形成中（1=形成中，0=分離中）
} AspectInfo;
```

## 核心函數說明

### 主要計算函數

- `calculate_chart(...)`: 計算整個占星盤，包括行星位置、宮位和相位
- `estimate_timezone(...)`: 根據經度估算時區
- `local_to_ut(...)`: 將本地時間轉換為世界時
- `calculate_pars_fortunae(...)`: 計算幸運點位置

### 輸出和顯示函數

- `display_chart(...)`: 在控制台顯示占星盤數據
- `generate_svg_chart(...)`: 生成SVG格式的占星盤圖表
- `save_txt_data(...)`: 將占星盤數據保存為文本文件
- `create_folder(...)`: 創建存儲輸出文件的文件夾

## 擴展和定制

### 添加新的天體

如需添加新的行星或固定點：

1. 在常量定義區域增加新天體的標識和名稱
2. 更新 `PLANETS_COUNT` 或 `EXTRA_POINTS_COUNT` 常量
3. 在 `calculate_chart()` 函數中添加對應的計算代碼
4. 在顯示和輸出函數中添加對應的處理邏輯

### 修改分宮系統

若要更改分宮系統：

1. 在 `swe_houses()` 調用中更改系統識別符（如'P'代表Placidus）
2. 可選系統包括：
   - 'P': Placidus
   - 'K': Koch
   - 'O': Porphyrius
   - 'R': Regiomontanus
   - 'C': Campanus
   - 'A': Equal (Ascendant)
   - 'E': Equal (MC)
   - 'W': Whole Sign
   - 'B': Alcabitius

### 修改相位計算

如需調整相位規則：

1. 修改 `aspect_angles[]` 和 `aspect_orbs[]` 數組
2. 在 `calculate_chart()` 函數中的相位計算邏輯中進行調整

### 修改圖表樣式

如需調整占星盤的視覺表現：

1. 修改 `generate_svg_chart()` 函數中的SVG生成代碼
2. 更改圓環半徑、顏色、字體大小等參數
3. 調整行星符號和星座符號的位置和大小

## 常見問題

### 星曆表路徑問題

程序會按照以下順序查找星曆表文件：
1. `EPHE_PATH` 宏定義的目錄下的JPL DE441星曆表
2. `EPHE_PATH` 宏定義的目錄下的JPL DE431星曆表
3. `EPHE_PATH` 宏定義的目錄下的瑞士星曆表(.se1文件)
4. 內建的Moshier近似計算（無需外部文件）

如遇星曆表問題，請檢查路徑設置和文件完整性。

### 時區計算問題

程序根據經度自動估算時區，精度可能不足。如需更精確的時區處理，可修改 `estimate_timezone()` 函數，接入時區數據庫。

### 占星符號顯示問題

如有占星符號無法正確顯示的問題，請確保：
1. 使用的字體支持占星符號（如Arial Unicode MS）
2. HTML文件指定了正確的字符編碼（UTF-8）

## 技術依賴

- Swiss Ephemeris 2.10.03+
- C99兼容編譯器
- 標準C庫（stdio.h, stdlib.h, math.h, string.h等） 