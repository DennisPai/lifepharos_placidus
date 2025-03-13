// 占星盤計算器 Web 界面 JavaScript

document.addEventListener('DOMContentLoaded', function() {
    // 表單驗證
    const form = document.getElementById('chart-form');
    if (form) {
        form.addEventListener('submit', function(event) {
            let isValid = true;
            
            // 獲取所有必填字段
            const requiredFields = form.querySelectorAll('[required]');
            
            // 檢查每個必填字段
            requiredFields.forEach(function(field) {
                if (!field.value.trim()) {
                    isValid = false;
                    // 添加錯誤樣式
                    field.classList.add('is-invalid');
                } else {
                    // 移除錯誤樣式
                    field.classList.remove('is-invalid');
                }
            });
            
            // 驗證數值範圍
            const year = document.getElementById('year');
            const month = document.getElementById('month');
            const day = document.getElementById('day');
            const hour = document.getElementById('hour');
            const minute = document.getElementById('minute');
            const longitude = document.getElementById('longitude');
            const latitude = document.getElementById('latitude');
            
            if (year && parseInt(year.value) < 1800 || parseInt(year.value) > 2100) {
                isValid = false;
                year.classList.add('is-invalid');
            }
            
            if (month && (parseInt(month.value) < 1 || parseInt(month.value) > 12)) {
                isValid = false;
                month.classList.add('is-invalid');
            }
            
            if (day && (parseInt(day.value) < 1 || parseInt(day.value) > 31)) {
                isValid = false;
                day.classList.add('is-invalid');
            }
            
            if (hour && (parseInt(hour.value) < 0 || parseInt(hour.value) > 23)) {
                isValid = false;
                hour.classList.add('is-invalid');
            }
            
            if (minute && (parseInt(minute.value) < 0 || parseInt(minute.value) > 59)) {
                isValid = false;
                minute.classList.add('is-invalid');
            }
            
            if (longitude && (parseFloat(longitude.value) < -180 || parseFloat(longitude.value) > 180)) {
                isValid = false;
                longitude.classList.add('is-invalid');
            }
            
            if (latitude && (parseFloat(latitude.value) < -90 || parseFloat(latitude.value) > 90)) {
                isValid = false;
                latitude.classList.add('is-invalid');
            }
            
            // 如果表單無效，阻止提交
            if (!isValid) {
                event.preventDefault();
                // 顯示錯誤消息
                const errorAlert = document.createElement('div');
                errorAlert.className = 'alert alert-danger mt-3';
                errorAlert.textContent = '請檢查表單中的錯誤並重新提交';
                form.prepend(errorAlert);
                
                // 5秒後自動移除錯誤消息
                setTimeout(function() {
                    errorAlert.remove();
                }, 5000);
            } else {
                // 顯示加載指示器
                const submitBtn = form.querySelector('button[type="submit"]');
                if (submitBtn) {
                    submitBtn.disabled = true;
                    submitBtn.innerHTML = '<span class="spinner-border spinner-border-sm" role="status" aria-hidden="true"></span> 計算中...';
                }
            }
        });
        
        // 輸入時移除錯誤樣式
        const inputs = form.querySelectorAll('input');
        inputs.forEach(function(input) {
            input.addEventListener('input', function() {
                this.classList.remove('is-invalid');
            });
        });
    }
    
    // 結果頁面功能
    const chartContainer = document.getElementById('chart-container');
    if (chartContainer) {
        // 添加圖表縮放功能
        const chartSvg = chartContainer.querySelector('svg');
        if (chartSvg) {
            let scale = 1;
            const zoomIn = document.getElementById('zoom-in');
            const zoomOut = document.getElementById('zoom-out');
            const resetZoom = document.getElementById('reset-zoom');
            
            if (zoomIn) {
                zoomIn.addEventListener('click', function() {
                    scale += 0.1;
                    chartSvg.style.transform = `scale(${scale})`;
                });
            }
            
            if (zoomOut) {
                zoomOut.addEventListener('click', function() {
                    if (scale > 0.5) {
                        scale -= 0.1;
                        chartSvg.style.transform = `scale(${scale})`;
                    }
                });
            }
            
            if (resetZoom) {
                resetZoom.addEventListener('click', function() {
                    scale = 1;
                    chartSvg.style.transform = 'scale(1)';
                });
            }
        }
    }
}); 