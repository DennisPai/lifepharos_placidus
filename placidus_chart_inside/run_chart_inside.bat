@echo off

REM Set variables from command line
set year=%1
set month=%2
set day=%3
set hour=%4
set minute=%5
set longitude=%6
set latitude=%7

REM Run the chart calculation
echo Calculating astrological chart...
placidus_chart.exe %year% %month% %day% %hour% %minute% %longitude% %latitude%

REM Check if HTML file was generated
set html_file=chart_%year%_%month%_%day%_%hour%_%minute%.html
if exist "%html_file%" (
    echo Chart generated, opening...
    start "" "%html_file%"
    echo HTML file opened: %html_file%
) else (
    echo Chart file not found.
)

echo Calculation completed!
echo.
echo Note: If you need to convert HTML to PDF, please install wkhtmltopdf software
echo Download URL: https://wkhtmltopdf.org/downloads.html 
echo 注意：如需將HTML轉為PDF，請安裝wkhtmltopdf軟體
echo 下載網址：https://wkhtmltopdf.org/downloads.html 