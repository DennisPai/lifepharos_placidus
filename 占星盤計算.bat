@echo off

echo Welcome to Astrological Chart Calculator

REM Set variables from command line
set year=%1
set month=%2
set day=%3
set hour=%4
set minute=%5
set longitude=%6
set latitude=%7

REM Check command line parameters
if "%year%"=="" goto prompt_inputs
if "%month%"=="" goto prompt_inputs
if "%day%"=="" goto prompt_inputs
if "%hour%"=="" goto prompt_inputs
if "%minute%"=="" goto prompt_inputs
if "%longitude%"=="" goto prompt_inputs
if "%latitude%"=="" goto prompt_inputs
goto run_calculation

:prompt_inputs
if "%year%"=="" set /p year=Enter year (e.g., 1990): 
if "%month%"=="" set /p month=Enter month (e.g., 1): 
if "%day%"=="" set /p day=Enter day (e.g., 1): 
if "%hour%"=="" set /p hour=Enter hour (e.g., 12): 
if "%minute%"=="" set /p minute=Enter minute (e.g., 0): 
if "%longitude%"=="" set /p longitude=Enter longitude (East is positive, e.g., 121.5): 
if "%latitude%"=="" set /p latitude=Enter latitude (North is positive, e.g., 25.0): 

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
    echo Chart file not found.
)

echo Calculation complete!
echo.
echo Note: To convert HTML to PDF, please install wkhtmltopdf
echo Download: https://wkhtmltopdf.org/downloads.html