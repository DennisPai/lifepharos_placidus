import os
import sys
import subprocess
import tempfile
import glob
from flask import Flask, render_template, request, redirect, url_for, flash, send_file, abort
from werkzeug.utils import secure_filename
import platform
import logging

app = Flask(__name__)
app.secret_key = os.environ.get('SECRET_KEY', 'dev_key_12345')

# 設置日誌
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[logging.FileHandler('app.log'), logging.StreamHandler()]
)
logger = logging.getLogger(__name__)

# 檢測運行環境
IS_WINDOWS = platform.system() == 'Windows'

# 設置路徑
CHART_RESULTS_DIR = os.path.join(os.path.dirname(__file__), 'chart_results')
if not os.path.exists(CHART_RESULTS_DIR):
    os.makedirs(CHART_RESULTS_DIR)

if IS_WINDOWS:
    PLACIDUS_CHART_EXE = os.path.join(os.path.dirname(__file__), 'placidus_chart_inside', 'placidus_chart_inside.exe')
else:
    # 在非Windows環境下，我們使用Wine來運行Windows可執行文件
    # 注意：這需要在服務器上安裝Wine
    PLACIDUS_CHART_EXE = 'wine ' + os.path.join(os.path.dirname(__file__), 'placidus_chart_inside', 'placidus_chart_inside.exe')

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/calculate', methods=['POST'])
def calculate():
    try:
        # 從表單獲取數據
        year = request.form.get('year')
        month = request.form.get('month')
        day = request.form.get('day')
        hour = request.form.get('hour')
        minute = request.form.get('minute')
        longitude = request.form.get('longitude')
        latitude = request.form.get('latitude')
        
        # 驗證輸入
        if not all([year, month, day, hour, minute, longitude, latitude]):
            flash('請填寫所有必填字段')
            return redirect(url_for('index'))
        
        # 清理之前的結果
        for file in glob.glob(os.path.join(CHART_RESULTS_DIR, '*')):
            try:
                if os.path.isfile(file):
                    os.remove(file)
            except Exception as e:
                logger.error(f"無法刪除檔案 {file}: {e}")
        
        # 構建命令
        cmd = f"{PLACIDUS_CHART_EXE} {year} {month} {day} {hour} {minute} {longitude} {latitude}"
        logger.info(f"執行命令: {cmd}")
        
        # 在占星盤程序目錄中執行命令
        working_dir = os.path.dirname(PLACIDUS_CHART_EXE.split()[0] if ' ' in PLACIDUS_CHART_EXE else PLACIDUS_CHART_EXE)
        
        # 執行命令
        process = subprocess.Popen(
            cmd,
            shell=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            cwd=working_dir
        )
        stdout, stderr = process.communicate()
        
        if process.returncode != 0:
            logger.error(f"命令執行失敗，返回碼: {process.returncode}")
            logger.error(f"stderr: {stderr}")
            flash('計算過程中發生錯誤')
            return redirect(url_for('index'))
        
        logger.info(f"命令執行成功: {stdout}")
        
        # 找到生成的文件
        folder_name = f"{year}_{month}_{day}_{hour}_{minute}_{float(longitude):.2f}_{float(latitude):.2f}"
        folder_path = os.path.join(working_dir, folder_name)
        
        if not os.path.exists(folder_path):
            logger.error(f"找不到生成的資料夾: {folder_path}")
            flash('無法找到生成的占星盤數據')
            return redirect(url_for('index'))
        
        # 複製HTML文件到結果目錄
        html_file = os.path.join(folder_path, f"{year}_{month}_{day}_{hour}_{minute}_{float(longitude):.2f}_{float(latitude):.2f}.html")
        txt_file = os.path.join(folder_path, f"{year}_{month}_{day}_{hour}_{minute}_{float(longitude):.2f}_{float(latitude):.2f}.txt")
        
        if not os.path.exists(html_file):
            html_file = os.path.join(working_dir, f"chart_{year}_{month}_{day}_{hour}_{minute}.html")
            if not os.path.exists(html_file):
                logger.error(f"找不到生成的HTML文件: {html_file}")
                flash('無法找到生成的占星盤圖表')
                return redirect(url_for('index'))
        
        # 複製文件到結果目錄
        result_html = os.path.join(CHART_RESULTS_DIR, f"chart_{year}_{month}_{day}_{hour}_{minute}.html")
        result_txt = os.path.join(CHART_RESULTS_DIR, f"data_{year}_{month}_{day}_{hour}_{minute}.txt")
        
        with open(html_file, 'r', encoding='utf-8') as src, open(result_html, 'w', encoding='utf-8') as dst:
            dst.write(src.read())
        
        if os.path.exists(txt_file):
            with open(txt_file, 'r', encoding='utf-8') as src, open(result_txt, 'w', encoding='utf-8') as dst:
                dst.write(src.read())
        
        # 返回結果頁面
        return render_template('result.html', 
                               year=year, month=month, day=day, 
                               hour=hour, minute=minute,
                               longitude=longitude, latitude=latitude,
                               html_file=os.path.basename(result_html),
                               txt_file=os.path.basename(result_txt) if os.path.exists(txt_file) else None)
                               
    except Exception as e:
        logger.exception(f"處理過程中發生異常: {e}")
        flash('處理請求時發生錯誤')
        return redirect(url_for('index'))

@app.route('/view/<filename>')
def view_chart(filename):
    try:
        filepath = os.path.join(CHART_RESULTS_DIR, filename)
        if not os.path.exists(filepath):
            abort(404)
        
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        
        return content
    except Exception as e:
        logger.exception(f"查看文件時發生錯誤: {e}")
        abort(500)

@app.route('/download/<filename>')
def download_file(filename):
    try:
        filepath = os.path.join(CHART_RESULTS_DIR, filename)
        if not os.path.exists(filepath):
            abort(404)
        
        return send_file(filepath, as_attachment=True)
    except Exception as e:
        logger.exception(f"下載文件時發生錯誤: {e}")
        abort(500)

@app.errorhandler(404)
def page_not_found(e):
    return render_template('error.html', error="找不到請求的頁面"), 404

@app.errorhandler(500)
def internal_server_error(e):
    return render_template('error.html', error="伺服器內部錯誤"), 500

if __name__ == '__main__':
    port = int(os.environ.get('PORT', 5000))
    app.run(host='0.0.0.0', port=port, debug=False) 