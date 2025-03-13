@echo off

echo Welcome to Astrological Chart Calculator

REM Check command line parameters
if "%1"=="" goto prompt_inputs
set year=%1
set month=%2
set day=%3
set hour=%4
set minute=%5
set longitude=%6
set latitude=%7
goto run_calculation

:prompt_inputs
set /p year=Enter year (e.g., 1990): 
set /p month=Enter month (e.g., 1): 
set /p day=Enter day (e.g., 1): 
set /p hour=Enter hour (e.g., 12): 
set /p minute=Enter minute (e.g., 0): 
set /p longitude=Enter longitude (East is positive, e.g., 121.5): 
set /p latitude=Enter latitude (North is positive, e.g., 25.0): 

:run_calculation
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
    echo Chart file not found: %html_file%
    dir chart_*.html
)

echo Calculation complete!
echo.
echo Note: To convert HTML to PDF, please install wkhtmltopdf
echo Download: https://wkhtmltopdf.org/downloads.html
pause 