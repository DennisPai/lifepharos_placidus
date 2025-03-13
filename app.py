import os
import sys
import subprocess
import tempfile
import glob
import shutil
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

# 設置星曆表路徑
DE406_PATH = os.path.join(os.path.dirname(__file__), 'de406.eph')

if IS_WINDOWS:
    PLACIDUS_CHART_DIR = os.path.join(os.path.dirname(__file__), 'placidus_chart_inside')
    PLACIDUS_CHART_EXE = os.path.join(PLACIDUS_CHART_DIR, 'placidus_chart_inside.exe')
else:
    # 在非Windows環境下，我們使用Wine來運行Windows可執行文件
    PLACIDUS_CHART_DIR = os.path.join(os.path.dirname(__file__), 'placidus_chart_inside')
    PLACIDUS_CHART_EXE = 'wine ' + os.path.join(PLACIDUS_CHART_DIR, 'placidus_chart_inside.exe')

# 確保星曆表文件在正確的位置
def ensure_ephemeris_file():
    # 檢查星曆表是否存在
    working_dir = os.path.dirname(PLACIDUS_CHART_EXE.split()[0] if ' ' in PLACIDUS_CHART_EXE else PLACIDUS_CHART_EXE)
    target_path = os.path.join(working_dir, 'de406.eph')
    
    if not os.path.exists(target_path) and os.path.exists(DE406_PATH):
        logger.info(f"複製星曆表文件到工作目錄: {target_path}")
        shutil.copy2(DE406_PATH, target_path)
    
    return os.path.exists(target_path)

@app.route('/')
def index():
    # 檢查星曆表文件
    has_ephemeris = ensure_ephemeris_file()
    if not has_ephemeris:
        flash('警告：找不到星曆表文件(de406.eph)，計算結果可能不準確')
    
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
        
        # 確保星曆表文件存在
        has_ephemeris = ensure_ephemeris_file()
        if not has_ephemeris:
            logger.warning("找不到星曆表文件(de406.eph)，使用內建近似計算")
        
        # 檢查是否可以執行占星盤程序
        executable_path = PLACIDUS_CHART_EXE.split()[0] if ' ' in PLACIDUS_CHART_EXE else PLACIDUS_CHART_EXE
        if not os.path.exists(executable_path) and not IS_WINDOWS:
            logger.error(f"在非Windows環境下找不到可執行文件: {executable_path}")
            flash('當前伺服器環境不支持運行Windows程序。請在本地Windows環境運行此應用或使用已生成的結果。')
            return redirect(url_for('index'))
        
        # 構建命令
        cmd = f"{PLACIDUS_CHART_EXE} {year} {month} {day} {hour} {minute} {longitude} {latitude}"
        logger.info(f"執行命令: {cmd}")
        
        # 在占星盤程序目錄中執行命令
        working_dir = os.path.dirname(PLACIDUS_CHART_EXE.split()[0] if ' ' in PLACIDUS_CHART_EXE else PLACIDUS_CHART_EXE)
        
        try:
            # 執行命令
            process = subprocess.Popen(
                cmd,
                shell=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                cwd=working_dir
            )
            stdout, stderr = process.communicate(timeout=30)  # 添加超時設置
            
            if process.returncode != 0:
                logger.error(f"命令執行失敗，返回碼: {process.returncode}")
                logger.error(f"stderr: {stderr}")
                flash('計算過程中發生錯誤')
                return redirect(url_for('index'))
            
            logger.info(f"命令執行成功: {stdout}")
        
        except subprocess.TimeoutExpired:
            logger.error("命令執行超時")
            flash('計算過程超時，請稍後再試')
            return redirect(url_for('index'))
        except Exception as e:
            logger.exception(f"執行命令時發生異常: {e}")
            flash('無法執行占星盤計算程序。請在Windows環境中運行此應用。')
            return redirect(url_for('index'))
        
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
        
        try:
            with open(html_file, 'r', encoding='utf-8') as src, open(result_html, 'w', encoding='utf-8') as dst:
                dst.write(src.read())
            
            if os.path.exists(txt_file):
                with open(txt_file, 'r', encoding='utf-8') as src, open(result_txt, 'w', encoding='utf-8') as dst:
                    dst.write(src.read())
        except Exception as e:
            logger.exception(f"處理結果文件時發生異常: {e}")
            flash('處理結果文件時發生錯誤')
            return redirect(url_for('index'))
        
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