FROM python:3.9-slim

# 安裝wine以支持運行Windows程序
RUN apt-get update && apt-get install -y --no-install-recommends \
    wine \
    wine32 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# 複製依賴文件
COPY requirements.txt .

# 安裝Python依賴
RUN pip install --no-cache-dir -r requirements.txt

# 複製應用代碼
COPY . .

# 創建結果目錄
RUN mkdir -p chart_results

# 設置環境變數
ENV PORT=8080
ENV PYTHONUNBUFFERED=1

# 暴露端口
EXPOSE 8080

# 運行應用
CMD gunicorn --bind 0.0.0.0:$PORT app:app 