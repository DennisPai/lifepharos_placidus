# 占星盤計算器 Web 界面

這是一個基於Flask的Web應用程序，為瑞士星曆占星盤計算程序提供了一個友好的用戶界面。

## 功能特點

- 簡單易用的Web界面，用於輸入出生日期、時間和地點
- 生成互動式占星盤圖表
- 提供詳細的占星數據文本文件下載
- 響應式設計，適合各種設備使用

## 系統要求

- Python 3.6+
- Flask 2.3+
- 瑞士星曆占星盤計算程序 (placidus_chart_inside.exe)

## 安裝步驟

1. 確保您已安裝Python 3.6或更高版本
2. 克隆或下載此存儲庫
3. 安裝所需的Python依賴項：

```
pip install -r requirements.txt
```

## 使用方法

1. 啟動Web服務器：

```
python app.py
```

2. 在瀏覽器中訪問：http://localhost:5000
3. 輸入出生日期、時間和地點信息
4. 點擊"計算占星盤"按鈕
5. 查看生成的占星盤圖表和下載詳細數據

## 文件結構

- `app.py` - Flask應用程序主文件
- `templates/` - HTML模板文件
  - `index.html` - 首頁/輸入表單
  - `result.html` - 結果顯示頁面
  - `error.html` - 錯誤頁面
- `static/` - 靜態資源文件
  - `css/` - 樣式表文件
  - `js/` - JavaScript文件
  - `images/` - 圖片資源
- `placidus_chart_inside/` - 占星盤計算程序
  - `placidus_chart_inside.exe` - 編譯好的占星盤計算程序
  - `swisseph-master_inside/` - Swiss Ephemeris庫及星曆表
- `chart_results/` - 生成的圖表和數據文件存儲目錄

## 技術架構

### 前端

- HTML5, CSS3, JavaScript
- Bootstrap 5 - 響應式UI框架
- jQuery - JavaScript庫（用於AJAX請求和DOM操作）

### 後端

- Python 3 - 主要編程語言
- Flask - Web框架
- Flask-WTF - 表單處理和驗證
- Jinja2 - 模板引擎
- C - 占星盤計算核心（placidus_chart_inside）

### 數據流

1. 用戶通過Web表單提交出生信息
2. Flask應用程序接收並驗證數據
3. Flask調用placidus_chart_inside.exe進行計算
4. 計算生成HTML/SVG圖表和TXT文本數據
5. Flask應用程序讀取生成的文件並呈現結果頁面
6. 用戶可以查看圖表和下載數據文件

## 部署指南

### 本地開發環境

建議使用虛擬環境隔離依賴：

```bash
# 創建虛擬環境
python -m venv venv

# 激活虛擬環境
# Windows:
venv\Scripts\activate
# Linux/Mac:
source venv/bin/activate

# 安裝依賴
pip install -r requirements.txt

# 啟動開發服務器
python app.py
```

### 部署到生產環境

#### 使用Gunicorn和Nginx（Linux）

1. 安裝Gunicorn：

```bash
pip install gunicorn
```

2. 創建Gunicorn啟動腳本：

```bash
gunicorn -w 4 -b 127.0.0.1:5000 app:app
```

3. 配置Nginx代理：

```nginx
server {
    listen 80;
    server_name your_domain.com;

    location / {
        proxy_pass http://127.0.0.1:5000;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
    }

    location /static {
        alias /path/to/your/app/static;
    }
}
```

#### 使用Docker

1. 創建Dockerfile：

```dockerfile
FROM python:3.9-slim

WORKDIR /app

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY . .

EXPOSE 5000

CMD ["gunicorn", "-w", "4", "-b", "0.0.0.0:5000", "app:app"]
```

2. 構建並運行Docker容器：

```bash
docker build -t astrology-chart-app .
docker run -p 5000:5000 astrology-chart-app
```

#### 部署到Zeabur（雲平台）

詳細步驟請參閱下文的Zeabur部署指南。

## 開發指南

### 添加新功能

1. 克隆存儲庫並創建新分支：

```bash
git clone https://github.com/yourusername/astrology-chart-app.git
git checkout -b feature/your-feature-name
```

2. 進行代碼修改和測試
3. 提交變更並創建Pull Request

### 修改占星盤計算

如需修改占星盤計算邏輯或輸出格式：

1. 編輯`placidus_chart_inside/placidus_chart_inside.c`
2. 使用GCC或MinGW重新編譯程序
3. 更新輸出解析邏輯（如果需要）

### 修改Web界面

1. 前端模板位於`templates/`目錄
2. 靜態資源位於`static/`目錄
3. 路由和視圖函數位於`app.py`

## 故障排除

### 常見問題

1. **無法啟動應用程序**
   - 確保已正確安裝所有依賴項
   - 檢查是否已激活虛擬環境
   - 檢查端口5000是否被占用

2. **占星盤計算錯誤**
   - 確保`placidus_chart_inside.exe`存在並有執行權限
   - 檢查星曆表文件是否存在於正確位置
   - 檢查日期和位置輸入是否有效

3. **無法顯示圖表**
   - 檢查生成的HTML文件是否存在
   - 檢查存儲權限和路徑配置
   - 查看服務器日誌以獲取詳細錯誤信息

### 日誌

應用程序日誌可幫助診斷問題：

```python
# 在app.py中設置日誌
import logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[logging.FileHandler('app.log'), logging.StreamHandler()]
)
```

## 許可證

本項目使用MIT許可證。詳見LICENSE文件。

## 致謝

- Swiss Ephemeris團隊提供的天文計算庫
- Flask團隊提供的Web框架
- 所有貢獻者和測試人員

## 聯繫方式

如有問題或建議，請通過以下方式聯繫：

- 提交GitHub Issues
- 電子郵件：your-email@example.com 