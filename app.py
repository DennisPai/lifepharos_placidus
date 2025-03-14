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

# 檢查自定義Wine路徑環境變量
WINE_EXECUTABLE = os.environ.get('WINE_EXECUTABLE', 'wine')

if IS_WINDOWS:
    PLACIDUS_CHART_DIR = os.path.join(os.path.dirname(__file__), 'placidus_chart_inside')
    PLACIDUS_CHART_EXE = os.path.join(PLACIDUS_CHART_DIR, 'placidus_chart_inside.exe')
else:
    # 在非Windows環境下，我們使用Wine來運行Windows可執行文件
    PLACIDUS_CHART_DIR = os.path.join(os.path.dirname(__file__), 'placidus_chart_inside')
    PLACIDUS_CHART_EXE_PATH = os.path.join(PLACIDUS_CHART_DIR, 'placidus_chart_inside.exe')
    PLACIDUS_CHART_EXE = f'{WINE_EXECUTABLE} "{PLACIDUS_CHART_EXE_PATH}"'

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
        if not IS_WINDOWS:
            # 在非Windows環境下，確保Wine可用
            try:
                wine_version_check = subprocess.run([WINE_EXECUTABLE, '--version'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
                logger.info(f"Wine版本: {wine_version_check.stdout.strip()}")
            except Exception as e:
                logger.warning(f"Wine可能未正確配置: {e}")
                # 即使Wine不可用，也繼續嘗試執行 - Zeabur的Docker環境應該已配置好Wine
        
        # 構建命令
        if IS_WINDOWS:
            cmd = f'"{PLACIDUS_CHART_EXE}" {year} {month} {day} {hour} {minute} {longitude} {latitude}'
        else:
            # 對於Wine，我們需要確保路徑格式正確
            cmd = f'{WINE_EXECUTABLE} "{PLACIDUS_CHART_EXE_PATH}" {year} {month} {day} {hour} {minute} {longitude} {latitude}'
        
        logger.info(f"執行命令: {cmd}")
        
        # 在占星盤程序目錄中執行命令
        if IS_WINDOWS:
            working_dir = os.path.dirname(PLACIDUS_CHART_EXE)
        else:
            # 在非Windows環境下，直接使用PLACIDUS_CHART_DIR
            working_dir = PLACIDUS_CHART_DIR
        
        logger.info(f"工作目錄: {working_dir}")
        logger.info(f"可執行文件路徑: {PLACIDUS_CHART_EXE}")
        logger.info(f"是否Windows環境: {IS_WINDOWS}")
        
        try:
            # 執行命令
            logger.info(f"開始執行命令: {cmd}")
            
            # 確保工作目錄存在
            if not os.path.exists(working_dir):
                logger.error(f"工作目錄不存在: {working_dir}")
                os.makedirs(working_dir, exist_ok=True)
                logger.info(f"已創建工作目錄: {working_dir}")
            
            # 檢查可執行文件是否存在
            exe_path = os.path.join(PLACIDUS_CHART_DIR, 'placidus_chart_inside.exe')
            if not os.path.exists(exe_path):
                logger.error(f"可執行文件不存在: {exe_path}")
                flash('找不到占星盤計算程序')
                return redirect(url_for('index'))
            
            # 確保可執行文件有執行權限
            if not IS_WINDOWS:
                try:
                    os.chmod(exe_path, 0o755)
                    logger.info(f"已設置可執行文件權限: {exe_path}")
                except Exception as e:
                    logger.warning(f"設置可執行文件權限時出錯: {e}")
            
            process = subprocess.Popen(
                cmd,
                shell=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                cwd=working_dir
            )
            stdout, stderr = process.communicate(timeout=30)  # 添加超時設置
            
            # 記錄詳細輸出，無論成功或失敗
            logger.info(f"命令輸出: {stdout}")
            logger.info(f"錯誤輸出: {stderr}")
            
            if process.returncode != 0:
                logger.error(f"命令執行失敗，返回碼: {process.returncode}")
                flash('計算過程中發生錯誤')
                return redirect(url_for('index'))
            
            logger.info(f"命令執行成功，返回碼: {process.returncode}")
        
        except subprocess.TimeoutExpired:
            logger.error("命令執行超時")
            flash('計算過程超時，請稍後再試')
            return redirect(url_for('index'))
        except Exception as e:
            logger.exception(f"執行命令時發生異常: {e}")
            flash('執行占星盤計算程序時發生錯誤，請稍後再試。')
            return redirect(url_for('index'))
        
        # 找到生成的文件
        # 嘗試多種可能的文件夾名稱格式
        possible_folder_names = [
            # 標準格式 (分鐘可能用 '00' 或 '0')
            f"{year}_{month}_{day}_{hour}_{minute}_{float(longitude):.2f}_{float(latitude):.2f}",
            f"{year}_{month}_{day}_{hour}_{int(minute)}_{float(longitude):.2f}_{float(latitude):.2f}",
            # 可能的簡化格式 (精度減少)
            f"{year}_{month}_{day}_{hour}_{minute}_{float(longitude):.1f}_{float(latitude):.1f}",
            f"{year}_{month}_{day}_{hour}_{int(minute)}_{float(longitude):.1f}_{float(latitude):.1f}",
            # 最基本格式
            f"{year}_{month}_{day}_{hour}_{minute}",
            f"{year}_{month}_{day}_{hour}_{int(minute)}"
        ]
        
        folder_path = None
        logger.info(f"嘗試尋找生成的文件夾...")
        
        # 檢查所有可能的文件夾名稱
        for folder_name in possible_folder_names:
            temp_path = os.path.join(working_dir, folder_name)
            logger.info(f"檢查路徑: {temp_path}")
            if os.path.exists(temp_path):
                folder_path = temp_path
                logger.info(f"找到匹配的文件夾: {folder_path}")
                break
        
        # 如果仍然找不到，進行更廣泛的搜索
        if folder_path is None:
            # 列出工作目錄中所有文件夾
            try:
                all_items = os.listdir(working_dir)
                all_folders = [item for item in all_items if os.path.isdir(os.path.join(working_dir, item))]
                logger.info(f"工作目錄中的所有文件夾: {all_folders}")
                
                # 尋找最近創建的文件夾，該文件夾名稱包含年月日
                matching_folders = [folder for folder in all_folders if f"{year}_{month}_{day}" in folder]
                if matching_folders:
                    # 按創建時間排序，取最新的
                    latest_folder = max(
                        matching_folders, 
                        key=lambda f: os.path.getctime(os.path.join(working_dir, f))
                    )
                    folder_path = os.path.join(working_dir, latest_folder)
                    logger.info(f"使用最近創建的匹配文件夾: {folder_path}")
            except Exception as e:
                logger.error(f"廣泛搜索文件夾時出錯: {e}")
        
        if folder_path is None:
            logger.error(f"找不到生成的資料夾，嘗試過的名稱: {possible_folder_names}")
            # 列出工作目錄的內容，幫助診斷
            try:
                dir_content = os.listdir(working_dir)
                logger.info(f"工作目錄內容: {dir_content}")
            except Exception as e:
                logger.error(f"無法列出工作目錄內容: {e}")
            
            flash('無法找到生成的占星盤數據')
            return redirect(url_for('index'))
        
        # 根據找到的文件夾尋找HTML和TXT文件
        # 嘗試多種可能的文件名格式
        html_file = None
        txt_file = None
        
        # 檢查文件夾中的文件
        folder_files = os.listdir(folder_path)
        logger.info(f"文件夾內容: {folder_files}")
        
        # 首先檢查TXT文件
        txt_files = [f for f in folder_files if f.endswith('.txt')]
        if txt_files:
            txt_file = os.path.join(folder_path, txt_files[0])
            logger.info(f"找到TXT文件: {txt_file}")
        
        # 檢查HTML文件
        html_files = [f for f in folder_files if f.endswith('.html')]
        if html_files:
            html_file = os.path.join(folder_path, html_files[0])
            logger.info(f"找到HTML文件: {html_file}")
        
        # 如果文件夾中沒有HTML文件，檢查工作目錄
        if html_file is None:
            possible_html_names = [
                f"chart_{year}_{month}_{day}_{hour}_{minute}.html",
                f"chart_{year}_{month}_{day}_{hour}_{int(minute)}.html"
            ]
            
            for html_name in possible_html_names:
                temp_path = os.path.join(working_dir, html_name)
                if os.path.exists(temp_path):
                    html_file = temp_path
                    logger.info(f"在工作目錄中找到HTML文件: {html_file}")
                    break
        
        if html_file is None:
            logger.error(f"找不到生成的HTML文件")
            flash('無法找到生成的占星盤圖表')
            return redirect(url_for('index'))
        
        # 複製文件到結果目錄
        result_html = os.path.join(CHART_RESULTS_DIR, f"chart_{year}_{month}_{day}_{hour}_{minute}.html")
        result_txt = os.path.join(CHART_RESULTS_DIR, f"data_{year}_{month}_{day}_{hour}_{minute}.txt") if txt_file else None
        
        try:
            with open(html_file, 'r', encoding='utf-8') as src, open(result_html, 'w', encoding='utf-8') as dst:
                dst.write(src.read())
            
            if txt_file and os.path.exists(txt_file):
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